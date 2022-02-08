param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\OSB_EpicEOS

Clone-Repository "online-subsystem-blueprints" "OSB"

exit 0