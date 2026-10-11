# Checks the requirements to build and use the Bloodborne port with DLSS and the multilingual menu.
# Usage:  powershell -ExecutionPolicy Bypass -File check_requirements.ps1 [-Game D:\path\CUSA03173] [-DlssSdk D:\path\dlss-sdk]
# Without parameters it checks modules\game\CUSA03173 and modules\dlss-sdk.
# It changes nothing: prints [OK], [MISSING] or [WARNING] for every item and exits with 1 if something is missing.
param(
    [string]$Game = '',
    [string]$DlssSdk = '',
    [string]$Msys = 'C:\msys64'
)
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $Game -and (Test-Path (Join-Path $repo 'modules\game\CUSA03173'))) { $Game = Join-Path $repo 'modules\game\CUSA03173' }
if (-not $DlssSdk) {
    $DlssSdk = Join-Path $repo 'modules\dlss-sdk'
    $sibling = Join-Path (Split-Path $repo -Parent) 'dlss-sdk'
    if (-not (Test-Path $DlssSdk) -and (Test-Path $sibling)) { $DlssSdk = $sibling }
}

$script:missing = 0
function Ok($t)      { Write-Host "[OK]      $t" -ForegroundColor Green }
function Missing($t) { Write-Host "[MISSING] $t" -ForegroundColor Red; $script:missing++ }
function Warn($t)    { Write-Host "[WARNING] $t" -ForegroundColor Yellow }

Write-Host '== System'
$os = Get-CimInstance Win32_OperatingSystem
if ([Environment]::Is64BitOperatingSystem -and [version]$os.Version -ge [version]'10.0') { Ok "64-bit Windows ($($os.Caption))" }
else { Missing '64-bit Windows 10 or 11 is required' }

Add-Type -Namespace Bb -Name Cpu -MemberDefinition '[DllImport("kernel32.dll")] public static extern bool IsProcessorFeaturePresent(uint feature);' -ErrorAction SilentlyContinue
if ([Bb.Cpu]::IsProcessorFeaturePresent(40)) { Ok "CPU with AVX2 ($((Get-CimInstance Win32_Processor).Name.Trim()))" }
else { Missing 'the CPU does not support AVX2' }

$gpus = Get-CimInstance Win32_VideoController
foreach ($g in $gpus) { Write-Host "          GPU: $($g.Name) (driver $($g.DriverVersion))" }
$api = $null
$icd = Get-ChildItem "$env:SystemRoot\System32\DriverStore\FileRepository" -Recurse -Include 'nv-vk64.json','amd-vulkan64.json','igvk64.json' -ErrorAction SilentlyContinue
foreach ($f in $icd) {
    try { $v = [version]((Get-Content $f.FullName -Raw | ConvertFrom-Json).ICD.api_version); if (-not $api -or $v -gt $api) { $api = $v } } catch {}
}
if ($api -and $api -ge [version]'1.3') { Ok "Vulkan driver $api (1.3 or newer is required)" }
elseif ($api) { Missing "Vulkan driver ${api}: update the graphics driver (1.3 is required)" }
else { Warn 'Vulkan version not detected: update the graphics driver' }
if ($gpus | Where-Object { $_.Name -match 'RTX' }) { Ok 'NVIDIA RTX GPU present (needed only for DLSS)' }
else { Warn 'no NVIDIA RTX GPU: the game works, but DLSS will not be available' }

$free = [math]::Floor((Get-PSDrive ($PSScriptRoot.Substring(0,1))).Free / 1GB)
if ($free -ge 15) { Ok "free disk space: $free GB" } else { Warn "free disk space: $free GB (about 15 are needed)" }

Write-Host '== Programs'
$git = Get-Command git -ErrorAction SilentlyContinue
if ($git) { Ok (& git --version) } else { Missing 'Git (winget install --id Git.Git -e)' }

