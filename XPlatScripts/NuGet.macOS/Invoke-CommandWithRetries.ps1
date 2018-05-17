# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Retry a generic ScriptBlock with incremental backoff.

.DESCRIPTION
Invokes the passed-in $Command. If we hit an exception, we call into $IsErrorRetriable.
If it returns false, we just re-throw the exception. If it retruns true, we retry
with incremental backoff, first sleeping $InitialSleepSeconds and then doubling
that number on every retry up to $MaxSleepSeconds.

.PARAMETER Command
The ScriptBlock to run.

.PARAMETER IsErrorRetriable
A ScriptBlock that takes an ErrorRecord on the command-line (the exception is
available at $errorRecord.Exception). Return $true if we want to retry this error.

.PARAMETER MaxTries
The maximum number of times we'll try $Command before failing.

.PARAMETER InitialSleepSeconds
The number of seconds we'll sleep the first time we hit a retriable exception.

.PARAMETER MaxSleepSeconds
We double InitialSleepSeconds every time we want to retry, up to MaxSleepSeconds.
#>
[CmdletBinding()]
Param(
    [Parameter(Mandatory)]
    [ScriptBlock]$Command,
    [Parameter(Mandatory)]
    [ScriptBlock]$IsErrorRetriable,
    [Parameter()]
    [Int]$MaxTries = 10,
    [Parameter()]
    [Int]$InitialSleepSeconds = 5,
    [Parameter()]
    [Int]$MaxSleepSeconds = 300
)

$ErrorActionPreference = "stop"

function main
{
    $currentTry = 1

    while ($true)
    {
        try
        {
            return Invoke-Command $Command
        }
        catch
        {
            $err = $_
            if ((-not (retriable $err)) -or ($currentTry -ge $MaxTries))
            {
                throw $err
            }
            
            Write-Warning "Caught retriable error ($currentTry of $MaxTries): $err"
            sleepBeforeRetrying $currentTry
            $currentTry += 1
        }
    }
}

function retriable($err)
{
    return Invoke-Command -ArgumentList @($err) -ScriptBlock $IsErrorRetriable
}

function sleepBeforeRetrying($currentTry)
{
    $multiplier = [math]::pow(2, $currentTry-1)
    $seconds = [math]::min($MaxSleepSeconds, $InitialSleepSeconds * $multiplier)
    Write-Verbose "Sleeping for $seconds seconds"
    Start-Sleep -Seconds $seconds
}

main
