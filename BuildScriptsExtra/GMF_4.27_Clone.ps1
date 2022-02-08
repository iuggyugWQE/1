param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\GMF_4.27

Clone-Repository "game-management-framework" "GMF"

exit 0