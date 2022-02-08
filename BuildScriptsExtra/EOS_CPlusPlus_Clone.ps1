param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\EOS_CPlusPlus

Clone-Repository "eos-online-subsystem" "EOS"

exit 0