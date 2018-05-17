#!/usr/local/bin/powershell

# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

pushd $PSScriptRoot

$ScriptPath = "$PSScriptRoot/../../XPlatScripts/NuGet.macOS/NuGetRestore.ps1"

$Args = "-Verbose -PackagesConfigRoot ../../ -NuGetConfig ../../NuGet.Config -PackagesPath ../../packages"

Invoke-Expression "& $ScriptPath $Args"

popd
