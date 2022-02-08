param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\GMF

Clone-Repository "game-management-framework" "GMF"

exit 0