param()

Push-Location $PSScriptRoot\..\ClangTidy_4.27
try {
    if (Test-Path .\Plugins) {
        Remove-Item -Force -Recurse .\Plugins
    }
    New-Item -ItemType Directory .\Plugins
    Push-Location .\Plugins
    try  {
        if ($env:CI -eq "true") {
            git clone --recursive "https://gitlab-ci-token:$env:CI_JOB_TOKEN@gitlab.com/redpointgames/clang-tidy-for-unreal-engine.git" ClangTidy
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
        } else {
            git clone --recursive "git@gitlab.com:redpointgames/clang-tidy-for-unreal-engine.git" ClangTidy
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
        }
        git --git-dir=ClangTidy/.git --work-tree=ClangTidy checkout $(git --git-dir=ClangTidy/.git --work-tree=ClangTidy describe --tags --abbrev=0)
        if ($LastExitCode -ne 0) {
            exit $LastExitCode
        }
        Push-Location ./ClangTidy
        try {
            git submodule update --init --recursive
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            } 
        } finally { 
            Pop-Location 
        }
    } finally {
        Pop-Location
    }
} finally {
    Pop-Location
}
exit 0