<#
.SYNOPSIS
Invokes a list of tests on the iOS Simulator. This returns a list PSObjects containing test success result and test output.

.DESCRIPTION
Runs a list of test binaries locally on a specific iOS Simulator device and runtime. This script 
sets up the iOS Simulator at the beginning and cleanly handles shutting it down after test execution.
It returns a list of PSObjects, each containing the following:
    testBinaryName: name of the test binary
    testSucceeded: boolean result of this test, true if it passed
    output: the output of this test

.PARAMETER DeviceType
This value must be a valid simulator device type, that can be called when prefixed 
with com.appleCoreSimulator.SimDeviceType (e.g. iPhone-7-Plus)
See the list by running 'xcrun simctl list runtimes'

.PARAMETER DeviceRuntime
This must be a valid iOS Simulator Runtime, and must be a valid command
when suffixed with com.apple.CoreSimulator.SimRuntime (e.g. iOS-11-2)
See the list by running 'xcrun simctl list devicetypes'

.PARAMETER TestBinaryPaths
A list of test binaries to install and run on the iOS Simulator

.PARAMETER ReportPath
The directory we will drop all of our test reports.

.PARAMETER TestTimeoutSeconds
Test timeout, in seconds, for a single test.

#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$DeviceType,
    [Parameter(Mandatory)]
    [String]$DeviceRuntime,
    [Parameter(Mandatory)]
    [String[]]$TestBinaryPaths,
    [Parameter()]
    [String]$ReportPath = $env:BUILD_BINARIESDIRECTORY,
    [Parameter()]
    [Int]$TestTimeoutSeconds = 0
)

$ErrorActionPreference = "Stop"

function main
{
    $deviceUDID = launchSimulator
    $testResults = @()
    try
    {
        foreach ($test in $TestBinaryPaths)
        {
            $testResults += runiOSSimulatorTest $test
        }
    }
    finally
    {
        shutdownSimulator $deviceUDID
    }
    return $testResults
}

# Shut down all simulators that may be on. Some of them might be in broken or weird states, 
# so make a new instance ourselves each time. After testing or if exception is hit, we will 
# clean up and delete the simulator in the function shutdownSimulator
function launchSimulator
{
    $xcodePath = getXcodePath
    $simulatorPath = [io.path]::combine($xcodePath, "Applications", "Simulator.app")

    Write-Verbose "Launching Simulator App: open $simulatorPath"
    . open $simulatorPath

    $simDevice = "com.apple.CoreSimulator.SimDeviceType.$DeviceType"
    $simRuntime = "com.apple.CoreSimulator.SimRuntime.$DeviceRuntime"

    . xcrun simctl shutdown all | Out-Null

    Write-Host "Creating a simulator: xcrun simctl create BigParkiOS $simDevice $simRuntime"
    $simulatorUDID = . xcrun simctl create BigParkiOS $simDevice $simRuntime

    if (!$simulatorUDID)
    {
        Write-Error "Error creating simulator $DeviceType, $DeviceRuntime. Please ensure this simulator is installed on the machine by viewing all simulators with 'xcrun simctl list'"
    }

    . xcrun simctl boot $simulatorUDID | Out-Null

    return $simulatorUDID
}

# Each XCode version automatically includes SDKs for specific iOS runtimes. Try to use
# this corresponding version of XCode for a runtime, otherwise default to the active XCode version
function getXcodePath()
{
    if ($DeviceRuntime -match "11-2")
    {
        Write-Verbose "Using Xcode 9.2 to run Simulator with runtime $DeviceRuntime"
        $xcodePath = "/Applications/Xcode_9.2.app/Contents/Developer"
    }
    elseif ($DeviceRuntime -match "11-3")
    {
        Write-Verbose "Using Xcode 9.3 to run Simulator with runtime $DeviceRuntime"
        $xcodePath = "/Applications/Xcode_9.3.app/Contents/Developer"
    }
    if (!$xcodePath -or -not (Test-Path $xcodePath))
    {
        Write-Verbose "Xcode developer path $xcodePath does not exist"
        $xcodePath = . xcode-select -p
        Write-Verbose "Using active Xcode developer directory instead: $xcodePath"
    }
    if (!$xcodePath -or -not (Test-Path $xcodePath))
    {
        Write-Error "No Xcode path exists. Check that Xcode is installed and that 'xcode-select -p' returns a path"
    }
    return $xcodePath
}

