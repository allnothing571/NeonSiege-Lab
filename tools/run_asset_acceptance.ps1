<#
.SYNOPSIS
    Compile and run the standalone audio asset acceptance harness.

.DESCRIPTION
    The harness (tools/asset_acceptance_harness.cpp) is intentionally not part
    of the CMake build, so building an audio asset set never requires editing
    CMakeLists.txt. This script compiles it against the already built
    neon_sdl.lib / neon_core.lib / SDL static libraries and runs it against a
    real assets directory.

    The default assets directory is a staging copy, so the script never touches
    the files in the source tree.

.PARAMETER Configuration
    Release or Debug. Must match a configuration that has already been built.

.PARAMETER AssetsRoot
    Directory that contains audio/ui, audio/combat and audio/system. Defaults
    to <build>/<Configuration>/assets, which the NeonSiege post-build step
    populates from the source tree.

.PARAMETER BuildDir
    CMake build directory. Defaults to out/build/x64-Publish relative to the
    repository root.

.PARAMETER SourceTree
    When set, the harness runs against this repository's assets/ directory
    instead of the staged copy. The harness restores every file it parks, but
    the staging copy is safer and is the default.
#>

[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',

    [string]$AssetsRoot,

    [string]$BuildDir,

    [switch]$SourceTree
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $BuildDir) {
    $BuildDir = Join-Path $repoRoot 'out\build\x64-Publish'
}
$BuildDir = (Resolve-Path -LiteralPath $BuildDir).Path

$vsDevCmd = 'D:\VS2022\Common7\Tools\VsDevCmd.bat'
if (-not (Test-Path -LiteralPath $vsDevCmd)) {
    throw "VsDevCmd.bat not found at $vsDevCmd"
}

$outDir = Join-Path $repoRoot ("out\asset_acceptance\{0}" -f $Configuration)
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$exePath = Join-Path $outDir 'asset_acceptance_harness.exe'
$sourcePath = Join-Path $PSScriptRoot 'asset_acceptance_harness.cpp'

# Static library paths for the requested configuration.
$sdlRelease = Join-Path $BuildDir '_deps\sdl2-build\Release'
$sdlDebug = Join-Path $BuildDir '_deps\sdl2-build\Debug'
$sdlImageRelease = Join-Path $BuildDir '_deps\sdl2_image-build\Release'
$sdlImageDebug = Join-Path $BuildDir '_deps\sdl2_image-build\Debug'
$sdlMixerRelease = Join-Path $BuildDir '_deps\sdl2_mixer-build\Release'
$sdlMixerDebug = Join-Path $BuildDir '_deps\sdl2_mixer-build\Debug'
$sdlTtfRelease = Join-Path $BuildDir '_deps\sdl2_ttf-build\Release'
$sdlTtfDebug = Join-Path $BuildDir '_deps\sdl2_ttf-build\Debug'

if ($Configuration -eq 'Release') {
    $libs = @(
        (Join-Path $BuildDir 'Release\neon_sdl.lib'),
        (Join-Path $BuildDir 'Release\neon_core.lib'),
        (Join-Path $sdlMixerRelease 'SDL2_mixer-static.lib'),
        (Join-Path $sdlImageRelease 'SDL2_image-static.lib'),
        (Join-Path $sdlTtfRelease 'SDL2_ttf.lib'),
        (Join-Path $sdlRelease 'SDL2-static.lib')
    )
    $runtime = '/MD'
} else {
    $libs = @(
        (Join-Path $BuildDir 'Debug\neon_sdl.lib'),
        (Join-Path $BuildDir 'Debug\neon_core.lib'),
        (Join-Path $sdlMixerDebug 'SDL2_mixer-staticd.lib'),
        (Join-Path $sdlImageDebug 'SDL2_image-staticd.lib'),
        (Join-Path $sdlTtfDebug 'SDL2_ttfd.lib'),
        (Join-Path $sdlDebug 'SDL2-staticd.lib')
    )
    $runtime = '/MDd'
}

