param()

. $PSScriptRoot\CloneHelper.ps1

Set-Location $PSScriptRoot\..\ClangTidy

Clone-Repository "clang-tidy-for-unreal-engine" "ClangTidy"

exit 0