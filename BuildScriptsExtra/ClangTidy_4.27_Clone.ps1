param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\ClangTidy_4.27

Clone-Repository "clang-tidy-for-unreal-engine" "ClangTidy"

exit 0