#!/usr/bin/env pwsh
#Requires -Version 5

param([Parameter(Mandatory=$true)][string]$FolderName, [Parameter(Mandatory=$true)][string] $ProjectName, [Parameter(Mandatory=$true)] $EnginePath, [switch][bool]$Deploy)

$process = $null

trap {
    if ($process -ne $null -and -not $process.HasExited) {
        $process.Kill();
    }
    Write-Error $_
    exit 1
}

$ProjectRoot = (Resolve-Path -Path (Join-Path $PSScriptRoot "..")).Path

Write-Host ">> INFO: Using Unreal Engine located at '$EnginePath'"

function Wait-For-Process-Exit($Process) {
    while (!$Process.HasExited) {
        Start-Sleep -Seconds 1
    }
}

$ErrorActionPreference = "Stop"

$cwd = Get-Location

Write-Host ">> STEP: Clean output"
if (Test-Path "$ProjectRoot\$FolderName\OutputServer") {
    Remove-Item -Recurse -Force "$ProjectRoot\$FolderName\OutputServer"
}
New-Item -ItemType Directory "$ProjectRoot\$FolderName\OutputServer"

Write-Host ">> STEP: Build editor"
Set-Content -Value "" -Path "$ProjectRoot\AgonesResources\Empty.txt"
$process = Start-Process `
    -FilePath "$EnginePath\Engine\Binaries\DotNET\UnrealBuildTool.exe" `
    -ArgumentList @(
        "Development",
        "Win64",
        "-Project=`"$ProjectRoot\$FolderName\$ProjectName.uproject`"",
        "-TargetType=Editor",
        "-Progress",
        "-NoHotReloadFromIDE"
    ) `
    -NoNewWindow `
    -PassThru
$handle = $process.Handle
Wait-For-Process-Exit -Process $process
if ($process.ExitCode -ne 0) {
    Write-Error -Message ("General pre-build failed (got exit code: " + $process.ExitCode + ")!")
    exit 1
}

Write-Host ">> STEP: Build server (Development)"
$process = Start-Process `
    -FilePath "$EnginePath\Engine\Build\BatchFiles\RunUAT.bat" `
    -ArgumentList @(
        "BuildCookRun",
        "-project=$ProjectRoot\$FolderName\$ProjectName.uproject",
        "-noP4",
        "-platform=Win64",
        "-clientconfig=Development",
        "-serverconfig=Development",
        "-cook",
        "-server",
        "-serverplatform=Linux",
        "-noclient",
        "-allmaps",
        "-build",
        "-stage",
        "-pak",
        "-archive",
        "-archivedirectory=$ProjectRoot\$FolderName\OutputServer",
        "-unattended"
    ) `
    -NoNewWindow `
    -PassThru `
    -RedirectStandardInput "$ProjectRoot\AgonesResources\Empty.txt"
$handle = $process.Handle
Wait-For-Process-Exit -Process $process
if ($process.ExitCode -ne 0) {
    Write-Error -Message "General build failed!"
    exit 1
}

if ($Deploy) {
    Push-Location $PSScriptRoot
    try {
        & skaffold run
    } finally {
        Pop-Location
    }
}

Write-Host ">> STEP: All done!"

exit 0