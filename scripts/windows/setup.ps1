# Setup completo del port di Bloodborne su Windows: controlla ogni requisito, installa solo quello
# che manca, scarica i componenti e compila. Si puo' rilanciare quante volte si vuole: i passi
# gia' fatti vengono saltati.
# Uso (dalla cartella del repository):  setup.bat [-SenzaDlss] [-SoloControllo]
#   -SenzaDlss      salta Visual Studio Build Tools e l'SDK di NVIDIA (circa 9 GB in meno)
#   -SoloControllo  dice cosa manca senza installare ne' compilare nulla
param(
    [switch]$SenzaDlss,
    [switch]$SoloControllo,
    [string]$Msys = 'C:\msys64'
)
# 'Continue': con 'Stop' PowerShell 5.1 tratta come errore fatale qualsiasi riga scritta su stderr da
# un programma esterno (gli avvisi di CMake, i messaggi di git). Gli esiti sono controllati a mano.
$ErrorActionPreference = 'Continue'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repo

function Titolo($t)  { Write-Host ''; Write-Host "== $t" -ForegroundColor Cyan }
function Ok($t)      { Write-Host "[OK]       $t" -ForegroundColor Green }
function Faccio($t)  { Write-Host "[INSTALLO] $t" -ForegroundColor Yellow }
function Avviso($t)  { Write-Host "[AVVISO]   $t" -ForegroundColor Yellow }
function Errore($t)  { Write-Host "[ERRORE]   $t" -ForegroundColor Red; exit 1 }
function AggiornaPath {
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User')
}
# Installa un pacchetto con winget; in modalita' -SoloControllo lo segnala soltanto.
function Installa($nome, $id, $extra) {
    if ($SoloControllo) { Write-Host "[MANCA]    $nome" -ForegroundColor Red; $script:mancanti++; return $false }
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Errore "manca $nome e non c'e' winget per installarlo: installa 'Programma di installazione app' dal Microsoft Store"
    }
    Faccio $nome
    $argomenti = @('install', '--id', $id, '-e') + $extra
    & winget @argomenti | Out-Host
    if ($LASTEXITCODE -ne 0) { Errore "installazione di $nome fallita (winget, codice $LASTEXITCODE)" }
    AggiornaPath
    return $true
}
function Bash($comando) {
    $env:CHERE_INVOKING = '1'; $env:MSYSTEM = 'MINGW64'; $env:MSYS2_PATH_TYPE = ''
    # Out-Host: l'output va a schermo e non nel valore di ritorno della funzione.
    & "$Msys\usr\bin\bash.exe" -lc $comando | Out-Host
    return $LASTEXITCODE
}
function TrovaPython {
    foreach ($p in (& where.exe python 2>$null)) {
        if ($p -match 'WindowsApps') { continue }
        $v = & $p -c "import sys,tkinter;print('%d.%d' % sys.version_info[:2])" 2>$null
        if ($v -and [version]$v -ge [version]'3.10') { return $p }
    }
    return $null
}
function LibrerieMancanti {
    $file = 'bin\gcc.exe', 'bin\cmake.exe', 'bin\ninja.exe', 'bin\glslang.exe', 'bin\pkgconf.exe',
            'include\SDL3\SDL.h', 'include\vulkan\vulkan.h', 'include\vma\vk_mem_alloc.h',
            'include\libavcodec\avcodec.h', 'include\boost\version.hpp', 'include\fmt\format.h',
            'include\Zydis\Zydis.h', 'include\xxhash.h', 'include\tsl\robin_map.h',
            'include\magic_enum\magic_enum.hpp', 'include\xbyak\xbyak.h', 'include\miniz.h', 'lib\libminiz.a'
    return @($file | Where-Object { -not (Test-Path "$Msys\mingw64\$_") })
}
$script:mancanti = 0

