# Smoke test of the Windows installer: installs it silently into a temporary folder, checks what
# it installed and what it must never contain, starts nothing of the game, and uninstalls it.
# Usage:  powershell -ExecutionPolicy Bypass -File scripts\windows\test_installer.ps1 [-Setup dist\bbport-setup-x.exe] [-BuildDlss]
# Without -Setup it takes the newest installer in dist\. Exit code 0 when everything is right.
#   -BuildDlss  also builds DLSS in the installed copy, as build_dlss.bat does on a user's PC:
#               downloads NVIDIA's SDK files (63 MB) and needs the Microsoft C++ compiler.
param([string]$Setup = '', [switch]$BuildDlss)
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
# Made-up saves of an earlier installation, to test the import page.
$saves = "$target-saves"
$slot = Join-Path $saves 'user\savedata\1\CUSA00207\SPRJ0005'
New-Item -ItemType Directory -Force $slot | Out-Null
Set-Content -Path (Join-Path $slot 'userdata0000') -Value 'test save' -Encoding ASCII
$install = Start-Process $Setup -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', "/DIR=$target", '/TASKS=', "/SAVES=$saves", "/LOG=$target.log" -PassThru -Wait
Check 'the installer finishes without error' ($install.ExitCode -eq 0)
if ($install.ExitCode -ne 0 -and (Test-Path "$target.log")) { Get-Content "$target.log" -Tail 30 | Out-Host }

foreach ($file in 'Bloodborne.exe', 'run.bat', 'launch_gui.bat', 'launcher.py', 'LICENSE', 'SOURCE.txt', 'README_RELEASE.txt',
                  'out\bbport.exe', 'out\bb-gpu-capabilities.exe', 'scripts\prepare.py', 'scripts\patches.py',
                  'scripts\windows\make_icon.ps1', 'patches\Bloodborne.xml', 'fsr4_shaders\rcas.spv', 'unins000.exe',
                  'build_dlss.bat', 'scripts\windows\build_dlss.ps1', 'gpu\dlss\bbdlss.cpp', 'gpu\dlss\build_bbdlss.bat',
                  'gpu\dlss\LICENSE.txt', 'gpu\dlss\vk_include\vulkan\vulkan.h', 'gpu\dlss\vk_include\vk_video\vulkan_video_codecs_common.h') {
    Check "installed: $file" (Test-Path (Join-Path $target $file))
}
Check 'the saves of the earlier installation are imported' (Test-Path (Join-Path $target 'user\savedata\1\CUSA00207\SPRJ0005\userdata0000'))
Check 'the port''s DLLs are installed' (@(Get-ChildItem "$target\out\*.dll" -ErrorAction SilentlyContinue).Count -ge 50)
Check 'the optimized FSR 4 shaders are installed' (@(Get-ChildItem "$target\fsr4_shaders\opt\*.spv" -ErrorAction SilentlyContinue).Count -gt 0)

# Nothing of the game, no icon made from it, no saves or settings of the person who built it.
$forbidden = 'eboot*', 'boot*.bin', '*.elf', '*.sfo', 'icon0.*', '*.ico', '*.pkg', 'bbport.ini', 'launcher_settings.json'
$found = @(Get-ChildItem $target -Recurse -File -Include $forbidden -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notlike "$target\user\*" })
$found | ForEach-Object { Write-Host "  $($_.FullName)" -ForegroundColor Red }
# user\ holds only the saves imported above: the package itself ships none.
$shipped = @(Get-ChildItem "$target\user" -Recurse -File -ErrorAction SilentlyContinue | Where-Object { $_.Name -ne 'userdata0000' })
Check 'no game file, icon, save or personal setting in the package' ($found.Count -eq 0 -and $shipped.Count -eq 0)
$nvidia = @(Get-ChildItem $target -Recurse -File -Include 'nvngx*', 'nvsdk*', 'bbdlss.dll', '*.lib' -ErrorAction SilentlyContinue)
if ($Setup -match '-dlss') { Write-Host "[NOTE] DLSS package: $($nvidia.Count) NVIDIA file(s) inside, keep it private" -ForegroundColor Yellow }
else { Check 'no NVIDIA DLSS file in the package' ($nvidia.Count -eq 0) }

if ($BuildDlss) {
    # What a user with an NVIDIA RTX card gets from build_dlss.bat; -AnyGpu because test machines have none.
    & powershell -NoProfile -ExecutionPolicy Bypass -File "$target\scripts\windows\build_dlss.ps1" -Yes -AnyGpu | Out-Host
    Check 'DLSS builds in the installed copy' ($LASTEXITCODE -eq 0 -and (Test-Path "$target\out\bbdlss.dll") -and (Test-Path "$target\out\nvngx_dlss.dll"))
}

if (Test-Path "$target\unins000.exe") {
    $remove = Start-Process "$target\unins000.exe" -ArgumentList '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART' -PassThru -Wait
    Start-Sleep -Seconds 2
    Check 'the uninstaller finishes without error' ($remove.ExitCode -eq 0)
    Check 'the uninstaller keeps the saves' (Test-Path (Join-Path $target 'user\savedata\1\CUSA00207\SPRJ0005\userdata0000'))
    Check 'the uninstaller removes the port' (-not (Test-Path "$target\out\bbport.exe") -and -not (Test-Path "$target\Bloodborne.exe"))
    Check 'the uninstaller removes the DLSS files' (-not (Test-Path "$target\modules") -and -not (Test-Path "$target\out\bbdlss.dll"))
}
if (Test-Path $target) { Remove-Item -Recurse -Force $target -ErrorAction SilentlyContinue }
Remove-Item "$target.log" -ErrorAction SilentlyContinue
if (Test-Path $saves) { Remove-Item -Recurse -Force $saves -ErrorAction SilentlyContinue }

Write-Host ''
if ($problems.Count) { Write-Host "$($problems.Count) check(s) failed" -ForegroundColor Red; exit 1 }
Write-Host 'Installer test passed.' -ForegroundColor Green