$py = (& where.exe python 2>$null) | Where-Object { $_ -notmatch 'WindowsApps' } | Select-Object -First 1
if ($py) {
    $pv = [version]((& $py -c "import sys;print('%d.%d' % sys.version_info[:2])"))
    $tk = (& $py -c "import tkinter;print('ok')" 2>$null)
    if ($pv -ge [version]'3.10' -and $tk -eq 'ok') { Ok "Python $pv with tkinter ($py)" }
    elseif ($pv -lt [version]'3.10') { Missing "Python $pv is too old (3.10 or newer is required)" }
    else { Missing 'Python without tkinter: reinstall it from python.org with the default options' }
} else { Missing 'Python 3.10+ on PATH (winget install --id Python.Python.3.13 -e)' }

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = if (Test-Path $vswhere) { & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath } else { $null }
if ($vs) { Ok "Visual Studio with C++ tools ($vs)" }
else { Missing 'Visual Studio 2022 (or Build Tools) with the C++ workload: needed for the DLSS bridge DLL' }

Write-Host '== MSYS2 and libraries'
if (Test-Path "$Msys\usr\bin\bash.exe") {
    Ok "MSYS2 in $Msys"
    foreach ($t in 'gcc.exe','cmake.exe','ninja.exe','glslang.exe','pkgconf.exe') {
        if (Test-Path "$Msys\mingw64\bin\$t") { Ok "MinGW64: $t" } else { Missing "MinGW64: $t (run setup.bat)" }
    }
    $hdr = @{ 'SDL3' = 'SDL3\SDL.h'; 'Vulkan headers' = 'vulkan\vulkan.h'; 'VMA' = 'vma\vk_mem_alloc.h'; 'FFmpeg' = 'libavcodec\avcodec.h';
              'Boost' = 'boost\version.hpp'; 'fmt' = 'fmt\format.h'; 'Zydis' = 'Zydis\Zydis.h'; 'xxhash' = 'xxhash.h';
              'robin-map' = 'tsl\robin_map.h'; 'magic_enum' = 'magic_enum\magic_enum.hpp'; 'xbyak' = 'xbyak\xbyak.h'; 'miniz' = 'miniz.h' }
    foreach ($k in $hdr.Keys | Sort-Object) {
        if (Test-Path "$Msys\mingw64\include\$($hdr[$k])") { Ok "library: $k" } else { Missing "library: $k (run setup.bat)" }
    }
    if (Test-Path "$Msys\mingw64\lib\libminiz.a") { Ok 'library: miniz (libminiz.a)' } else { Missing 'library: libminiz.a (run setup.bat)' }
} else { Missing "MSYS2 in $Msys (winget install --id MSYS2.MSYS2 -e)" }

Write-Host '== DLSS SDK'
if ((Test-Path "$DlssSdk\include\nvsdk_ngx_vk.h") -and (Test-Path "$DlssSdk\lib\Windows_x86_64\x64\nvsdk_ngx_s.lib") -and (Test-Path "$DlssSdk\lib\Windows_x86_64\rel\nvngx_dlss.dll")) { Ok "DLSS SDK in $DlssSdk" }
else { Missing "DLSS SDK in $DlssSdk (git clone --depth 1 --branch v310.9.1 https://github.com/NVIDIA/DLSS.git `"$DlssSdk`")" }

Write-Host '== Game'
if ($Game) {
    $sfo = Join-Path $Game 'sce_sys\param.sfo'
    if ((Test-Path (Join-Path $Game 'eboot.bin')) -and (Test-Path (Join-Path $Game 'dvdroot_ps4')) -and (Test-Path $sfo)) {
        $txt = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($sfo))
        if ((Split-Path $Game -Leaf) -eq 'CUSA03173' -and $txt -match '01\.09') { Ok "game dump CUSA03173 version 1.09 ($Game)" }
        else { Missing 'the dump must be CUSA03173 updated to version 1.09' }
    } else { Missing "eboot.bin, dvdroot_ps4 or sce_sys\param.sfo is missing in $Game" }
} else { Warn 'game dump not found in modules\game\CUSA03173: copy it there, or run again with -Game "D:\path\CUSA03173"' }

Write-Host ''
if ($script:missing) { Write-Host "Missing requirements: $script:missing" -ForegroundColor Red; exit 1 }
Write-Host 'All mandatory requirements are present.' -ForegroundColor Green
