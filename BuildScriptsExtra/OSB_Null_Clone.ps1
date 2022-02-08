param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\OSB_Null

Clone-Repository "online-subsystem-blueprints" "OSB"

exit 0