# Controlla i requisiti per compilare e usare il port di Bloodborne con DLSS e menu multilingua.
# Uso:  powershell -ExecutionPolicy Bypass -File check_requisiti.ps1 [-Gioco D:\percorso\CUSA03173] [-DlssSdk D:\bbport\dlss-sdk]
# Non modifica nulla: stampa [OK], [MANCA] o [AVVISO] per ogni voce ed esce con 1 se manca qualcosa.
param(
    [string]$Gioco = '',
    [string]$DlssSdk = (Join-Path (Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent) 'dlss-sdk'),
    [string]$Msys = 'C:\msys64'
)

$script:mancanti = 0
function Ok($t)     { Write-Host "[OK]     $t" -ForegroundColor Green }
function Manca($t)  { Write-Host "[MANCA]  $t" -ForegroundColor Red; $script:mancanti++ }
function Avviso($t) { Write-Host "[AVVISO] $t" -ForegroundColor Yellow }

Write-Host '== Sistema'
$os = Get-CimInstance Win32_OperatingSystem
if ([Environment]::Is64BitOperatingSystem -and [version]$os.Version -ge [version]'10.0') { Ok "Windows a 64 bit ($($os.Caption))" }
else { Manca 'serve Windows 10 o 11 a 64 bit' }

Add-Type -Namespace Bb -Name Cpu -MemberDefinition '[DllImport("kernel32.dll")] public static extern bool IsProcessorFeaturePresent(uint feature);' -ErrorAction SilentlyContinue
if ([Bb.Cpu]::IsProcessorFeaturePresent(40)) { Ok "CPU con AVX2 ($((Get-CimInstance Win32_Processor).Name.Trim()))" }
else { Manca 'la CPU non supporta AVX2' }

$gpus = Get-CimInstance Win32_VideoController
foreach ($g in $gpus) { Write-Host "         GPU: $($g.Name) (driver $($g.DriverVersion))" }
$api = $null
$icd = Get-ChildItem "$env:SystemRoot\System32\DriverStore\FileRepository" -Recurse -Include 'nv-vk64.json','amd-vulkan64.json','igvk64.json' -ErrorAction SilentlyContinue
foreach ($f in $icd) {
    try { $v = [version]((Get-Content $f.FullName -Raw | ConvertFrom-Json).ICD.api_version); if (-not $api -or $v -gt $api) { $api = $v } } catch {}
}
if ($api -and $api -ge [version]'1.3') { Ok "driver Vulkan $api (serve 1.3 o superiore)" }
elseif ($api) { Manca "driver Vulkan ${api}: aggiorna il driver della scheda video (serve 1.3)" }
else { Avviso 'versione Vulkan non rilevata: aggiorna il driver della scheda video' }
if ($gpus | Where-Object { $_.Name -match 'RTX' }) { Ok 'GPU NVIDIA RTX presente (necessaria solo per il DLSS)' }
else { Avviso 'nessuna GPU NVIDIA RTX: il gioco funziona, ma il DLSS non sara'' disponibile' }

$libero = [math]::Round((Get-PSDrive ($PSScriptRoot.Substring(0,1))).Free / 1GB)
if ($libero -ge 15) { Ok "spazio libero sul disco: $libero GB" } else { Avviso "spazio libero sul disco: $libero GB (ne servono circa 15)" }

Write-Host '== Programmi'
$git = Get-Command git -ErrorAction SilentlyContinue
if ($git) { Ok (& git --version) } else { Manca 'Git (winget install --id Git.Git -e)' }

