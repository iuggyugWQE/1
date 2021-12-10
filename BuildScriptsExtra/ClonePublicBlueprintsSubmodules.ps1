param()

Push-Location $PSScriptRoot\..\ExampleBP_EOS_4.26
try {
    if (Test-Path .\Plugins) {
        Remove-Item -Force -Recurse .\Plugins
    }
    New-Item -ItemType Directory .\Plugins
    Push-Location .\Plugins
    try  {
        if ($env:CI -eq "true") {
            git clone --recursive "https://gitlab-ci-token:$env:CI_JOB_TOKEN@gitlab.com/redpointgames/eos-online-subsystem.git" EOS
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
            git clone --recursive "https://gitlab-ci-token:$env:CI_JOB_TOKEN@gitlab.com/redpointgames/online-subsystem-blueprints.git" OSB
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
        } else {
            git clone --recursive "git@gitlab.com:redpointgames/eos-online-subsystem.git" EOS
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
            git clone --recursive "git@gitlab.com:redpointgames/online-subsystem-blueprints.git" OSB
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            }
        }
        git --git-dir=EOS/.git --work-tree=EOS checkout $(git --git-dir=EOS/.git --work-tree=EOS describe --tags --abbrev=0)
        if ($LastExitCode -ne 0) {
            exit $LastExitCode
        }
        Push-Location ./EOS
        try {
            git submodule update --init --recursive
            if ($LastExitCode -ne 0) {
                exit $LastExitCode
            } 
        } finally { 
            Pop-Location 
        }
        git --git-dir=OSB/.git --work-tree=OSB checkout $(git --git-dir=OSB/.git --work-tree=OSB describe --tags --abbrev=0)
        if ($LastExitCode -ne 0) {
            exit $LastExitCode
        }
        Push-Location ./OSB
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