$missing = $libs | Where-Object { -not (Test-Path -LiteralPath $_) }
if ($missing) {
    throw ("missing build output(s); build configuration {0} first:`n  {1}" -f `
        $Configuration, ($missing -join "`n  "))
}

if (-not $AssetsRoot) {
    $AssetsRoot = Join-Path $BuildDir ("{0}\assets" -f $Configuration)
}
if ($SourceTree) {
    $AssetsRoot = Join-Path $repoRoot 'assets'
}
if (-not (Test-Path -LiteralPath $AssetsRoot)) {
    throw "assets root not found: $AssetsRoot"
}
$AssetsRoot = (Resolve-Path -LiteralPath $AssetsRoot).Path

$includeDirs = @()

# SDL headers come from the dependency checkouts; neon_sdl itself compiles
# against the generated include tree, but the public SDL.h / SDL_mixer.h
# surfaces are identical there.
$dependencyRoot = Join-Path (Split-Path -Parent $repoRoot) 'dependencies'
$includeDirs += (Join-Path $repoRoot 'src')
$includeDirs += (Join-Path $dependencyRoot 'SDL2\include')
$includeDirs += (Join-Path $dependencyRoot 'SDL_mixer\include')

$includeArgs = @()
foreach ($dir in $includeDirs) {
    if (Test-Path -LiteralPath $dir) {
        $includeArgs += ('/I"{0}"' -f (Resolve-Path -LiteralPath $dir).Path)
    } else {
        Write-Warning "include directory not found, skipping: $dir"
    }
}
if ($includeArgs.Count -lt 2) {
    throw "could not resolve the SDL include directories"
}

Write-Host "compiling harness ($Configuration)" -ForegroundColor Cyan
$objectPath = Join-Path $outDir 'asset_acceptance_harness.obj'
$compileCmd = 'call "{0}" -arch=x64 -host_arch=x64 >nul && cl.exe /nologo /std:c++17 /EHsc /utf-8 {1} {2} /c /Fo"{3}" "{4}"' -f `
    $vsDevCmd, $runtime, ($includeArgs -join ' '), $objectPath, $sourcePath

$compileOutput = & cmd.exe /c $compileCmd 2>&1
$compileExit = $LASTEXITCODE
$compileOutput | ForEach-Object { Write-Host "  $_" }
if ($compileExit -ne 0) {
    throw "compilation failed with exit code $compileExit"
}

Write-Host "linking harness" -ForegroundColor Cyan
$libArgs = ($libs | ForEach-Object { '"{0}"' -f $_ }) -join ' '
$systemLibs = 'winmm.lib imm32.lib version.lib setupapi.lib user32.lib gdi32.lib ole32.lib oleaut32.lib advapi32.lib shell32.lib'
$linkCmd = 'call "{0}" -arch=x64 -host_arch=x64 >nul && link.exe /nologo /SUBSYSTEM:CONSOLE /OUT:"{1}" "{2}" {3} {4}' -f `
    $vsDevCmd, $exePath, $objectPath, $libArgs, $systemLibs

$linkOutput = & cmd.exe /c $linkCmd 2>&1
$linkExit = $LASTEXITCODE
$linkOutput | ForEach-Object { Write-Host "  $_" }
if ($linkExit -ne 0) {
    throw "link failed with exit code $linkExit"
}

Write-Host ""
Write-Host "running harness against $AssetsRoot" -ForegroundColor Cyan
$scratchRoot = Join-Path $outDir 'corrupt_probe_assets'
if (Test-Path -LiteralPath $scratchRoot) {
    Remove-Item -LiteralPath $scratchRoot -Recurse -Force
}

& $exePath $AssetsRoot $scratchRoot
$exit = $LASTEXITCODE

# A failed SDL_mixer load can hold the corrupt probe open until the harness
# process exits. Cleanup therefore belongs here, after the child has ended.
if (Test-Path -LiteralPath $scratchRoot) {
    try {
        Remove-Item -LiteralPath $scratchRoot -Recurse -Force
    }
    catch {
        Write-Host "scratch cleanup failed: $_" -ForegroundColor Red
        $exit = 1
    }
}

Write-Host ""
if ($exit -ne 0) {
    Write-Host "asset acceptance FAILED (exit $exit)" -ForegroundColor Red
} else {
    Write-Host "asset acceptance passed" -ForegroundColor Green
}
exit $exit
