param()

function Clone-Repository($ProjectName, $FolderName) {
    if (!(Test-Path .\Plugins)) {
        New-Item -ItemType Directory .\Plugins
    }
    Push-Location .\Plugins
    try {
        Write-Host ("Updating repository in " + $((Get-Location).Path) + "\$FolderName")
        if (!(Test-Path .\$FolderName)) {
            if ($env:CI -eq "true") {
                git clone --recursive "https://gitlab-ci-token:$env:CI_JOB_TOKEN@gitlab.com/redpointgames/$ProjectName.git" $FolderName
                if ($LastExitCode -ne 0) {
                    Write-Host "git clone failed with exit code $LastExitCode"
                    exit $LastExitCode
                }
            } else {
                git clone --recursive "git@gitlab.com:redpointgames/$ProjectName.git" $FolderName
                if ($LastExitCode -ne 0) {
                    Write-Host "git clone failed with exit code $LastExitCode"
                    exit $LastExitCode
                }
            }
        } else {
            if ($env:CI -eq "true") {
                git --git-dir=$FolderName/.git --work-tree=$FolderName remote set-url origin "https://gitlab-ci-token:$env:CI_JOB_TOKEN@gitlab.com/redpointgames/$ProjectName.git"
                if ($LastExitCode -ne 0) {
                    Write-Host "git remote set-url origin failed with exit code $LastExitCode"
                    exit $LastExitCode
                }
            } else {
                git --git-dir=$FolderName/.git --work-tree=$FolderName remote set-url origin "git@gitlab.com:redpointgames/$ProjectName.git"
                if ($LastExitCode -ne 0) {
                    Write-Host "git remote set-url origin failed with exit code $LastExitCode"
                    exit $LastExitCode
                }
            }
            git --git-dir=$FolderName/.git --work-tree=$FolderName fetch --all -P -t
            if ($LastExitCode -ne 0) {
                Write-Host "git fetch --all failed with exit code $LastExitCode"
                exit $LastExitCode
            }
        }
        $LatestTag = (git --git-dir=$FolderName/.git --work-tree=$FolderName tag -l --sort=-refname | Out-String).Split("`n")[0].Trim()
        if ($LastExitCode -ne 0) {
            Write-Host "git tag -l failed with exit code $LastExitCode"
            exit $LastExitCode
        }
        if ($LatestTag -eq "" -or $null -eq $LatestTag) {
            $LatestTag = "main"
        } else {
            $LatestTag = "tags/$LatestTag"
        }
        git --git-dir=$FolderName/.git --work-tree=$FolderName checkout $LatestTag
        if ($LastExitCode -ne 0) {
            Write-Host "git checkout failed with exit code $LastExitCode"
            exit $LastExitCode
        }
        Push-Location ./$FolderName
        try {
            git submodule update --init --recursive
            if ($LastExitCode -ne 0) {
                Write-Host "git submodule update --init --recursive with exit code $LastExitCode"
                exit $LastExitCode
            } 
        } finally { 
            Pop-Location 
        }
    } finally {
        Pop-Location
    }
}