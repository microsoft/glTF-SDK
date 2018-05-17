<#
.SYNOPSIS
Invokes a configured set of tests.

.DESCRIPTION
This script is mostly for running tests from VSTS, but can also be used to run
tests locally. It uses a passed-in configuration file to determine the set of
tests it will run. It currently handles running .dll and .appx tests through
VSTest on Windows, and GoogleTest executables on Windows, MacOS and Android.

.PARAMETER ConfigFile
A .json file with a list of tests to run. For sample format
see: Build/Scripts/Tests.*.json

.PARAMETER Platform
The platform these tests were built for. Valid values are: arm, x64, x86, win32. Valid values for Android are: x86, x86_64, arm32, arm64.

.PARAMETER Configuration
The configuration these tests were built for. Valid values are: release, debug

.PARAMETER iOSSimulatorType
This value must be a valid simulator device type, that can be called when prefixed 
with com.appleCoreSimulator.SimDeviceType (e.g. iPhone-7-Plus)

.PARAMETER iOSSimulatorRuntime
This must be a valid iOS Simulator Runtime, and must be a valid command
when suffixed with com.apple.CoreSimulator.SimRuntime (e.g. iOS-11-2)

.PARAMETER ReportPath
The directory we will drop all of our test reports.

.PARAMETER VSTestVersion
The version of Visual Studio we'll use when running VSTest tests. Valid values are: 14.0, 15.0

.PARAMETER TestNames
A list of test names to run. If not provided, we'll use the config file to determine which tests to run.

.PARAMETER Tags
A comma-separated list of test tags we want to run. Every test in the passed-in config file has one
or more tags (if a test has no provided tags it will have the 'default' tag).
If a test has at least one of the passed-in tags we run it.

.PARAMETER JustRunEverything
If set, we'll run every configured test regardless of whether or not it's configured to run for
this reason/platform/configuration.

.PARAMETER DryRun
If set, we won't run tests - we'll just print what we would run.

.PARAMETER BuildReason
DEPRECATED - DON'T USE. Retained for now so this PR doesn't break existing build definitions.

.PARAMETER AndroidApiLevel
This must be a valid android api level. For example: "21", etc.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$ConfigFile,
    [Parameter()]
    [String]$Platform,
    [Parameter()]
    [String]$Configuration,
    [Parameter()]
    [String]$iOSSimulatorType = "iPhone-8",
    [Parameter()]
    [String]$iOSSimulatorRuntime = "iOS-11-3",
    [Parameter()]
    [String]$ReportPath,
    [Parameter()]
    [String]$VsTestVersion = "15.0",
    [Parameter()]
    [String[]]$TestNames,
    [Parameter()]
    [String]$Tags = "default",
    [Parameter()]
    [String]$AppxDependencyPath,
    [Parameter()]
    [Switch]$JustRunEverything,
    [Parameter()]
    [Switch]$DryRun,
    [Parameter()]
    [Switch]$BuildReason,
    [Parameter()]
    [String]$AndroidApiLevel = "21"
)

$CurrentPath = Get-Location
$ErrorActionPreference = "Stop"

$Verbose = [bool]$PSBoundParameters["Verbose"]

$RunSimulatorTest = (Join-Path $PSScriptRoot "Invoke-iOSSimulatorTest.ps1")
$RunAndroidEmulatorTest = (Join-Path $PSScriptRoot "Invoke-AndroidEmulatorTest.ps1")
$UninstallAppx = (Join-Path $PSScriptRoot "Uninstall-AppxFiles.ps1")
$InstallAppx = (Join-Path $PSScriptRoot "Install-AppxFiles.ps1")

if ($BuildReason)
{
    Write-Warning "Build reason $BuildReason was provided, but this parameter is no longer being used."
}

$TestReportExtensions = @{
    "AppxCpp" = "trx"
    "AppxCSharp" = "trx"
    "Dll" = "trx"
    "GoogleTest" = "xml"
}
$TestReportTypes = @{
    "AppxCpp" = "VSTest"
    "AppxCSharp" = "VSTest"
    "Dll" = "VSTest"
    "GoogleTest" = "JUnit"
}