$py = (& where.exe python 2>$null) | Where-Object { $_ -notmatch 'WindowsApps' } | Select-Object -First 1
if ($py) {
    $pv = [version]((& $py -c "import sys;print('%d.%d' % sys.version_info[:2])"))
    $tk = (& $py -c "import tkinter;print('ok')" 2>$null)
    if ($pv -ge [version]'3.10' -and $tk -eq 'ok') { Ok "Python $pv con tkinter ($py)" }
    elseif ($pv -lt [version]'3.10') { Manca "Python $pv e' troppo vecchio (serve 3.10 o superiore)" }
    else { Manca 'Python senza tkinter: reinstallalo da python.org con le opzioni predefinite' }
} else { Manca 'Python 3.10+ (winget install --id Python.Python.3.13 -e)' }

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = if (Test-Path $vswhere) { & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath } else { $null }
if ($vs) { Ok "Visual Studio con strumenti C++ ($vs)" }
else { Manca 'Visual Studio 2022 (o Build Tools) con il carico di lavoro C++: serve per la DLL ponte del DLSS' }

Write-Host '== MSYS2 e librerie'
if (Test-Path "$Msys\usr\bin\bash.exe") {
    Ok "MSYS2 in $Msys"
    foreach ($t in 'gcc.exe','cmake.exe','ninja.exe','glslang.exe','pkgconf.exe') {
        if (Test-Path "$Msys\mingw64\bin\$t") { Ok "MinGW64: $t" } else { Manca "MinGW64: $t (esegui install_deps.sh)" }
    }
    $hdr = @{ 'SDL3' = 'SDL3\SDL.h'; 'Vulkan headers' = 'vulkan\vulkan.h'; 'VMA' = 'vma\vk_mem_alloc.h'; 'FFmpeg' = 'libavcodec\avcodec.h';
              'Boost' = 'boost\version.hpp'; 'fmt' = 'fmt\format.h'; 'Zydis' = 'Zydis\Zydis.h'; 'xxhash' = 'xxhash.h';
              'robin-map' = 'tsl\robin_map.h'; 'magic_enum' = 'magic_enum\magic_enum.hpp'; 'xbyak' = 'xbyak\xbyak.h'; 'miniz' = 'miniz.h' }
    foreach ($k in $hdr.Keys | Sort-Object) {
        if (Test-Path "$Msys\mingw64\include\$($hdr[$k])") { Ok "libreria: $k" } else { Manca "libreria: $k (esegui install_deps.sh)" }
    }
    if (Test-Path "$Msys\mingw64\lib\libminiz.a") { Ok 'libreria: miniz (libminiz.a)' } else { Manca 'libreria: libminiz.a (esegui install_deps.sh)' }
} else { Manca "MSYS2 in $Msys (winget install --id MSYS2.MSYS2 -e)" }

Write-Host '== SDK DLSS'
if ((Test-Path "$DlssSdk\include\nvsdk_ngx_vk.h") -and (Test-Path "$DlssSdk\lib\Windows_x86_64\x64\nvsdk_ngx_s.lib") -and (Test-Path "$DlssSdk\lib\Windows_x86_64\rel\nvngx_dlss.dll")) { Ok "SDK DLSS in $DlssSdk" }
else { Manca "SDK DLSS in $DlssSdk (git clone --depth 1 --branch v310.9.1 https://github.com/NVIDIA/DLSS.git `"$DlssSdk`")" }

Write-Host '== Gioco'
if ($Gioco) {
    $sfo = Join-Path $Gioco 'sce_sys\param.sfo'
    if ((Test-Path (Join-Path $Gioco 'eboot.bin')) -and (Test-Path (Join-Path $Gioco 'dvdroot_ps4')) -and (Test-Path $sfo)) {
        $txt = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($sfo))
        if ((Split-Path $Gioco -Leaf) -eq 'CUSA03173' -and $txt -match '01\.09') { Ok "dump del gioco CUSA03173 versione 1.09 ($Gioco)" }
        else { Manca 'il dump deve essere CUSA03173 aggiornato alla versione 1.09' }
    } else { Manca "in $Gioco mancano eboot.bin, dvdroot_ps4 o sce_sys\param.sfo" }
} else { Avviso 'dump del gioco non controllato: ripeti con -Gioco "D:\percorso\CUSA03173"' }

Write-Host ''
if ($script:mancanti) { Write-Host "Requisiti mancanti: $script:mancanti" -ForegroundColor Red; exit 1 }
Write-Host 'Tutti i requisiti obbligatori sono presenti.' -ForegroundColor Green
