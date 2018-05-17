[CmdletBinding()]
param(
    [Parameter()]
    [String]$Platform,
    [Parameter()]
    [String]$Configuration,
    [Parameter()]
    [String]$ApiLevel,
    [Parameter()]
    [String]$ReportPath,
    [Parameter()]
    [Object]$Tests
)

$DEST_PATH="/data/local/tmp"

function main()
{
    $abi = findAbi
    $AVD_NAME="AndroidAVD_$($abi)_$($Configuration)"
    $ROOT_PATH = "$PSScriptRoot\..\.."
    return (runTests $abi)
}

function findAbi()
{
    $abi = "x86"
    if ($Platform -eq "arm32")
    {
        $abi = "armeabi-v7a"
    }
    elseif ($Platform -eq "arm64")
    {
        $abi = "arm64-v8a"
    }
    elseif ($Platform -eq "x86_64")
    {
        $abi = "x86_64"
    }
    return $abi
}

function runTests($abi)
{
    $testResult = @()
    launchAvd $abi
    foreach ($test in $Tests)
    {
        $testDir = $test.Name
        $testName = $test.Name
        $outputFilename = "$($testName).$($Configuration).android_$($abi)-GoogleTest.xml"
        if (Test-Path $ReportPath\$outputFilename)
        {
            Remove-Item -Path $ReportPath\$outputFilename
        }
        pushTest $testDir $testName
        $lines = runTest $testDir $testName $outputFilename
        $testResult += [PSObject]@{ testBinaryName=$testName; testSucceeded=$true; output=$lines}
    }
    cleanAndDeleteEmulator
    return $testResult
}

function launchAvd($abi)
{
    cleanAndDeleteEmulator
    Write-Verbose "launching avd"
    $package = "system-images;android-$ApiLevel;google_apis;$abi"
    echo no | avdmanager create avd -n $AVD_NAME -k $package -f | Write-Host
    Write-Verbose "avd $($AVD_NAME) created"
    start powershell "emulator -port 5554 -partition-size 900 -avd $AVD_NAME -no-audio -verbose -no-snapshot -wipe-data"
    Write-Verbose "adb emulator-5554 waiting for device"
    adb -s emulator-5554 wait-for-device | Write-Host
    Write-Verbose "emulator-5554 booting"
    $state = adb -s emulator-5554 shell getprop sys.boot_completed
    $timeout = New-TimeSpan -Minutes 2
    $customTimer = [diagnostics.stopwatch]::StartNew()
    while (($customTimer.elapsed -lt $timeout) -and ($state -ne 1))
    {
        if ($state -like "*error*")
        {
            cleanAndDeleteEmulator
            Write-Error "$($state), unable to start emulator"
        }
        Start-Sleep -Seconds 2
        $state = adb -s emulator-5554 shell getprop sys.boot_completed
    }
    $customTimer.Stop()
    if ($state -ne 1)
    {
        cleanAndDeleteEmulator
        Write-Error "2 minutes timeout. Emulator state: $($state). Unable to start emulator within 2 minutes"
    }
    Write-Verbose "emulator-5554 boot completed."
}

function pushTest($testDir, $testName)
{
    Write-Verbose "pushing test $testName"
    adb -s emulator-5554 shell rm -rf $DEST_PATH/* | Write-Host
    adb -s emulator-5554 push $ROOT_PATH\Built\Out\android_$abi\$Configuration\$testDir $DEST_PATH/ | Write-Host
    adb -s emulator-5554 shell chmod 777 $DEST_PATH/$testDir/$testName | Write-Host
}

function runTest($testDir, $testName, $outputFilename)
{
    Write-Verbose "running test $testName"
    adb -s emulator-5554 shell "cd $DEST_PATH/$testDir; ./$testName --gtest_output=xml:$outputFilename" | Tee-Object -Variable outputLog | Write-Host
    Write-Verbose "finished running $testName"
    adb -s emulator-5554 pull $DEST_PATH/$testDir/$outputFilename $ReportPath | Write-Host
    return $outputLog
}

function cleanAndDeleteEmulator()
{
    Write-Verbose "cleaning and deleting emulator-5554"
    $deviceList = adb devices
    if ($deviceList -like "*emulator-5554*")
    {
        adb -s emulator-5554 shell rm -rf $DEST_PATH/* | Write-Host
        adb -s emulator-5554 shell wipe data | Write-Host
        adb -s emulator-5554 emu kill | Write-Host
    }
    $avdList = avdmanager list avd
    if ($avdList -like "*$($AVD_NAME)*")
    {
        avdmanager delete avd -n $AVD_NAME | Write-Host
    }
    Write-Verbose "cleaning and deleting emulator-5554 completed."
}

function resolveString($string, $abi)
{
    $string = $string -replace "<abi>", $abi
    $string = $string -replace "<configuration>", $Configuration
    return $string
}

main