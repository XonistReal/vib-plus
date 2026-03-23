param(
    [string]$SdlRoot = "",
    [string]$OutDir = "build-win",
    [ValidateSet("auto", "cl", "clang")]
    [string]$Compiler = "auto",
    [ValidateSet("smoke", "pong", "flappy", "chess", "all")]
    [string]$Target = "all"
)

$ErrorActionPreference = "Stop"

function Resolve-Compiler {
    param([string]$Wanted)
    if ($Wanted -ne "auto") { return $Wanted }
    $cl = Get-Command cl -ErrorAction SilentlyContinue
    if ($null -ne $cl) { return "cl" }
    $clang = Get-Command clang -ErrorAction SilentlyContinue
    if ($null -ne $clang) { return "clang" }
    return "cl"
}

function Resolve-SdlRoot {
    param([string]$Given)
    if ($Given -and (Test-Path $Given)) { return (Resolve-Path $Given).Path }
    if ($env:SDL3_ROOT -and (Test-Path $env:SDL3_ROOT)) { return (Resolve-Path $env:SDL3_ROOT).Path }
    if ($env:SDL2_ROOT -and (Test-Path $env:SDL2_ROOT)) { return (Resolve-Path $env:SDL2_ROOT).Path }
    throw "SDL root not found. Pass -SdlRoot or set SDL3_ROOT."
}

function Resolve-VcVars {
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { return "" }
    $install = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $install) { return "" }
    $vcvars = Join-Path $install "VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $vcvars) { return $vcvars }
    return ""
}

$project = Split-Path -Parent $MyInvocation.MyCommand.Path
$out = Join-Path $project $OutDir
New-Item -ItemType Directory -Force -Path $out | Out-Null

$compiler = Resolve-Compiler -Wanted $Compiler
$sdl = Resolve-SdlRoot -Given $SdlRoot

$includeDir = Join-Path $sdl "include"
$libDirA = Join-Path $sdl "lib\x64"
$libDirB = Join-Path $sdl "lib"
$libDir = if (Test-Path $libDirA) { $libDirA } elseif (Test-Path $libDirB) { $libDirB } else { "" }
if (-not (Test-Path $includeDir)) { throw "SDL include dir missing: $includeDir" }
if (-not $libDir -or -not (Test-Path $libDir)) { throw "SDL lib dir missing under: $sdl\lib\x64 or $sdl\lib" }

$sdlLib = if (Test-Path (Join-Path $libDir "SDL3.lib")) { "SDL3.lib" } elseif (Test-Path (Join-Path $libDir "SDL2.lib")) { "SDL2.lib" } else { "" }
$sdlMainLib = if (Test-Path (Join-Path $libDir "SDL3main.lib")) { "SDL3main.lib" } elseif (Test-Path (Join-Path $libDir "SDL2main.lib")) { "SDL2main.lib" } else { "" }
if (-not $sdlLib) { throw "Could not find SDL3.lib or SDL2.lib in $libDir" }

$srcBackend = Join-Path $project "sdl_backend.c"
$targets = @()
if ($Target -eq "smoke" -or $Target -eq "all") { $targets += @{ src = "smoke_main.c"; exe = "vib_sdl_smoke.exe" } }
if ($Target -eq "pong" -or $Target -eq "all") { $targets += @{ src = "pong_main.c"; exe = "vib_pong.exe" } }
if ($Target -eq "flappy" -or $Target -eq "all") { $targets += @{ src = "flappy_main.c"; exe = "vib_flappy.exe" } }
if ($Target -eq "chess" -or $Target -eq "all") { $targets += @{ src = "chess_main.c"; exe = "vib_chess.exe" } }

Push-Location $project
try {
    foreach ($t in $targets) {
        $src = Join-Path $project $t.src
        $exe = Join-Path $out $t.exe
        if ($compiler -eq "cl") {
            $clCmd = @(
                "cl",
                "/nologo",
                "/O2",
                "/I`"$includeDir`"",
                "`"$srcBackend`"",
                "`"$src`"",
                "/link",
                "/LIBPATH:`"$libDir`"",
                $sdlLib,
                "Shell32.lib",
                "/SUBSYSTEM:WINDOWS",
                "/ENTRY:mainCRTStartup",
                "/OUT:`"$exe`""
            ) -join " "
            $vcvars = Resolve-VcVars
            if ($vcvars) {
                Write-Host "Building $($t.exe) with cl via vcvars64..."
                cmd /c "`"$vcvars`" && $clCmd"
                if ($LASTEXITCODE -ne 0) { throw "Build failed for $($t.exe) (cl)." }
            } else {
                Write-Host "Building $($t.exe) with cl (PATH already configured)..."
                Invoke-Expression $clCmd
                if ($LASTEXITCODE -ne 0) { throw "Build failed for $($t.exe) (cl)." }
            }
        } else {
            $sdlName = if ($sdlLib -eq "SDL3.lib") { "SDL3" } else { "SDL2" }
            $cmd = @(
                "clang",
                "-O2",
                "-I", "`"$includeDir`"",
                "`"$srcBackend`"",
                "`"$src`"",
                "-L", "`"$libDir`"",
                "-l$sdlName",
                "-lShell32",
                "-mwindows",
                "-o", "`"$exe`""
            ) -join " "
            Write-Host "Building $($t.exe) with clang..."
            Invoke-Expression $cmd
            if ($LASTEXITCODE -ne 0) { throw "Build failed for $($t.exe) (clang)." }
        }
    }
}
finally {
    Pop-Location
}

$dllCandidates = @(
    (Join-Path $sdl "lib\x64\SDL3.dll"),
    (Join-Path $sdl "lib\x64\SDL2.dll"),
    (Join-Path $sdl "bin\SDL3.dll"),
    (Join-Path $sdl "bin\SDL2.dll"),
    (Join-Path $sdl "SDL3.dll"),
    (Join-Path $sdl "SDL2.dll")
)
foreach ($dll in $dllCandidates) {
    if (Test-Path $dll) {
        Copy-Item $dll -Destination (Join-Path $out (Split-Path $dll -Leaf)) -Force
        break
    }
}

Write-Host ""
Write-Host "Build complete in:"
Write-Host "  $out"
Write-Host ""
Write-Host "Run examples:"
Write-Host "  `"$out\\vib_pong.exe`""
Write-Host "  `"$out\\vib_flappy.exe`""
Write-Host "  `"$out\\vib_chess.exe`""

