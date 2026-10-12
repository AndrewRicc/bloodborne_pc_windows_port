# Full setup of the Bloodborne port on Windows: checks every requirement, installs only what is
# missing, downloads the components and builds. It can be run as many times as needed: the steps
# already done are skipped.
# Usage (from the repository folder):  setup.bat [-NoDlss] [-CheckOnly] [-NoShortcut]
#   -NoDlss     skips Visual Studio Build Tools and the NVIDIA SDK (about 9 GB less)
#   -CheckOnly  reports what is missing without installing or building anything
#   -NoShortcut does not build Bloodborne.exe and does not create the desktop shortcut
param(
    [switch]$NoDlss,
    [switch]$CheckOnly,
    [switch]$NoShortcut,
    [string]$Msys = 'C:\msys64'
)
# 'Continue': with 'Stop', PowerShell 5.1 treats any line an external program writes to stderr
# (CMake warnings, git messages) as a fatal error. Results are checked by hand.
$ErrorActionPreference = 'Continue'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repo

function Section($t) { Write-Host ''; Write-Host "== $t" -ForegroundColor Cyan }
function Ok($t)      { Write-Host "[OK]      $t" -ForegroundColor Green }
function Doing($t)   { Write-Host "[INSTALL] $t" -ForegroundColor Yellow }
function Warn($t)    { Write-Host "[WARNING] $t" -ForegroundColor Yellow }
function Missing($t) { Write-Host "[MISSING] $t" -ForegroundColor Red; $script:missing++ }
function Fail($t)    { Write-Host "[ERROR]   $t" -ForegroundColor Red; exit 1 }
function RefreshPath {
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User')
}
# Installs a package with winget; with -CheckOnly it only reports it.
function Install($name, $id, $extra) {
    if ($CheckOnly) { Missing $name; return $false }
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Fail "$name is missing and winget is not available to install it: install 'App Installer' from the Microsoft Store"
    }
    Doing $name
    $arguments = @('install', '--id', $id, '-e') + $extra
    & winget @arguments | Out-Host
    # -1978335189: winget finds the package already installed with no upgrade. Not an error.
    if ($LASTEXITCODE -eq -1978335189) { Warn "$name is already installed according to winget" }
    elseif ($LASTEXITCODE -ne 0) { Fail "installing $name failed (winget, code $LASTEXITCODE)" }
    RefreshPath
    return $true
}
function Bash($command) {
    $env:CHERE_INVOKING = '1'; $env:MSYSTEM = 'MINGW64'; $env:MSYS2_PATH_TYPE = ''
    # Out-Host: the output goes to the screen and not into the function's return value.
    & "$Msys\usr\bin\bash.exe" -lc $command | Out-Host
    return $LASTEXITCODE
}
function FindPython {
    # Python can be installed without being on PATH: besides PATH, look at the "py" launcher and
    # the usual installation folders.
    $candidates = @(& where.exe python 2>$null)
    if (Get-Command py -ErrorAction SilentlyContinue) { $candidates += @(& py -3 -c "import sys;print(sys.executable)" 2>$null) }
    foreach ($pattern in "$env:ProgramFiles\Python3*\python.exe", "$env:LocalAppData\Programs\Python\Python3*\python.exe",
                         "$env:LocalAppData\Python\*\python.exe") {
        $candidates += @(Get-ChildItem $pattern -ErrorAction SilentlyContinue | Sort-Object FullName -Descending | ForEach-Object FullName)
    }
    foreach ($p in $candidates) {
        if (-not $p -or $p -match 'WindowsApps' -or -not (Test-Path $p)) { continue }
        $v = & $p -c "import sys,tkinter;print('%d.%d' % sys.version_info[:2])" 2>$null
        if ($v -and [version]$v -ge [version]'3.10') { return $p }
    }
    return $null
}
function MissingLibraries {
    $files = 'bin\gcc.exe', 'bin\cmake.exe', 'bin\ninja.exe', 'bin\glslang.exe', 'bin\pkgconf.exe',
             'include\SDL3\SDL.h', 'include\vulkan\vulkan.h', 'include\vma\vk_mem_alloc.h',
             'include\libavcodec\avcodec.h', 'include\boost\version.hpp', 'include\fmt\format.h',
             'include\Zydis\Zydis.h', 'include\xxhash.h', 'include\tsl\robin_map.h',
             'include\magic_enum\magic_enum.hpp', 'include\xbyak\xbyak.h', 'include\miniz.h', 'lib\libminiz.a'
    return @($files | Where-Object { -not (Test-Path "$Msys\mingw64\$_") })
}
$script:missing = 0

