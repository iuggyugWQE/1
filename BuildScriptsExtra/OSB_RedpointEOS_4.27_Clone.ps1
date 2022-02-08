param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\OSB_RedpointEOS_4.27

Clone-Repository "eos-online-subsystem" "EOS"
Clone-Repository "online-subsystem-blueprints" "OSB"

exit 0