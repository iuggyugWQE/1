param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\RSI

Clone-Repository "replicated-sublevel-instances" "RSI"

exit 0