$VsTestPaths = @{
    '14.0' = 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe'
    '15.0' = 'C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe'
}
$VsTestPath = $VsTestPaths[$VsTestVersion]
if (! $VsTestPath)
{
    Write-Error "No vstest path defined for version $VsTestVersion"
}

function main
{
    if ($ReportPath -and -not (Test-Path $ReportPath))
    {
        New-Item -Type Directory $ReportPath | Out-Null
    }

    $config = Get-Content $ConfigFile | ConvertFrom-Json
    $testsToRun = getTestsToRun $config

    # We can run all non-appx tests in parallel, but we must run appx tests one at a time
    $appxTests = $testsToRun | Where-Object { $_.Framework -match "Appx" }
    $iOSSimulatorTests = $testsToRun | Where-Object { $_.Framework -match "iOSSimulator" }
    $androidEmulatorTests = $testsToRun | Where-Object { $_.Framework -match "androidEmulator" }
    $nonAppxTests = $testsToRun | Where-Object { $_.Framework -notmatch "Appx" -and $_.Framework -notmatch "iOSSimulator" -and $_.Framework -notmatch "androidEmulator"}
    
    $testResults = @()

    # First, start all non-appx tests in parallel
    $nonAppxJobs = $nonAppxTests | Foreach-Object { runTest $_ $config }
    Write-Verbose "Running $($nonAppxJobs.Count) tests in the background."

    # If there are existing appx tests or dependencies installed on the system they can prevent us from installing new appx tests.
    if ($appxTests -and $AppxDependencyPath)
    {
        removeAllExistingAppxTests $appxTests $config
        reinstallAllAppxDependencies
    }
    
    # Then, run appx tests one at a time
    foreach ($test in $appxTests)
    {
        $testResults += runTest $test $config | Receive-Job -Wait -AutoRemoveJob -ErrorAction Continue
    }
    
    # Simulator tests need to be run one at a time
    $testResults += runiOSSimulatorTests $iOSSimulatorTests

    $testResults += runAndroidEmulatorTests $androidEmulatorTests

    # Then, wait for all non-appx tests to complete (they're probably already all completed)
    Write-Verbose "All child jobs: $(Get-Job | Format-Table Id,Name,State | Out-String)"
    foreach ($job in $nonAppxJobs)
    {
        Write-Verbose "Waiting for job: $($job | Format-Table Id,Name,State | Out-String)"
        $testResults += $job | Receive-Job -Wait -AutoRemoveJob
    }

    publishTestReports
    
    # Then, fail if any test failed
    if ($testResults -contains $false)
    {
        Write-Host "##vso[task.logissue type=error]Hit at least one test failure. Search for [TEST FAILED] or [TEST BINARY FAILED] to in the log to find where we failed."
        exit 1
    }

    exit 0
}

function getTestsToRun($config)
{
    foreach ($test in $config.tests)
    {
        if (isTestApplicable $test)
        {
            Write-Verbose "Running $test"
            Write-Output $test
        }
    }
}

function isTestApplicable($test)
{
    if ($JustRunEverything)
    {
        return $true
    }

    # If -TestNames was provided, trust that the user knows what they're doing
    if ($TestNames)
    {
        return ($TestNames -contains $test.Name)
    }

    if (($test.Framework -notmatch "Appx") -and ((getTestPlatform) -eq "ARM"))
    {
        Write-Verbose "Not running $($test.Framework) test $($test.Name) on ARM"
        return $false
    }

    if ($test.Platforms -and -not ($test.Platforms -contains $Platform))
    {
        Write-Verbose "Not running $($test.Name) because we aren't on one of: $($test.Platforms)"
        return $false
    }
    
    if ($test.Configurations -and -not ($test.Configurations -contains $Configuration))
    {
        Write-Verbose "Not running $($test.Name) because we aren't on one of: $($test.Configurations)"
        return $false
    }

    if ($test.Tags)
    {
        $testTags = $test.Tags
    }
    else
    {
        $testTags = "default"
    }

    $providedTags = $Tags -split ","
    if (! ($testTags | Where-Object { $providedTags -contains $_ }))
    {
        Write-Verbose "Not running $($test.Name) because it has tags [$($testTags -join ",")] but we're only running tests with tags [$($providedTags -join ",")]"
        return $false
    }

    return $true
}

