# Builds the Windows installer (dist\bbport-setup-<version>.exe) from the port already built in
# out\ (run setup.bat first). The installer never contains game files, the game's icon or files
# prepared from the game: only the port, its DLLs, the FSR 4 shaders and the launcher.
# Usage:  powershell -ExecutionPolicy Bypass -File scripts\windows\make_installer.ps1 [-WithDlss] [-Version x]
#   -WithDlss  also packs bbdlss.dll and nvngx_dlss.dll. NVIDIA's SDK licence does not allow using
#              the SDK in a way that subjects it to an open source licence, and the port is GPL:
#              keep such a package private, do not publish it.
#   -Version   version text in the file name and in Windows' program list
#              (default: date and commit, e.g. 2026.10.12-36ae776)
# Needs Inno Setup 6 (winget install --id JRSoftware.InnoSetup -e) and MSYS2 (for Bloodborne.exe).
param(
    [switch]$WithDlss,
    [string]$Version = '',
    [string]$Msys = 'C:\msys64'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repo
function Fail($t) { Write-Host "[ERROR] $t" -ForegroundColor Red; exit 1 }
function Step($t) { Write-Host "== $t" -ForegroundColor Cyan }

$iscc = "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe", "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "$env:ProgramFiles\Inno Setup 6\ISCC.exe" | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) { Fail 'Inno Setup 6 not found: winget install --id JRSoftware.InnoSetup -e' }
if (-not (Test-Path 'out\bbport.exe')) { Fail 'out\bbport.exe is missing: run setup.bat first' }
if (-not (Get-ChildItem 'fsr4_shaders\opt\*.spv' -ErrorAction SilentlyContinue)) { Fail 'fsr4_shaders is missing or incomplete: run setup.bat first' }
if ($WithDlss -and -not ((Test-Path 'out\bbdlss.dll') -and (Test-Path 'out\nvngx_dlss.dll'))) { Fail '-WithDlss: out\bbdlss.dll or out\nvngx_dlss.dll is missing' }

$commit = (& git rev-parse --short HEAD).Trim()
$dirty = [bool](& git status --porcelain --untracked-files=no --ignore-submodules)
if (-not $Version) {
    $Version = (Get-Date -Format 'yyyy.MM.dd') + "-$commit"
    if ($WithDlss) { $Version += '-dlss' }
}

Step "Staging files (version $Version)"
$stage = Join-Path $repo 'dist\stage'
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force "$stage\out", "$stage\scripts\windows", "$stage\patches" | Out-Null
Copy-Item 'run.bat', 'launch_gui.bat', 'launcher.py', 'LICENSE', 'README.md', 'CHANGES.md' $stage
Copy-Item 'packaging\windows\README_RELEASE.txt' $stage
Copy-Item 'scripts\*.py' "$stage\scripts"
Copy-Item 'scripts\windows\make_icon.ps1' "$stage\scripts\windows"
Copy-Item 'patches\Bloodborne.xml' "$stage\patches"
Copy-Item 'fsr4_shaders' $stage -Recurse
# out\: only the port's own programs and DLLs. Everything else in out\ is made at run time, part
# of it from the user's game copy, and must never be packed.
Copy-Item 'out\bbport.exe', 'out\bb-gpu-capabilities.exe' "$stage\out"
$dlss = 'bbdlss.dll', 'nvngx_dlss.dll'
Get-ChildItem 'out\*.dll' | Where-Object { $WithDlss -or $dlss -notcontains $_.Name } | Copy-Item -Destination "$stage\out"

$origin = (& git config --get remote.mio.url)
if (-not $origin) { $origin = (& git config --get remote.origin.url) }
$source = @(
    'Bloodborne PC Port (bbport) - source code of this package',
    '',
    "Repository: $origin",
    "Commit:     $(& git rev-parse HEAD)" + $(if ($dirty) { ' (with local changes not yet committed)' } else { '' }),
    "Built:      $(Get-Date -Format 'yyyy-MM-dd')",
    '',
    'The port is licensed under the GNU General Public License, version 2 or later (see LICENSE).',
    'The complete source code of this build is the commit above, with its git submodules.'
)
Set-Content -Path "$stage\SOURCE.txt" -Value $source -Encoding ASCII

Step 'Building Bloodborne.exe (without an icon)'
$env:CHERE_INVOKING = '1'; $env:MSYSTEM = 'MINGW64'; $env:MSYS2_PATH_TYPE = ''
& "$Msys\usr\bin\bash.exe" -lc 'gcc -O2 -s -mwindows scripts/windows/bloodborne_launcher.c -o dist/stage/Bloodborne.exe' | Out-Host
if ($LASTEXITCODE -ne 0 -or -not (Test-Path "$stage\Bloodborne.exe")) { Fail 'building Bloodborne.exe failed' }

Step 'Checking that nothing of the game is in the package'
$forbidden = 'eboot*', 'boot*.bin', 'entry.bin', 'content.bin', 'patches.bin', '*.elf', '*.sfo', 'icon0.*', '*.ico',
             '*.pkg', 'analysis.json', 'link.json', 'libc-link.json', 'launcher_settings.json', 'bbport.ini', '*.log'
if (-not $WithDlss) { $forbidden += 'nvngx*', 'bbdlss*', 'nvsdk*' }
$bad = Get-ChildItem $stage -Recurse -File -Include $forbidden
if ($bad) { $bad | ForEach-Object { Write-Host "  $($_.FullName)" -ForegroundColor Red }; Fail 'files that must not be distributed are in the stage folder' }
if (Test-Path "$stage\user") { Fail 'the stage folder contains save files' }
$count = (Get-ChildItem $stage -Recurse -File).Count
$size = [math]::Round((Get-ChildItem $stage -Recurse -File | Measure-Object Length -Sum).Sum / 1MB)
Write-Host "$count files, $size MB"

Step 'Compiling the installer'
$has = if ($WithDlss) { '1' } else { '0' }
& $iscc /Qp "/DAppVersion=$Version" "/DStage=$stage" "/DHasDlss=$has" 'packaging\windows\bbport.iss' | Out-Host
if ($LASTEXITCODE -ne 0) { Fail 'Inno Setup reported errors' }
$setup = Join-Path $repo "dist\bbport-setup-$Version.exe"
if (-not (Test-Path $setup)) { Fail "the installer was not produced: $setup" }
Write-Host ''
Write-Host "Installer: $setup ($([math]::Round((Get-Item $setup).Length / 1MB)) MB)" -ForegroundColor Green
if ($dirty) { Write-Host '[WARNING] built from a working tree with uncommitted changes: commit and push before publishing, so SOURCE.txt names a public commit' -ForegroundColor Yellow }
if ($WithDlss) { Write-Host '[WARNING] this package contains NVIDIA DLSS files: keep it private' -ForegroundColor Yellow }
