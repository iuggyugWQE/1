param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\OSB_LoginUI

Clone-Repository "online-subsystem-blueprints" "OSB"

exit 0