# Launch each simulator type one at a time, and gracefully clean up after tests
function runiOSSimulatorTests($simulatorTests)
{
    if (!$simulatorTests)
    {
        return
    }

    $testBinaries = $simulatorTests | ForEach-Object { findTestBinary $_ $config}

    # iOSSimulatorTest script returns a list of PSObjects [testSucceeded; output; testBinaryName;], one object for each test run
    $testResults = . $RunSimulatorTest -DeviceType $iOSSimulatorType -DeviceRuntime $iOSSimulatorRuntime -TestBinaryPaths $testBinaries -ReportPath $ReportPath -Verbose:$Verbose

    foreach($object in $testResults)
    {
        $testBinaryName = $object.testBinaryName
        foreach ($line in $object.output)
        {
            Write-Host $line
            if ($line -match "\[\s*FAILED\s*\]\s*(.+)$")
            {
                $testName = $matches[1]
                Write-Host "##vso[task.logissue type=error][TEST FAILED] $testBinaryName::$testName"
                
                # this edits the actual $testResults object
                $object.testSucceeded = $false
            }
        }
        if ($object.output -notcontains "Global test environment tear-down")
        {
            Write-Host "##vso[task.logissue type=error][TEST FAILED] $testBinaryName exited unexpectedly or crashed"
            $object.testSucceeded = $false
        }
        
        if ($object.testSucceeded)
        {
            Write-Verbose "[TEST BINARY PASSED] $testBinaryName"
        }
        else
        {
            Write-Host "##vso[task.logissue type=error][TEST BINARY FAILED] $testBinaryName"
        }
    }
    return $testResults.testSucceeded
}

function runAndroidEmulatorTests($emulatorTests)
{
    if (!$emulatorTests)
    {
        return
    }

    $testResults = . $RunAndroidEmulatorTest -Platform $Platform -Configuration $Configuration -Tests $emulatorTests -ApiLevel $AndroidApiLevel -ReportPath $ReportPath

    foreach ($object in $testResults)
    {
        $testBinaryName = $object.testBinaryName
        foreach ($line in $object.output)
        {
            if ($line -match "\[\s*FAILED\s*\]\s*(.+)$")
            {
                $testName = $matches[1]
                Write-Host "##vso[task.logissue type=error][TEST FAILED] $testBinaryName::$testName"
                
                # this edits the actual $testResults object
                $object.testSucceeded = $false
            }
        }

        if ("$($object.output)" -notlike "*Global test environment tear-down*")
        {
            Write-Host "##vso[task.logissue type=error][TEST FAILED] $testBinaryName exited unexpectedly or crashed"
            $object.testSucceeded = $false
        }
        
        if ($object.testSucceeded)
        {
            Write-Verbose "[TEST BINARY PASSED] $testBinaryName"
        }
        else
        {
            Write-Host "##vso[task.logissue type=error][TEST BINARY FAILED] $testBinaryName"
        }
    }
    return $testResults.testSucceeded
}

function runTest($test, $config)
{
    if ($test.framework -eq "GoogleTest")
    {
        return runGoogleTest $test $config
    }
    if ($test.framework -match "Appx")
    {
        return runAppxTest $test $config
    }
    if ($test.framework -eq "Dll")
    {
        return runDllTest $test $config
    }

    Write-Error "Don't know how to run $($test.framework)"
}

