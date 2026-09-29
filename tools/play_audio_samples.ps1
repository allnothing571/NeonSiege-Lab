<#
.SYNOPSIS
    Play the Neon Siege sound effects through the default Windows audio device.

.DESCRIPTION
    Automated tests can prove format, loadability and level, but they cannot
    judge whether a sound is pleasant or whether the five combat sounds are
    distinguishable by ear. This script plays the files so that step is quick.

    Groups:
      all       every catalog file in catalog order (default)
      ui        menu select / confirm / back, for the "must be distinguishable"
                check from the listening checklist
      combat    player shot, enemy shot, hit, player damaged, enemy died,
                reload start, reload complete, back to back
      system    wave start, upgrade selected, victory, game over
      repeat    the dense-trigger check: player shot x10, hit x10, then
                enemy died x3 overlapping

.PARAMETER Group
    all | ui | combat | system | repeat

.PARAMETER Pause
    Seconds to wait after each file. Default 0.35.

.PARAMETER Root
    Asset directory. Defaults to the repository assets/ directory.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\play_audio_samples.ps1 -Group all
#>

[CmdletBinding()]
param(
    [ValidateSet('all', 'ui', 'combat', 'system', 'repeat')]
    [string]$Group = 'all',

    [double]$Pause = 0.35,

    [string]$Root
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $Root) {
    $Root = Join-Path $repoRoot 'assets\audio'
}
if (-not (Test-Path -LiteralPath $Root)) {
    throw "asset directory not found: $Root"
}
$Root = (Resolve-Path -LiteralPath $Root).Path

if (-not ('Mci' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class Mci {
  [DllImport("winmm.dll", CharSet=CharSet.Unicode)]
  static extern int mciSendString(string cmd, StringBuilder ret, int len, IntPtr hwnd);
  public static int Send(string cmd) { return mciSendString(cmd, null, 0, IntPtr.Zero); }
  public static string Query(string cmd) {
    var sb = new StringBuilder(260);
    mciSendString(cmd, sb, sb.Capacity, IntPtr.Zero);
    return sb.ToString();
  }
}
'@
}

$groups = [ordered]@{
    ui = @(
        'ui\select.wav',
        'ui\confirm.wav',
        'ui\back.wav'
    )
    combat = @(
        'combat\player_shot.wav',
        'combat\enemy_shot.wav',
        'combat\projectile_hit.wav',
		'combat\player_damaged.wav',
		'combat\enemy_died.wav',
		'combat\reload_start.wav',
		'combat\reload_complete.wav'
    )
    system = @(
        'system\wave_start.wav',
        'system\upgrade_selected.wav',
        'system\victory.wav',
        'system\game_over.wav'
    )
}

$playlist = switch ($Group) {
    'all' {
        @()
        foreach ($key in $groups.Keys) { $playlist += $groups[$key] }
        $playlist
    }
    'repeat' {
        @()
        for ($i = 0; $i -lt 10; $i++) { $playlist += 'combat\player_shot.wav' }
        for ($i = 0; $i -lt 10; $i++) { $playlist += 'combat\projectile_hit.wav' }
        for ($i = 0; $i -lt 3; $i++) { $playlist += 'combat\enemy_died.wav' }
        $playlist
    }
    default { $groups[$Group] }
}

$index = 0
foreach ($relative in $playlist) {
    $path = Join-Path $Root $relative
    if (-not (Test-Path -LiteralPath $path)) {
        Write-Warning "missing: $relative"
        continue
    }

    $alias = "neon_play_$index"
    $opened = [Mci]::Send(('open "{0}" type waveaudio alias {1}' -f $path, $alias))
    if ($opened -ne 0) {
        Write-Warning "cannot open $relative (mci $opened)"
        $index++
        continue
    }

    [void][Mci]::Send("play $alias wait")
    [void][Mci]::Send("close $alias")

    $label = if ($Group -eq 'repeat') {
        '{0}  [{1}/{2}]' -f $relative, ($index + 1), $playlist.Count
    } else {
        $relative
    }
    Write-Host $label
    if ($Pause -gt 0) {
        Start-Sleep -Milliseconds ([int]($Pause * 1000))
    }
    $index++
}

Write-Host ""
Write-Host "played $index file(s)" -ForegroundColor Cyan
