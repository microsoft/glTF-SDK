#!/bin/bash
# parameter is the file and path
echo "$1 ${@:2}"
powershell -NoProfile -ExecutionPolicy unrestricted -Command ". '$1' ${@:2}; exit $LASTEXITCODE"