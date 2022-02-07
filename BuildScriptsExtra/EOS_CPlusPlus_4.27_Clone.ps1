param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\EOS_CPlusPlus_4.27

Clone-Repository "eos-online-subsystem" "EOS"

exit 0