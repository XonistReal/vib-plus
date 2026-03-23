param(
    [Parameter(Mandatory = $true)]
    [string]$Message,
    [string]$Branch = "",
    [string]$GitName = "",
    [string]$GitEmail = "",
    [switch]$PullFirst,
    [switch]$NoPush
)

$ErrorActionPreference = "Stop"

$git = "C:\Program Files\Git\cmd\git.exe"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

if (-not (Test-Path $git)) {
    throw "Git not found at $git"
}

Push-Location $root
try {
    if (-not (Test-Path ".git")) {
        throw "This folder is not a git repository: $root"
    }

    if ($Branch -ne "") {
        & $git checkout $Branch
        if ($LASTEXITCODE -ne 0) { throw "Failed to checkout branch '$Branch'." }
    }

    if ($PullFirst) {
        & $git pull --rebase
        if ($LASTEXITCODE -ne 0) { throw "git pull --rebase failed." }
    }

    & $git add -A

    & $git diff --cached --quiet
    if ($LASTEXITCODE -eq 0) {
        Write-Host "No staged changes to commit."
        return
    }

    if ($GitName -ne "" -and $GitEmail -ne "") {
        & $git -c user.name="$GitName" -c user.email="$GitEmail" commit -m $Message
    } else {
        & $git commit -m $Message
    }
    if ($LASTEXITCODE -ne 0) { throw "git commit failed." }

    if (-not $NoPush) {
        & $git push
        if ($LASTEXITCODE -ne 0) { throw "git push failed." }
    } else {
        Write-Host "Commit created locally; push skipped by -NoPush."
    }

    & $git status --short --branch
}
finally {
    Pop-Location
}