Section 'System'
$os = Get-CimInstance Win32_OperatingSystem
if (-not ([Environment]::Is64BitOperatingSystem -and [version]$os.Version -ge [version]'10.0')) { Fail '64-bit Windows 10 or 11 is required' }
Ok "64-bit Windows ($($os.Caption))"
Add-Type -Namespace Bb -Name Cpu -MemberDefinition '[DllImport("kernel32.dll")] public static extern bool IsProcessorFeaturePresent(uint feature);'
if (-not [Bb.Cpu]::IsProcessorFeaturePresent(40)) { Fail 'the CPU does not support AVX2: the game cannot run' }
Ok 'CPU with AVX2'
$rtx = [bool](Get-CimInstance Win32_VideoController | Where-Object { $_.Name -match 'RTX' })
$dlss = $rtx -and -not $NoDlss
if ($dlss) { Ok 'NVIDIA RTX GPU: DLSS will be built' }
elseif ($NoDlss) { Warn 'DLSS excluded on request (-NoDlss): FSR remains' }
else { Warn 'no NVIDIA RTX GPU: skipping the DLSS part, FSR remains' }

Section 'Programs'
if (Get-Command git -ErrorAction SilentlyContinue) { Ok (& git --version) }
else { [void](Install 'Git' 'Git.Git' @()) }

$py = FindPython
if ($py) { Ok "Python with tkinter ($py)" }
elseif (Install 'Python 3.13' 'Python.Python.3.13' @()) {
    $py = FindPython
    if (-not $py) { Fail 'Python is installed but I cannot find a python.exe 3.10+ with tkinter: close this window, reopen it and run setup.bat again; if it happens again, reinstall Python from python.org with the default options' }
    Ok "Python with tkinter ($py)"
}

$msysNew = $false
if (Test-Path "$Msys\usr\bin\bash.exe") { Ok "MSYS2 in $Msys" }
else { $msysNew = Install 'MSYS2' 'MSYS2.MSYS2' @() }

