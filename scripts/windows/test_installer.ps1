# Smoke test of the Windows installer: installs it silently into a temporary folder, checks what
# it installed and what it must never contain, starts nothing of the game, and uninstalls it.
# Usage:  powershell -ExecutionPolicy Bypass -File scripts\windows\test_installer.ps1 [-Setup dist\bbport-setup-x.exe]
# Without -Setup it takes the newest installer in dist\. Exit code 0 when everything is right.
param([string]$Setup = '')
$ErrorActionPreference = 'Continue'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $Setup) { $Setup = (Get-ChildItem (Join-Path $repo 'dist\bbport-setup-*.exe') | Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName }
if (-not $Setup -or -not (Test-Path $Setup)) { Write-Host '[ERROR] no installer found: run make_installer.ps1 first' -ForegroundColor Red; exit 1 }
$problems = New-Object System.Collections.Generic.List[string]
function Check($name, $ok) {
    if ($ok) { Write-Host "[PASS] $name" -ForegroundColor Green }
    else { Write-Host "[FAIL] $name" -ForegroundColor Red; $problems.Add($name) }
}

$target = Join-Path ([System.IO.Path]::GetTempPath()) ("bbport-install-test-" + [guid]::NewGuid().ToString('N').Substring(0, 8))
Write-Host "Installer: $Setup"
Write-Host "Target:    $target"
$install = Start-Process $Setup -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', "/DIR=$target", '/TASKS=', "/LOG=$target.log" -PassThru -Wait
Check 'the installer finishes without error' ($install.ExitCode -eq 0)
if ($install.ExitCode -ne 0 -and (Test-Path "$target.log")) { Get-Content "$target.log" -Tail 30 | Out-Host }

foreach ($file in 'Bloodborne.exe', 'run.bat', 'launch_gui.bat', 'launcher.py', 'LICENSE', 'SOURCE.txt', 'README_RELEASE.txt',
                  'out\bbport.exe', 'out\bb-gpu-capabilities.exe', 'scripts\prepare.py', 'scripts\patches.py',
                  'scripts\windows\make_icon.ps1', 'patches\Bloodborne.xml', 'fsr4_shaders\rcas.spv', 'unins000.exe') {
    Check "installed: $file" (Test-Path (Join-Path $target $file))
}
Check 'the port''s DLLs are installed' (@(Get-ChildItem "$target\out\*.dll" -ErrorAction SilentlyContinue).Count -ge 50)
Check 'the optimized FSR 4 shaders are installed' (@(Get-ChildItem "$target\fsr4_shaders\opt\*.spv" -ErrorAction SilentlyContinue).Count -gt 0)

# Nothing of the game, no icon made from it, no saves or settings of the person who built it.
$forbidden = 'eboot*', 'boot*.bin', '*.elf', '*.sfo', 'icon0.*', '*.ico', '*.pkg', 'bbport.ini', 'launcher_settings.json'
$found = @(Get-ChildItem $target -Recurse -File -Include $forbidden -ErrorAction SilentlyContinue)
$found | ForEach-Object { Write-Host "  $($_.FullName)" -ForegroundColor Red }
Check 'no game file, icon, save or personal setting in the package' ($found.Count -eq 0 -and -not (Test-Path "$target\user"))
$nvidia = @(Get-ChildItem $target -Recurse -File -Include 'nvngx*', 'bbdlss*' -ErrorAction SilentlyContinue)
if ($Setup -match '-dlss') { Write-Host "[NOTE] DLSS package: $($nvidia.Count) NVIDIA file(s) inside, keep it private" -ForegroundColor Yellow }
else { Check 'no NVIDIA DLSS file in the package' ($nvidia.Count -eq 0) }

if (Test-Path "$target\unins000.exe") {
    $remove = Start-Process "$target\unins000.exe" -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART' -PassThru -Wait
    Start-Sleep -Seconds 2
    Check 'the uninstaller finishes without error' ($remove.ExitCode -eq 0)
    Check 'the uninstaller removes the port' (-not (Test-Path "$target\out\bbport.exe") -and -not (Test-Path "$target\Bloodborne.exe"))
}
if (Test-Path $target) { Remove-Item -Recurse -Force $target -ErrorAction SilentlyContinue }
Remove-Item "$target.log" -ErrorAction SilentlyContinue

Write-Host ''
if ($problems.Count) { Write-Host "$($problems.Count) check(s) failed" -ForegroundColor Red; exit 1 }
Write-Host 'Installer test passed.' -ForegroundColor Green
