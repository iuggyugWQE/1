param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\MM_SimpleCPP

Clone-Repository "eos-online-subsystem" "EOS"
Clone-Repository "matchmaking" "Matchmaking"

exit 0