function shutdownSimulator($deviceUDID)
{
    Write-Verbose "Deleting $DeviceType Simulator ($deviceUDID)"
    . xcrun simctl shutdown $deviceUDID | Out-Null
    . xcrun simctl erase $deviceUDID | Out-Null
    . xcrun simctl delete $deviceUDID | Out-Null

    $deviceStillPresent = . xcrun simctl list | grep $deviceUDID 
    if ($deviceStillPresent)
    {
        Write-Error "$DeviceType Simulator ($deviceUDID) could not be deleted!" -ErrorAction Continue
    }
}
  
# Wrap the call to launch the iOS app in a timeout job in case it doesn't return
# in the unexpected case something goes wrong with the simulator or the test hangs
function runiOSSimulatorTest($testBinary)
{
    $testBinaryName = Split-Path -Leaf $testBinary

    Write-Verbose "Installing iOS Simulator test $testBinaryName"
    . xcrun simctl install booted $testBinary | Out-Null

    Write-Verbose "Running iOS Simulator test $testBinaryName"
    $appBundleName = . "/usr/libexec/PlistBuddy" -c "print CFBundleIdentifier" "$testBinary/Info.plist"
    
    $outputFileName = (Join-Path $ReportPath "$testBinaryName.txt")
    
    # xcrun simctl launch --stdout to a file is an alternative solution to --console,
    # Using --console will print to stdout and block until it completes. However, --stdout
    # is non-blocking, so we need to wait for the app process to finish before reading output.
    $output = . xcrun simctl launch --stdout="$outputFileName" booted "$appBundleName"
    
    # output is in the form: <appbundlename>: <pid>
    Write-Verbose "app bundle name and pid are $output"
    $appPid = $output.Split()[-1]

    $testOutput = getTestOutput $appPid $outputFileName

    Write-Verbose "iOS Simulator test $testBinaryName completed run"
    Write-Verbose "Test output located at $outputFileName"

    # TODO: implement return code parsing so we can actually determine if the test succeeded or not (e.g. crashed)
    $resultObject = [PSObject]@{ testBinaryName=$testBinaryName; testSucceeded=$true; output=$testOutput}

    return $resultObject
}

# get test output written to $outputFileName after $appPid finishes running
function getTestOutput($appPid, $outputFileName)
{
    waitForProcess $appPid
    waitForOutputWriteToFinish $outputFileName

    return (Get-Content $outputFileName)
}

function waitForProcess($appPid)
{
    $psResult = . ps -p $appPid
    while ($psResult.Count > 1)
    {
        Start-Sleep 5
        $psResult = . ps -p $appPid
    }
}

# The output file is empty immediately after because the output text hasn't yet been populated
# Keep checking the contents of the file to make sure the test has been fully written before actually returning it
# In the case that the test crashed or doesn't finish writing output, we will error out after the timeout

# TODO: look into better ways of telling when writing to the file is complete.
# e.g. Check if the process or parent has a file handler open with lsof, watch out for intermittent writes though
function waitForOutputWriteToFinish($outputFileName)
{
    if (-not (Test-Path $outputFileName))
    {
        Write-Error "There is no output file for $testBinaryName - expecting it at $outputFileName"
    }

    # wait for the file contents to be written for 2 minutes before returning what we have
    $fileWriteTimeout = 120

    Write-Verbose "Waiting for process to finish writing to output file ..."
    $startTime = Get-Date

    while (($(Get-Date) - $startTime).TotalSeconds -lt $fileWriteTimeout)
    {
        $output = (Get-Content $outputFileName)
        foreach ($line in $output)
        {
            if ($line -match "Global test environment tear-down")
            {
                Start-Sleep 5
                return
            }
        }
    }
    Write-Warning "Test process completed but did not reach global tear-down state - it may have crashed or hit an unexpected error. Timed out while writing to output file for $fileWriteTimeout sec".
}

main