Titolo 'Sistema'
$os = Get-CimInstance Win32_OperatingSystem
if (-not ([Environment]::Is64BitOperatingSystem -and [version]$os.Version -ge [version]'10.0')) { Errore 'serve Windows 10 o 11 a 64 bit' }
Ok "Windows a 64 bit ($($os.Caption))"
Add-Type -Namespace Bb -Name Cpu -MemberDefinition '[DllImport("kernel32.dll")] public static extern bool IsProcessorFeaturePresent(uint feature);'
if (-not [Bb.Cpu]::IsProcessorFeaturePresent(40)) { Errore 'la CPU non supporta AVX2: il gioco non puo'' funzionare' }
Ok 'CPU con AVX2'
$rtx = [bool](Get-CimInstance Win32_VideoController | Where-Object { $_.Name -match 'RTX' })
$dlss = $rtx -and -not $SenzaDlss
if ($dlss) { Ok 'GPU NVIDIA RTX: il DLSS verra'' compilato' }
elseif ($SenzaDlss) { Avviso 'DLSS escluso su richiesta (-SenzaDlss): resta FSR' }
else { Avviso 'nessuna GPU NVIDIA RTX: salto la parte DLSS, resta FSR' }

Titolo 'Programmi'
if (Get-Command git -ErrorAction SilentlyContinue) { Ok (& git --version) }
else { [void](Installa 'Git' 'Git.Git' @()) }

$py = TrovaPython
if ($py) { Ok "Python con tkinter ($py)" }
elseif (Installa 'Python 3.13' 'Python.Python.3.13' @()) {
    $py = TrovaPython
    if (-not $py) { Errore 'Python installato ma non ancora visibile: chiudi questa finestra, riaprila e rilancia setup.bat' }
}

$msysNuovo = $false
if (Test-Path "$Msys\usr\bin\bash.exe") { Ok "MSYS2 in $Msys" }
else { $msysNuovo = Installa 'MSYS2' 'MSYS2.MSYS2' @() }