function runGoogleTest($test, $config)
{
    $testBinary = findTestBinary $test $config
    $testReportPath = getTestReportPath $test

    $arguments = @()
    if ($testReportPath)
    {
        $arguments += "--gtest_output=xml:$testReportPath"
    }

    if ($test.Filter)
    {
        $arguments += "--gtest_filter=$($test.Filter)"
    }

    if ($test.AdditionalArguments)
    {
        $arguments += $test.AdditionalArguments
    }

    return Start-Job -Verbose:$Verbose -ArgumentList @($testBinary, $arguments, $DryRun) {
        Param($testBinary, $arguments, $DryRun)
        $ErrorActionPreference = "Stop"

        $testBinaryName = Split-Path -Leaf $testBinary

        Write-Verbose "Running from $($testBinary.Directory)"
        Set-Location $testBinary.Directory

        Write-Verbose "Running: $testBinary $arguments"

        if ($DryRun)
        {
            return $true
        }

        $ErrorActionPreference = "Continue" # Don't fail immediately if the test writes to stderr
        &$testBinary $arguments 2>&1 | Foreach-Object {
            # Parse the log and print errors to the error stream, so we get failure messages in email.
            $line = [String]$_
            Write-Host $line

            if ($line -match "\[\s*FAILED\s*\]\s*(.+)$")
            {
                $testName = $matches[1]
                Write-Host "##vso[task.logissue type=error][TEST FAILED] $testBinaryName::$testName"
            }
        }

        # TODO: We currently ignore stderr because some tests are printing to stderr due to an issue
        # where we don't properly clean up devices on Metal. Once we fix those issues, we should start 
        # failing on stderr (check $? or $error)
        $testSuccess = $lastexitcode -eq 0
        if ($testSuccess)
        {
            Write-Verbose "[TEST BINARY PASSED] $testBinaryName"
        }
        else
        {
            Write-Host "##vso[task.logissue type=error][TEST BINARY FAILED] $testBinaryName"
        }
        return $testSuccess
    }
}

function runAppxTest($test, $config)
{
    $testBinary = findTestBinary $test $config @"
Visual Studio doesn't build appx packages by default; consider building on the command-line with something like:
&"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\msbuild.exe" Build\Canvas3d.sln /p:platform="$Platform" /p:configuration="$Configuration" /p:VisualStudioVersion="$VsTestVersion"
"@

    # There's a silly bug in VSTest right now: it tries to install all .appx packages in the dependencies folder except ARM
    # tests, but Visual Studio recently started deploying ARM64 tests as well. VSTest does try to install these, but fails
    # because of course we're not running on an ARM64 device.
    Remove-Item (Join-Path $testBinary.Directory.FullName "Dependencies/ARM64") -Recurse -Force -ErrorAction Ignore

    _runVsTest $test $config @("/InIsolation")
}

function runDllTest($test, $config)
{
    _runVsTest $test $config @("/InIsolation", "/EnableCodeCoverage")
}

