<#
.SYNOPSIS
Run WACK tests against a single appx package.

.DESCRIPTION
Makes the necessary calls to appcert.exe to run WACK tests against
an appx package. Parses the resulting report to fail if any tests
failed that we don't want to ignore.

.PARAMETER PackageToTest
The path to the .appx or .appxbundle package we want to run WACK
tests against.

.PARAMETER ReportDirectory
A directory where we will drop WACK test results. We'll also try to
convert results to an .html file, which we'll drop in the same
directory. Right now we just drop a wackreport.xml file in the report
directory, and if that file already exists this script will fail.

.PARAMETER ConfigFile
A .json file containing a list of tests we want to ignore.
#>
#Requires -RunAsAdministrator
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [String]$PackageToTest,
    [Parameter(Mandatory)]
    [String]$ReportDirectory,
    [Parameter(Mandatory)]
    [String]$ConfigFile
)

$ErrorActionPreference = "Stop"
$AppCert = Join-Path ${Env:ProgramFiles(x86)} "Windows Kits/10/App Certification Kit/appcert.exe"

function main
{
    $config = Get-Content $ConfigFile | ConvertFrom-Json

    $reportFile = Join-Path $ReportDirectory "wackreport.xml"
    $packageFile = (Get-Item $PackageToTest).FullName

    resetWackState
    runWackTest $reportFile $packageFile
    processWackReport $reportFile $config
}

function resetWackState
{
    runAppCert "reset" | Out-Null
}

function runWackTest($reportFile, $packageFile)
{
    $exitCode = runAppCert @("test", "-appxpackagepath", $packageFile, "-reportoutputpath", $reportFile)
    handleWackExit $exitCode
}

function runAppCert($wackArgs)
{
    Write-Verbose "Running: AppCert.exe $wackArgs"

    $procInfo = New-Object System.Diagnostics.ProcessStartInfo
    $procInfo.CreateNoWindow = $true
    $procInfo.UseShellExecute = $false
    $procInfo.RedirectStandardOutput = $true
    $procInfo.RedirectStandardError = $true
    $procInfo.FileName = $AppCert
    $procInfo.Arguments = $wackArgs

    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $procInfo
    $proc.Start() | Out-Null
    $proc.StandardOutput.ReadToEnd() | Write-Host
    $proc.WaitForExit() | Out-Null
    [int]$exitCode = $proc.ExitCode
    Write-Verbose "AppCert.exe exit code: $exitCode"
    return $exitCode
}

function processWackReport($reportFile, $config)
{
    uploadFile $reportFile
    $htmlFile = convertReportToHtml $reportFile
    if ($htmlFile)
    {
        uploadFile $htmlFile
    }
    failOnReportErrors $reportFile $config
}

function uploadFile($file)
{
    Write-Verbose "Uploading $file to WACK artifact folder."
    # If we're running under VSTS this will upload the report as an attachment
    Write-Host "##vso[artifact.upload artifactname=WACK]$file"
}

function convertReportToHtml($reportFile)
{
    [xml]$xml = Get-Content $reportFile
    if (! ($xml."xml-stylesheet" -match "href='(.+)'"))
    {
        Write-Warning "No stylesheet available to convert report to html."
        return
    }
    $xslFile = $matches[1]
    $htmlFile = "$reportFile.html"
    
    Write-Verbose "Converting WACK report to HTML at $htmlFile"
    $xsl = New-Object System.Xml.Xsl.XslCompiledTransform
    $xslSettings = New-Object System.Xml.Xsl.XsltSettings
    $xslSettings.EnableDocumentFunction = $true

    $xsl.Load($xslFile, $xslSettings, (New-Object System.Xml.XmlUrlResolver)) | Out-Null
    $xsl.Transform($reportFile, $htmlFile) | Out-Null
    return $htmlFile
}

function failOnReportErrors($reportFile, $config)
{
    [xml]$xml = Get-Content $reportFile
    $failedTests = $xml.REPORT.REQUIREMENTS.REQUIREMENT.TEST | Where-Object { $_.RESULT."#cdata-section" -ne "PASS" }

    $notIgnored = @()
    foreach ($test in $failedTests)
    {
        $ignoreConfig = $config.testsToIgnore | Where-Object { $_.id -eq $test.INDEX }
        if ($ignoreConfig)
        {
            Write-Verbose "Test '$($test.NAME)' failed, but we are ignoring it because $($ignoreConfig.ignoreReason)"
        }
        else
        {
            $notIgnored += @($test)
        }
    }

    if ($notIgnored)
    {
        $messages = $notIgnored | Foreach-Object { "$($_.NAME): $($_.DESCRIPTION) $($_.MESSAGES.MESSAGE.TEXT -join '. ')" }
        Write-Error "Failed WACK tests. See attached report for more details.`n$($messages -join "`n")"
    }
}

# Running 'appcert' without any parameters provides a decoding of exit codes
function handleWackExit($exitCode)
{
    if ($exitCode -eq -1)
    {
        Write-Error "Invalid command line error occurred."
    }
    if ($exitCode -eq -2)
    {
        Write-Error "Infrastructure error occurred." 
    }
    if ($exitCode -eq -3)
    {
        Write-Error "User initiated error occurred." 
    }
    if ($exitCode -eq -4)
    {
        Write-Error "App installation error occurred." 
    }
    if ($exitCode -eq -5)
    {
        Write-Error "App unpackaging error occurred." 
    }
    if ($exitCode -lt 0)
    {
        Write-Error "Unknown error occurred, exit code $exitCode."
    }
}

main