if ($dlss) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = if (Test-Path $vswhere) { & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath } else { $null }
    if ($vs) { Ok "Visual Studio con strumenti C++ ($vs)" }
    else {
        [void](Installa 'Visual Studio Build Tools con C++ (circa 7 GB)' 'Microsoft.VisualStudio.2022.BuildTools' `
            @('--override', '--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'))
    }
}

Titolo 'Librerie MSYS2'
if (Test-Path "$Msys\usr\bin\bash.exe") {
    if ($msysNuovo) {
        Faccio 'aggiornamento di MSYS2 (due passate)'
        [void](Bash 'pacman -Syu --noconfirm'); [void](Bash 'pacman -Syu --noconfirm')
    }
    $manca = LibrerieMancanti
    if (-not $manca) { Ok 'compilatore e librerie gia'' presenti' }
    elseif ($SoloControllo) { Write-Host "[MANCA]    librerie MSYS2: $($manca -join ', ')" -ForegroundColor Red; $script:mancanti++ }
    else {
        Faccio "librerie MSYS2 (mancano: $($manca -join ', '))"
        # Il server di MSYS2 a volte risponde 429 (occupato): un secondo tentativo di solito basta.
        if ((Bash 'bash scripts/windows/install_deps.sh') -ne 0) {
            Avviso 'primo tentativo fallito, riprovo tra 20 secondi'
            Start-Sleep -Seconds 20
            if ((Bash 'bash scripts/windows/install_deps.sh') -ne 0) { Errore 'installazione delle librerie fallita: rilancia setup.bat tra qualche minuto' }
        }
        if (LibrerieMancanti) { Errore "librerie ancora mancanti: $((LibrerieMancanti) -join ', ')" }
    }
}

Titolo 'Componenti del codice'
$haGit = [bool](Get-Command git -ErrorAction SilentlyContinue)
if (-not (Test-Path (Join-Path $repo '.git'))) {
    Errore 'questa cartella non e'' un clone git (scaricata come ZIP?): scarica il progetto con "git clone", vedi GUIDA.md'
}
$sotto = 'gpu\third_party\fsr-vulkan'
$sottoPronti = (Test-Path "$sotto\.git") -and (Test-Path 'third_party\LibAtrac9\C\src')
if ($sottoPronti) { Ok 'sottomoduli (FSR, ImGui, LibAtrac9) gia'' scaricati' }
elseif ($SoloControllo -or -not $haGit) { Write-Host '[MANCA]    sottomoduli (FSR, ImGui, LibAtrac9)' -ForegroundColor Red; $script:mancanti++ }
else {
    Faccio 'sottomoduli (FSR, ImGui, LibAtrac9)'
    & git submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) { Errore 'download dei sottomoduli fallito' }
}
if ($haGit -and (Test-Path "$sotto\.git")) {
    $patch = '..\..\..\patches\fsr_vulkan_mingw.patch'
    & git -C $sotto apply --reverse --check $patch 2>$null
    if ($LASTEXITCODE -eq 0) { Ok 'correzione di FSR per MinGW gia'' applicata' }
    elseif ($SoloControllo) { Write-Host '[MANCA]    correzione di FSR per MinGW' -ForegroundColor Red; $script:mancanti++ }
    else {
        Faccio 'correzione di FSR per MinGW'
        & git -C $sotto apply $patch
        if ($LASTEXITCODE -ne 0) { Errore 'la correzione di FSR non si applica' }
    }
}
if ($dlss) {
    $sdk = Join-Path $repo 'modules\dlss-sdk'
    $accanto = Join-Path (Split-Path $repo -Parent) 'dlss-sdk'
    if (Test-Path "$sdk\include\nvsdk_ngx_vk.h") { Ok 'SDK DLSS in modules\dlss-sdk' }
    elseif (Test-Path "$accanto\include\nvsdk_ngx_vk.h") { Ok "SDK DLSS in $accanto" }
    elseif ($SoloControllo -or -not $haGit) { Write-Host '[MANCA]    SDK DLSS in modules\dlss-sdk' -ForegroundColor Red; $script:mancanti++ }
    else {
        Faccio 'SDK DLSS di NVIDIA in modules\dlss-sdk (circa 1,6 GB)'
        & git clone --depth 1 --branch v310.9.1 https://github.com/NVIDIA/DLSS.git $sdk
        if ($LASTEXITCODE -ne 0) { Errore 'download dell''SDK DLSS fallito' }
    }
}

Titolo 'Gioco'
$gioco = Join-Path $repo 'modules\game\CUSA03173'
if (Test-Path "$gioco\eboot.bin") { Ok 'gioco in modules\game\CUSA03173' }
else { Avviso 'gioco non trovato in modules\game\CUSA03173: copialo li'', oppure scegli eboot.bin dal launcher' }

if ($SoloControllo) {
    Write-Host ''
    if ($script:mancanti) { Write-Host "Voci mancanti: $script:mancanti. Lancia setup.bat senza -SoloControllo per installarle." -ForegroundColor Red; exit 1 }
    Write-Host 'Tutti i requisiti sono presenti.' -ForegroundColor Green
    exit 0
}

Titolo 'Compilazione'
if (Get-Process bbport -ErrorAction SilentlyContinue) { Errore 'il gioco e'' aperto: chiudilo e rilancia setup.bat' }
Write-Host 'La prima volta servono 20-30 minuti.'
if ((Bash 'bash scripts/windows/build_bbport.sh') -ne 0) { Errore 'compilazione del gioco fallita' }
if ($dlss) {
    & cmd /c 'gpu\dlss\build_bbdlss.bat'
    if ($LASTEXITCODE -ne 0) { Errore 'compilazione della parte DLSS fallita' }
}
$attesi = @('out\bbport.exe')
if ($dlss) { $attesi += 'out\bbdlss.dll', 'out\nvngx_dlss.dll' }
foreach ($f in $attesi) { if (-not (Test-Path $f)) { Errore "manca $f dopo la compilazione" } }

Write-Host ''
Write-Host 'Setup completato.' -ForegroundColor Green
Write-Host 'Per giocare: doppio clic su launch_gui.bat (launcher), oppure run.bat.'