function _runVsTest($test, $config, $arguments)
{
    $testPlatform = getTestPlatform
    $settingsFile = findAppxSettingsFile $config

    $testBinary = findTestBinary $test $config
    $testBinaryName = Split-Path -Leaf $testBinary
    $testReportPath = getTestReportPath $test

    $arguments += @("/Platform:$testPlatform", $testBinary)
    if ($settingsFile)
    {
        $arguments += "/Settings:$settingsFile"
    }

    $frameworkFilter = getFilterForFramework $test.framework
    $configuredFilter = $test.filter
    if ($frameworkFilter -and $configuredFilter)
    {
        $filter = "($frameworkFilter)&($configuredFilter)"
    }
    elseif ($frameworkFilter)
    {
        $filter = $frameworkFilter
    }
    elseif ($configuredFilter)
    {
        $filter = $configuredFilter
    }

    if ($filter)
    {
        $arguments += "/TestCaseFilter:$filter"
    }

    return Start-Job -ArgumentList @($VsTestPath, $arguments, $testReportPath, $testBinaryName, $DryRun) {
        Param($VsTestPath, $arguments, $testReportPath, $testBinaryName, $DryRun)
        $ErrorActionPreference = "Stop"
        $VerbosePreference = "Continue" 

        if ($testReportPath)
        {
            $arguments += "/logger:trx"

            # VSTest doesn't allow you to specify your test filename yet. Once we're
            # using 15.5+ we'll be able to specify a filename with /logger:trx;LogFile[Name]=<filename>
            # For now, just make sure our PWD is in a temporary directory and move it afterwards
            $tempPath = Join-Path (Split-Path $testReportPath) ([System.IO.Path]::GetRandomFileName())
            New-Item -ItemType Directory -Path $tempPath | Out-Null
            Write-Verbose "Running from $tempPath to ensure test results are dropped in the right location."
            Set-Location $tempPath
        }

        $ErrorActionPreference = "Continue"
        Write-Verbose "Running: $VsTestPath $arguments"
        if ($DryRun)
        {
            return $true
        }
        &$VsTestPath $arguments 2>&1 | ForEach-Object {
            # Parse the log and print errors to the error stream, so we get failure messages in email.
            $line = [String]$_
            Write-Host $line

            if ($line -match "^(Passed|Failed)\s+(\S+)$")
            {
                $result,$testName = $matches[1..2]
                if ($result -eq "Failed")
                {
                    Write-Host "##vso[task.logissue type=error][TEST FAILED] $testBinaryName::$testName"
                }
            }


            if ($line.StartsWith("Error: The active Test Run was aborted"))
            {
                $err = "[TEST FAILED] $testBinaryName crashed"
                if ($testName)
                {
                    $err += ". Last test that ran without crashing was $testName"
                }
                Write-Host "##vso[task.logissue type=error]$err"
            }
        }

        $testSuccess = $lastexitcode -eq 0
        if ($testSuccess)
        {
            Write-Verbose "[TEST BINARY PASSED] $testBinaryName"
        }
        else
        {
            Write-Host "##vso[task.logissue type=error][TEST BINARY FAILED] $testBinaryName"
        }

        # We can get rid of this once we move to a version of VSTest that allows us to specify
        # test filenames.
        if ($tempPath)
        {
            $tempFile = Get-ChildItem $tempPath -Recurse -File
            if ($tempFile.Count -eq 0)
            {
                Write-Warning "Test $testBinaryName didn't generate a .trx file"
            }
            elseif ($tempFile.Count -gt 1)
            {
                Write-Warning "Test $testBinaryName generated too many .trx files"
            }
            else
            {
                Move-Item $tempFile[0].FullName $testReportPath | Out-Null
            }
        }

        return $testSuccess
    }
}

function getFilterForFramework($framework)
{
    # Tests built in CppUnitTestFramework allow you to filter on Owner, FullyQualifiedName, ClassName
    # Tests built in CSharp allow you to filter on TestCategory, Priority, FullyQualifiedName, Name, ClassName
    # We only have a standard for filtering out CppUnitTestFramework tests right now
    if ($framework -ne "AppxCpp")
    {
        return $null
    }

    $ignorePlatformFilter = "Ignore" + (Get-Culture).TextInfo.ToTitleCase($Platform.ToLower())
    $ignoreConfigurationFilter = "Ignore" + (Get-Culture).TextInfo.ToTitleCase($Configuration.ToLower())
    return "Owner!=$ignorePlatformFilter&Owner!=$ignoreConfigurationFilter"
}

function removeAllExistingAppxTests($appxTests, $config)
{
    $appxFiles = $appxTests | Foreach-Object { findTestBinary $_ $config }
    if ($Platform -eq "arm")
    {
        $DeviceIp = "127.0.0.1"
    }
    . $UninstallAppx -AppxFiles $appxFiles -DeviceIp $DeviceIp -Verbose:$Verbose
}

