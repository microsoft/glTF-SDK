<#
.SYNOPSIS
Summarizes test failures by analyzing the output .trx and .xml files.

.DESCRIPTION
Provides 

.PARAMETER ReportPath
The folder to recursively search for .trx and .xml files

.INPUTS
Example format of GoogleTest .xml files are:

<testsuites tests="268" failures="17" disabled="0" errors="0" timestamp="2018-03-01T15:09:46" time="22.15" name="AllTests">
  <testsuite name="SpectreIfStreamTest" tests="10" failures="8" disabled="0" errors="0" time="0.074">
    <testcase name="SpectreIfStreamTest_ReadWriteAccess__none" status="run" time="0" classname="SpectreIfStreamTest" />
    <testcase name="SpectreIfStreamTest_PositionAndLength__none" status="run" time="0.001" classname="SpectreIfStreamTest">
      <failure message="C:\BA\74\s\CoreUtils\Shared\TestUtils\cpp\Inc\TestUtils/BigparkUnitTest.h:219&#x0A; hit exception"></failure>
      ...
    </testcase>
    ...
  </testsuite>
  ...
</testsuites>

-----
Example format of VSTest .trx files are:

<TestRun id="<guid>" name="bpbeibld@BP-SEGBUILD-20 2018-03-01 15:09:56" runUser="NORTHAMERICA\bpbeibld" xmlns="http://microsoft.com/schemas/VisualStudio/TeamTest/2010">
  <Results>
    <UnitTestResult testName="InitialiseLoader_Default" computerName="BP-SEGBUILD-20" duration="00:00:00.0119040" outcome="Passed"/>
    <UnitTestResult testName="Loader_ConvertAsync" computerName="BP-SEGBUILD-20" duration="00:00:00.1728856" outcome="Failed">
      <Output>
        <ErrorInfo>
          <Message>Test method Lift.Spectre.Loader.Test.LoaderCSTests.Loader_ConvertAsync threw exception blah</Message>
          <StackTrace> at System.IO.__Error.WinIOError(Int32 errorCode, String maybeFullPath) </StackTrace>
        </ErrorInfo>
      </Output>
    </UnitTestResult>
    ...
  </Results>
  <TestDefinitions>
    <UnitTest name="InitialiseLoader_Default">
      <TestMethod className="Lift.Spectre.Loader.Test.LoaderCSTests"/>
    </UnitTest>
    ...
  </TestDefinitions>
  <ResultSummary outcome="Failed">
    <Counters total="31" executed="31" passed="15" failed="16" error="0" timeout="0" aborted="0" inconclusive="0" passedButRunAborted="0" notRunnable="0" notExecuted="0" disconnected="0" warning="0" completed="0" inProgress="0" pending="0" />
  </ResultSummary>
</TestRun>

Copyright (C) 2017 Microsoft.
#>

[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$ReportPath
)

$ErrorActionPreference = "Stop"
function main()
{
    Write-Host @"
Note: to view errors in the Run Tests build task, search its log for [TEST FAILED] or [TEST BINARY FAILED] to in the log to find where we failed."
This task is a summary of the erors hit."

Analyzing .xml and .trx files in $ReportPath
"@

    writeErrorsFromXmlFiles

    # Make sure this task fails, but without extra write-error fluff
    exit 1
}

function writeErrorsFromXmlFiles()
{
    $gtestXmlPaths = Get-ChildItem -path $ReportPath -Include *.xml -Recurse | % { $_.FullName }
    $vstestTrxPaths = Get-ChildItem -path $ReportPath -Include *.trx -Recurse | % { $_.FullName }

    foreach ($xmlPath in $gtestXmlPaths)
    {
        writeErrorsFromGTestXmlFile $xmlPath
    }
    foreach ($trxPath in $vstestTrxPaths)
    {
        writeErrorsFromTrxFile $trxPath
    }
}

function writeErrorsFromGTestXmlFile($xmlPath)
{
    Write-Verbose $xmlPath
    $testSuites = ([xml](Get-Content -LiteralPath $xmlPath)).testsuites.testsuite
    $failedTestSuites = $testSuites | Where-Object { $_.failures -gt 0 }
    $fullErrorMessage = @()

    foreach ($testSuite in $failedTestSuites)
    {
        $failedTestCases = $testSuite.testcase | Where-Object { $_.failure -ne $null }
        $fullErrorMessage += @"

-----
$($testSuite.name) (GoogleTest) contained $($testSuite.failures) failure(s) of $($testSuite.tests) tests
"@

        foreach ($testCase in $failedTestCases)
        {
            $fullErrorMessage += @"

$($testCase.name) in class $($testCase.classname) failed, duration $($testCase.time)
"@
            $failures = $testCase.failure | Select-Object -Unique
            $failures | ForEach-Object { 
                $errorMessage = $_.message -replace "`n", "`n`t"
                $fullErrorMessage += "

Error Message:
    $errorMessage
"
            }
        }
    }
    writeMultilineMessage $fullErrorMessage
}

function writeErrorsFromTrxFile($trxPath)
{
    Write-Verbose $trxPath
    $testRun = ([xml](Get-Content -LiteralPath $trxPath)).TestRun
    $fullErrorMessage += @()

    if ($testRun.ResultSummary.outcome -ne "Failed")
    {
        return
    }

    $testSuite = $testRun.TestDefinitions.UnitTest.TestMethod[0].className
    $buildMachine = $testRun.Results.UnitTestResult[0].computerName
    $totalNumTests = $testRun.ResultSummary.Counters.total
    $totalFailedTests = $testRun.ResultSummary.Counters.failed
    $fullErrorMessage += @"

-----
$testSuite (vsTest) contained $totalFailedTests failure(s) of $totalNumTests tests, on build machine $buildMachine
"@

    $failedTestCases = $testRun.Results.UnitTestResult | Where-Object { $_.outcome -eq "Failed" }
    
    foreach ($testCase in $failedTestCases)
    {
        $testCaseName = $testCase.testName
        $testCaseDuration = $testCase.duration
        $fullErrorMessage += @"

$testCaseName failed, duration: $testCaseDuration
"@

        $errorMessage = $testCase.Output.ErrorInfo.Message -replace "`n", "`n`t"
        $stackTrace = $testCase.Output.ErrorInfo.StackTrace -replace "`n", "`n`t"
        
        $fullErrorMessage += @"

Error Message:
    $errorMessage
Stack Trace:
    $stackTrace
"@

    }
    writeMultilineMessage $fullErrorMessage
}

function writeMultilineMessage([string]$message)
{
    foreach ($line in $message.Split("`n"))
    {
        Write-Host $line
    }
}

main