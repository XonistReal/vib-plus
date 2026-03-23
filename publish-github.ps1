param(
    [Parameter(Mandatory = $true)]
    [string]$RepoName,
    [Parameter(Mandatory = $true)]
    [string]$GitName,
    [Parameter(Mandatory = $true)]
    [string]$GitEmail,
    [string]$Visibility = "public"
)

$ErrorActionPreference = "Stop"

$git = "C:\Program Files\Git\cmd\git.exe"
$gh = "C:\Program Files\GitHub CLI\gh.exe"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

if (-not (Test-Path $git)) { throw "Git not found at $git" }
if (-not (Test-Path $gh)) { throw "GitHub CLI not found at $gh" }

Push-Location $root
try {
    & $gh auth status *> $null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "GitHub login required. Follow the browser/device prompts..."
        & $gh auth login --hostname github.com --git-protocol https --web
    }

    & $git add -A
    & $git -c user.name="$GitName" -c user.email="$GitEmail" commit -m "Initial Vib+ language and SDL game runtime setup."
    if ($LASTEXITCODE -ne 0) {
        Write-Host "No new commit created (possibly nothing changed). Continuing..."
    }

    if ($Visibility -ne "public" -and $Visibility -ne "private") {
        throw "Visibility must be 'public' or 'private'."
    }

    & $gh repo create $RepoName --source . --remote origin --$Visibility --push
}
finally {
    Pop-Location
}