function reinstallAllAppxDependencies
{
    if (! $AppxDependencyPath)
    {
        return
    }

    if ($Platform -eq "arm")
    {
        $DeviceIp = "127.0.0.1"
    }

    $appxFiles = (Get-ChildItem $AppxDependencyPath -File -Recurse).FullName
    . $UninstallAppx -AppxFiles $appxFiles -DeviceIp $DeviceIp -Verbose:$Verbose
    . $InstallAppx -AppxFiles $appxFiles -DeviceIp $DeviceIp -Verbose:$Verbose
}

function findTestBinary($test, $config, $extraFailureMessage)
{
    $path = Join-Path (getRootPath $config) (resolveString $test.Path)
    $binary = Get-Item $path
    if (! $binary)
    {
        Write-Host "##vso[task.logissue type=error]No test binary at $path. $extraFailureMessage"
    }
    if ($binary.Count -ne 1)
    {
        Write-Host "##vso[task.logissue type=error]Multiple test binaries at $path"
    }

    # On mac, make sure the test binary artifact is executable and not just a document
    $Runtime = [System.Runtime.InteropServices.RuntimeInformation] 
    $OSPlatform = [System.Runtime.InteropServices.OSPlatform] 
    if ($Runtime::IsOSPlatform($OSPlatform::OSX) -and $test.framework -ne "iOSSimulator")
    {
        chmod a+x $binary
    }

    return $binary
}

function findAppxSettingsFile($config)
{
    if ($Platform -eq "arm")
    {
        $testSetting = "appxSettingsArm"
    }
    else
    {
        $testSetting = "appxSettings"
    }

    if ($config.$testSetting)
    {
        return Join-Path (getRootPath $config) $config.$testSetting
    }

    return $null
}

function getTestReportPath($test)
{
    if (! $ReportPath)
    {
        return $null
    }

    if (! [System.IO.Path]::IsPathRooted($ReportPath))
    {
        $ReportPath = Join-Path $CurrentPath $ReportPath
    }

    $extension = $TestReportExtensions[$test.Framework]
    if (! $extension)
    {
        Write-Error "No report extension defined for $($test.Framework)"
    }

    $testFileName = $test.Name
    if ($Configuration)
    {
        $testFileName += ".$Configuration"
    }
    if ($Platform)
    {
        $testFileName += ".$Platform"
    }
    
    $testFileName += "-" + $test.Framework + ".$extension"
    return (Join-Path $ReportPath $testFileName)
}

function publishTestReports
{
    if (! $ReportPath)
    {
        return
    }

    foreach ($reportFile in (Get-ChildItem -File $ReportPath))
    {
        publishTestReport $reportFile
    }
}

function publishTestReport($reportFile)
{
    if ($reportFile.BaseName -notmatch "^(.+)-(.+)$")
    {
        Write-Warning "Don't know how to handle report file $($reportFile.Fullname)"
        return
    }

    $runTitle = $matches[1]
    $framework = $matches[2]
    $reportType = $TestReportTypes[$framework]

    if (! $reportType)
    {
        Write-Warning "No report type defined for framework $framework"
        return
    }

    $params = "type=$reportType;runTitle=$runTitle;publishRunAttachments=true;resultFiles=$($reportFile.FullName);"
    if (getTestPlatform)
    {
        $params += "platform=$(getTestPlatform);"
    }
    if ($Configuration)
    {
        $params += "configuration=$Configuration"
    }

    Write-Host "##vso[results.publish $params]"
}

function getTestPlatform
{
    if ($Platform -eq "win32")
    {
        return "x86"
    }
    return $Platform
}

function resolveString($string)
{
    $string = $string -replace "<platform>",$Platform
    $string = $string -replace "<configuration>",$Configuration
    return $string
}

function getRootPath($config)
{
    $root = (Get-Item $ConfigFile).Directory.FullName
    if ($config.relativeRoot)
    {
        return Resolve-Path (Join-Path $root $config.relativeRoot)
    }
    return $root
}

main