if ($dlss) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = if (Test-Path $vswhere) { & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath } else { $null }
    if ($vs) { Ok "Visual Studio with C++ tools ($vs)" }
    else {
        [void](Install 'Visual Studio Build Tools with C++ (about 7 GB)' 'Microsoft.VisualStudio.2022.BuildTools' `
            @('--override', '--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'))
    }
}

Section 'MSYS2 libraries'
if (Test-Path "$Msys\usr\bin\bash.exe") {
    if ($msysNew) {
        Doing 'MSYS2 update (two passes)'
        [void](Bash 'pacman -Syu --noconfirm'); [void](Bash 'pacman -Syu --noconfirm')
    }
    $lacking = MissingLibraries
    if (-not $lacking) { Ok 'compiler and libraries already present' }
    elseif ($CheckOnly) { Missing "MSYS2 libraries: $($lacking -join ', ')" }
    else {
        Doing "MSYS2 libraries (missing: $($lacking -join ', '))"
        # The MSYS2 server sometimes answers 429 (busy): a second attempt is usually enough.
        if ((Bash 'bash scripts/windows/install_deps.sh') -ne 0) {
            Warn 'first attempt failed, retrying in 20 seconds'
            Start-Sleep -Seconds 20
            if ((Bash 'bash scripts/windows/install_deps.sh') -ne 0) { Fail 'installing the libraries failed: run setup.bat again in a few minutes' }
        }
        if (MissingLibraries) { Fail "libraries still missing: $((MissingLibraries) -join ', ')" }
    }
}

Section 'Source components'
$hasGit = [bool](Get-Command git -ErrorAction SilentlyContinue)
if (-not (Test-Path (Join-Path $repo '.git'))) {
    Fail 'this folder is not a git clone (downloaded as a ZIP?): download the project with "git clone", see GUIDE.md'
}
$sub = 'gpu\third_party\fsr-vulkan'
$subReady = (Test-Path "$sub\.git") -and (Test-Path 'third_party\LibAtrac9\C\src')
if ($subReady) { Ok 'submodules (FSR, ImGui, LibAtrac9) already downloaded' }
elseif ($CheckOnly -or -not $hasGit) { Missing 'submodules (FSR, ImGui, LibAtrac9)' }
else {
    Doing 'submodules (FSR, ImGui, LibAtrac9)'
    & git submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) { Fail 'downloading the submodules failed' }
}
if ($hasGit -and (Test-Path "$sub\.git")) {
    $patch = '..\..\..\patches\fsr_vulkan_mingw.patch'
    & git -C $sub apply --reverse --check $patch 2>$null
    if ($LASTEXITCODE -eq 0) { Ok 'FSR fix for MinGW already applied' }
    elseif ($CheckOnly) { Missing 'FSR fix for MinGW' }
    else {
        Doing 'FSR fix for MinGW'
        & git -C $sub apply $patch
        if ($LASTEXITCODE -ne 0) { Fail 'the FSR fix does not apply' }
    }
}
if ($dlss) {
    $sdk = Join-Path $repo 'modules\dlss-sdk'
    $sibling = Join-Path (Split-Path $repo -Parent) 'dlss-sdk'
    # All three files the build needs: a download cut short (disk full, lost connection) can leave
    # the folder with only some of them.
    function SdkComplete($dir) {
        return (Test-Path "$dir\include\nvsdk_ngx_vk.h") -and (Test-Path "$dir\lib\Windows_x86_64\x64\nvsdk_ngx_s.lib") -and
               (Test-Path "$dir\lib\Windows_x86_64\rel\nvngx_dlss.dll")
    }
    if ((Test-Path "$sdk\.git") -and -not (SdkComplete $sdk) -and -not $CheckOnly -and $hasGit) {
        Doing 'DLSS SDK in modules\dlss-sdk is incomplete: restoring its files'
        & git -C $sdk checkout -f HEAD
        if (-not (SdkComplete $sdk)) { Fail 'the DLSS SDK in modules\dlss-sdk is incomplete and could not be restored: delete the modules\dlss-sdk folder and run setup.bat again' }
    }
    if (SdkComplete $sdk) { Ok 'DLSS SDK in modules\dlss-sdk' }
    elseif (SdkComplete $sibling) { Ok "DLSS SDK in $sibling" }
    elseif (Test-Path $sdk) {
        if ($CheckOnly) { Missing 'DLSS SDK in modules\dlss-sdk is incomplete' }
        else { Fail 'the DLSS SDK in modules\dlss-sdk is incomplete: delete the modules\dlss-sdk folder and run setup.bat again' }
    }
    elseif ($CheckOnly -or -not $hasGit) { Missing 'DLSS SDK in modules\dlss-sdk' }
    else {
        Doing 'NVIDIA DLSS SDK in modules\dlss-sdk (about 1.6 GB)'
        & git clone --depth 1 --branch v310.9.1 https://github.com/NVIDIA/DLSS.git $sdk
        if ($LASTEXITCODE -ne 0) { Fail 'downloading the DLSS SDK failed' }
    }
}

Section 'FSR 4'
# The FSR 4 (v07, INT8) shaders and model weights are not in the repository: the port's own script,
# tools/fetch_fsr4_assets.sh, downloads them and then fixes and speeds up some in fsr4_shaders\opt.
# Without them "FSR 4 (INT8)" fails in the menu with "missing fsr4_shaders/...". Any GPU.
if ((Test-Path 'fsr4_shaders\rcas.spv') -and (Get-ChildItem 'fsr4_shaders\opt\*.spv' -ErrorAction SilentlyContinue)) {
    Ok 'FSR 4 shaders already present in fsr4_shaders'
}
elseif ($CheckOnly) { Warn 'FSR 4 shaders missing: FSR 4 (INT8) will not work until you run setup.bat' }
elseif (-not (Test-Path "$Msys\mingw64\bin\spirv-cross.exe")) { Warn 'spirv-cross is missing: skipping the FSR 4 shaders' }
else {
    Doing 'FSR 4 shaders (about 45 MB, from github.com/FireBurn/Q2RTX) in fsr4_shaders'
    # With Git for Windows the scripts may have CRLF line endings, which bash and the Perl rewrites reject.
    $command = "sed -i 's/\r`$//' tools/fetch_fsr4_assets.sh tools/fsr4_optimize.sh tools/*.pl tools/*.pm && bash tools/fetch_fsr4_assets.sh"
    if ((Bash $command) -ne 0) { Warn 'downloading or preparing the FSR 4 shaders failed: FSR 4 (INT8) will not be available, FSR 3.1 will. Run setup.bat again to retry' }
}

Section 'Game'
$game = Join-Path $repo 'modules\game\CUSA03173'
if (Test-Path "$game\eboot.bin") { Ok 'game in modules\game\CUSA03173' }
else { Warn 'game not found in modules\game\CUSA03173: copy it there, or pick eboot.bin in the launcher' }

if ($CheckOnly) {
    Write-Host ''
    if ($script:missing) { Write-Host "Missing items: $script:missing. Run setup.bat without -CheckOnly to install them." -ForegroundColor Red; exit 1 }
    Write-Host 'All requirements are present.' -ForegroundColor Green
    exit 0
}

Section 'Build'
if (Get-Process bbport -ErrorAction SilentlyContinue) { Fail 'the game is running: close it and run setup.bat again' }
Write-Host 'The first time takes 20-30 minutes.'
# The build script uses this Python even when it is not on PATH.
$env:PYTHON = $py -replace '\\', '/'
if ((Bash 'bash scripts/windows/build_bbport.sh') -ne 0) { Fail 'building the game failed' }
if ($dlss) {
    & cmd /c 'gpu\dlss\build_bbdlss.bat'
    if ($LASTEXITCODE -ne 0) { Fail 'building the DLSS part failed: the lines above give the reason. The game itself is built; run setup.bat -NoDlss to finish without DLSS' }
}
$expected = @('out\bbport.exe')
if ($dlss) { $expected += 'out\bbdlss.dll', 'out\nvngx_dlss.dll' }
foreach ($f in $expected) { if (-not (Test-Path $f)) { Fail "$f is missing after the build" } }

if (-not $NoShortcut) {
    Section 'Bloodborne.exe and desktop shortcut'
    # The icon is never part of the repository. It is modules\icon.ico when the user put one there,
    # otherwise it is made from the icon inside the user's own game dump (sce_sys\icon0.png).
    $icon = Join-Path $repo 'modules\icon.ico'
    if (Test-Path $icon) { Ok 'icon: modules\icon.ico' }
    else {
        $icon = $null
        $png = Join-Path $game 'sce_sys\icon0.png'
        if (Test-Path $png) {
            $made = Join-Path $repo 'out\bloodborne.ico'
            & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'make_icon.ps1') -Png $png -Out $made | Out-Host
            if ($LASTEXITCODE -eq 0) { $icon = $made; Ok 'icon: made from the game''s own sce_sys\icon0.png (out\bloodborne.ico)' }
            else { Warn 'could not make the icon from icon0.png' }
        }
        if (-not $icon) { Warn 'no icon: put an .ico file in modules\icon.ico and run setup.bat again to get one' }
    }
    $resource = ''
    if ($icon) {
        Set-Content -Path 'out\bloodborne_icon.rc' -Value ('1 ICON "' + $icon.Replace('\', '/') + '"') -Encoding ASCII
        $resource = 'windres out/bloodborne_icon.rc -O coff -o out/bloodborne_icon.o && '
    }
    $objects = if ($icon) { ' out/bloodborne_icon.o' } else { '' }
    $build = $resource + 'gcc -O2 -s -mwindows scripts/windows/bloodborne_launcher.c' + $objects + ' -o Bloodborne.exe'
    if ((Bash $build) -ne 0 -or -not (Test-Path 'Bloodborne.exe')) { Warn 'building Bloodborne.exe failed: start the port with launch_gui.bat' }
    else {
        Ok 'Bloodborne.exe built'
        try {
            $link = Join-Path ([Environment]::GetFolderPath('Desktop')) 'Bloodborne.lnk'
            $shortcut = (New-Object -ComObject WScript.Shell).CreateShortcut($link)
            $shortcut.TargetPath = Join-Path $repo 'Bloodborne.exe'
            $shortcut.WorkingDirectory = $repo
            $shortcut.IconLocation = (Join-Path $repo 'Bloodborne.exe') + ',0'
            $shortcut.Description = 'Bloodborne PC port launcher'
            $shortcut.Save()
            Ok "desktop shortcut: $link"
        } catch { Warn "could not create the desktop shortcut: $($_.Exception.Message)" }
    }
}

Write-Host ''
Write-Host 'Setup complete.' -ForegroundColor Green
Write-Host 'To play: the Bloodborne shortcut on the desktop, launch_gui.bat (launcher), or run.bat.'
