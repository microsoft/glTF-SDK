[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$ConfigFile,
    [Parameter(Mandatory)]
    [String]$PublishPath,
    [Parameter()]
    [String]$Platform,
    [Parameter()]
    [String]$Configuration
)

$ErrorActionPreference = "Stop"

$ExtensionsToIgnore = @(".pdb")
$Git = . (Join-Path $PSScriptRoot "Find-Git.ps1")

function main
{
    if (! (Test-Path $PublishPath))
    {
        New-Item -Type Directory $PublishPath | Out-Null
    }
    $config = Get-Content $ConfigFile | ConvertFrom-Json

    # git operations and `Resolve-Path -Relative` require us to be in the root
    # of the repo we're publishing from
    Push-Location (getRootPath $config)
    try
    {
        publishTestDependencies
        $gitFiles = getGitFiles

        $testsToPublish = $config.Tests | Where-Object { shouldPublishTest $_ }
        $postCommands = $testsToPublish | Foreach-Object { publishTest $_ $config $gitFiles }
        $postCommands | Out-File "$PublishPath/PostPublish.ps1" -Encoding ascii
    }
    finally
    {
        Pop-Location
    }
}

# Find all appx dependencies available under a Dependencies\ folder, and copy
# to a single 'stage' folder that can be used as the dependencies folder for
# a vstest run.
# We do it this way rather than copying test dependencies beside each test for a
# couple reasons:
# * Most tests have the same set of dependencies, so we minimize copying 
#   duplicates.
# * Different tests copy dependencies under different folder patterns.
function publishTestDependencies
{
    $testPlatform = getTestPlatform
    $destFolder = "$PublishPath/Dependencies/$testPlatform"
    New-Item -Type Directory -ErrorAction Ignore -Path $destFolder | Out-Null
    foreach ($dependencyFolder in Get-ChildItem -Directory -Recurse -Filter Dependencies)
    {
        foreach ($dependencyAppx in (Get-Item "$($dependencyFolder.FullName)/$testPlatform/*.appx"))
        {
            Write-Verbose "Copying $($dependencyAppx.FullName) to $destFolder"
            Copy-Item -ErrorAction Ignore $dependencyAppx.FullName $destFolder | Out-Null
        }
    }
}

# We want to publish tests that we may want to run manually (e.g. publish nightly tests for CI builds)
# but don't waste time publishing tests we don't ever run (e.g. don't publish .dll ARM tests).
function shouldPublishTest($test)
{
    if (($Platform -eq "ARM") -and ($test.Framework -notmatch "Appx"))
    {
        return $false
    }

    if ($test.Platforms -and !($test.Platforms -contains $Platform))
    {
        return $false
    }

    if ($test.Configurations -and !($test.Configurations -contains $Configuration))
    {
        return $false
    }

    return $true
}

function publishTest($test, $config, $gitFiles)
{
    foreach ($file in (getFilesToPublish $test $config))
    {
        $checkedInFile = findCheckedInCopyOfFile $file $gitFiles
        if ($checkedInFile)
        {
            Write-Output (getPublishCommandForCheckedInFile $checkedInFile $file)
        }
        else
        {
            $dest = Join-Path $PublishPath (Resolve-Path $file.FullName -Relative)
            Write-Verbose "Copy from $($file.FullName) to $dest"
            New-Item -Type Directory -ErrorAction Ignore (Split-Path -Parent $dest) | Out-Null
            Copy-Item $file.FullName $dest -Force | Out-Null
        }
    }
}

function getFilesToPublish($test, $config)
{
    $testBinary = findTestBinary $test $config
    if ($test.Framework -match "Appx")
    {
        return $testBinary
    }

    $testDirectory = (Get-Item $testBinary).Directory
    $files = Get-ChildItem $testDirectory -Recurse -File
    return $files | Where-Object { shouldPublishFile $_ $test.AlwaysPublishFiles }
}

function shouldPublishFile($file, $alwaysPublishFiles)
{
    $fileFullName = $file.FullName.Replace("\", "/")

    foreach ($alwaysPublishRegex in $alwaysPublishFiles)
    {
        if ($fileFullName -match $alwaysPublishRegex)
        {
            Write-Verbose "Publishing $fileFullName because it matches AlwaysPublishFiles regex $alwaysPublishRegex"
            return $true
        }
    }

    if ($ExtensionsToIgnore -contains $file.Extension)
    {
        Write-Verbose "Not publishing $fileFullName because if its extension."
        return $false
    }

    return $true
}

# Find a file that's checked-in that has the exact same contents as this file.
# As an optimization we only consider files that have the same name as this file.
function findCheckedInCopyOfFile($file, $gitFiles)
{
    # Assume if the file length is the same this is the file we want to use
    # If we really wanted we could use 'git hash-object', but this would take quite a bit longer
    return $gitFiles[$file.Name] | Where-Object { $file.Length -eq $_.Length } | Select-Object -First 1
}

function getPublishCommandForCheckedInFile($checkedInFile, $destFile)
{
    $source = Resolve-Path $checkedInFile.FullName -Relative
    $dest = Resolve-Path $destFile.FullName -Relative
    $destDir = Split-Path -Parent $dest
    Write-Verbose "Adding post-publish command to copy from $source to $dest"
    Write-Output "New-Item -Type Directory `"`$PSScriptRoot/$destDir`" -ErrorAction Ignore | Out-Null"
    Write-Output "Copy-Item `"`$PSScriptRoot/$source`" `"`$PSScriptRoot/$dest`" -Force -ErrorAction Stop"
}

# Assumes we're already in the root directory
function findTestBinary($test, $config)
{
    $path = resolveString $test.Path
    $binary = Get-Item $path
    if (! $binary)
    {
        Write-Error "No test binary at $path"
    }
    if ($binary.Count -ne 1)
    {
        Write-Error "Multiple test binaries at $path"
    }

    return $binary
}

function resolveString($string)
{
    $string = $string -replace "<platform>",$Platform
    $string = $string -replace "<configuration>",$Configuration
    return $string
}

# Builds an index of filenames to a list of checked-in files with that name
# Assumes we're already in the root directory
function getGitFiles
{
    $gitFiles = @{}
    foreach ($relativePath in (&$git ls-files))
    {
        $file = Get-Item $relativePath -Force
        if (! $gitFiles[$file.Name])
        {
            $gitFiles[$file.Name] = @()
        }
        $gitFiles[$file.Name] += @($file)
    }

    return $gitFiles
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

function getTestPlatform
{
    if ($Platform -eq "win32")
    {
        return "x86"
    }
    return $Platform
}

main