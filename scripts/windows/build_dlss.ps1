# Builds the DLSS part (out\bbdlss.dll + out\nvngx_dlss.dll) on this PC, in an installed copy of
# the port or in the repository. Nothing of NVIDIA is shipped with the port: this script downloads
# the few SDK files the bridge needs from NVIDIA's own repository and compiles the bridge here.
# Usage:  build_dlss.bat [-Yes] [-AnyGpu] [-Pause]
#   -Yes     does not ask before downloading or installing
#   -AnyGpu  builds even without an NVIDIA RTX card (DLSS will not run on such a PC)
#   -Pause   waits for Enter when something failed, so the window stays readable
param(
    [switch]$Yes,
    [switch]$AnyGpu,
    [switch]$Pause
)
# 'Continue': PowerShell 5.1 with 'Stop' treats any stderr line of an external program as fatal.
$ErrorActionPreference = 'Continue'
$ProgressPreference = 'SilentlyContinue'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $root

function Ok($t)    { Write-Host "[OK]      $t" -ForegroundColor Green }
function Doing($t) { Write-Host "[INSTALL] $t" -ForegroundColor Yellow }
function Fail($t)  {
    Write-Host "[ERROR]   $t" -ForegroundColor Red
    if ($Pause) { [void](Read-Host 'Press Enter to close') }
    exit 1
}
function Confirm($question) {
    if ($Yes) { return $true }
    return (Read-Host "$question [y/N]") -match '^(y|yes)$'
}

# The SDK release the bridge is written for, and the two binaries it takes from it.
$tag = 'v310.9.1'
$base = "https://raw.githubusercontent.com/NVIDIA/DLSS/$tag"
$lib = 'lib\Windows_x86_64\x64\nvsdk_ngx_s.lib'
$dll = 'lib\Windows_x86_64\rel\nvngx_dlss.dll'
$sha256 = @{
    $lib = '4e5d355086d2bc11e1a0842457d2519ea528ee1f3e112c45679a84960c07dff3'
    $dll = '3975567b8943c53acce397f2b72380092f84f162d00b0d2c7d08a1025c563983'
}
# The headers bbdlss.cpp includes, directly or through one another.
$headers = 'nvsdk_ngx.h', 'nvsdk_ngx_defs.h', 'nvsdk_ngx_defs_vk.h', 'nvsdk_ngx_helpers.h', 'nvsdk_ngx_helpers_cuda.h',
           'nvsdk_ngx_helpers_d3d.h', 'nvsdk_ngx_helpers_vk.h', 'nvsdk_ngx_loader.h', 'nvsdk_ngx_params.h', 'nvsdk_ngx_vk.h'

if (-not (Test-Path 'out\bbport.exe')) { Fail "out\bbport.exe is not in $root`: this is not an installed or built copy of the port" }
if (Get-Process bbport -ErrorAction SilentlyContinue) { Fail 'the game is running: close it and try again' }

$rtx = [bool](Get-CimInstance Win32_VideoController | Where-Object { $_.Name -match 'RTX' })
if ($rtx) { Ok 'NVIDIA RTX card found' }
elseif ($AnyGpu) { Write-Host '[WARNING] no NVIDIA RTX card: building anyway (-AnyGpu), DLSS will not run on this PC' -ForegroundColor Yellow }
else { Fail 'no NVIDIA RTX card in this PC: DLSS cannot run here. FSR 3.1, FSR 4 and TAA work on any card' }

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
function FindMsvc {
    if (-not (Test-Path $vswhere)) { return $null }
    return & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}
$vs = FindMsvc
if ($vs) { Ok "Visual Studio with C++ tools ($vs)" }
else {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Fail 'the Microsoft C++ compiler is missing and winget is not available to install it: install "Visual Studio Build Tools" with the "Desktop development with C++" workload, then run this again'
    }
    Write-Host 'The Microsoft C++ compiler is needed: Visual Studio Build Tools with C++, about 7 GB, from Microsoft (winget).'
    if (-not (Confirm 'Download and install it now?')) { Fail 'stopped: nothing was installed' }
    Doing 'Visual Studio Build Tools with C++ (this takes a while)'
    & winget install --id Microsoft.VisualStudio.2022.BuildTools -e --override '--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended' | Out-Host
    $vs = FindMsvc
    if (-not $vs) { Fail 'the Visual Studio Build Tools were not installed (the lines above give the reason)' }
    Ok "Visual Studio with C++ tools ($vs)"
}

# A full checkout of the SDK (setup.bat makes one when building from source) is used as it is.
$sdk = Join-Path $root 'modules\dlss-sdk'
function SdkComplete { return (Test-Path "$sdk\include\nvsdk_ngx_vk.h") -and (Test-Path "$sdk\$lib") -and (Test-Path "$sdk\$dll") }
if (SdkComplete) { Ok 'NVIDIA DLSS SDK files in modules\dlss-sdk' }
else {
    Write-Host "The NVIDIA DLSS SDK files are needed: about 63 MB, from github.com/NVIDIA/DLSS ($tag), into modules\dlss-sdk."
    Write-Host 'They are NVIDIA''s and stay under NVIDIA''s licence (modules\dlss-sdk\LICENSE.txt after the download);'
    Write-Host 'downloading them means accepting it: https://github.com/NVIDIA/DLSS/blob/main/LICENSE.txt'
    if (-not (Confirm 'Download them now?')) { Fail 'stopped: nothing was downloaded' }
    Doing 'NVIDIA DLSS SDK files'
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    $files = @('LICENSE.txt') + ($headers | ForEach-Object { "include\$_" }) + $lib, $dll
    foreach ($file in $files) {
        $target = Join-Path $sdk $file
        New-Item -ItemType Directory -Force (Split-Path $target -Parent) | Out-Null
        # Downloaded under another name: a download cut short never looks like a complete file.
        try { Invoke-WebRequest -Uri "$base/$($file.Replace('\', '/'))" -OutFile "$target.part" -UseBasicParsing }
        catch { Fail "downloading $file failed: $($_.Exception.Message)" }
        if ($sha256[$file] -and (Get-FileHash "$target.part" -Algorithm SHA256).Hash -ne $sha256[$file]) {
            Remove-Item "$target.part" -Force
            Fail "$file is not the expected file (checksum mismatch): nothing was built"
        }
        Move-Item "$target.part" $target -Force
    }
    if (-not (SdkComplete)) { Fail 'the SDK files are incomplete after the download' }
    Ok 'NVIDIA DLSS SDK files in modules\dlss-sdk'
}

# Vulkan headers: an installed copy carries them, a source checkout takes them from MSYS2.
$shipped = Join-Path $root 'gpu\dlss\vk_include'
if (Test-Path "$shipped\vulkan\vulkan.h") { $env:VK_INC = $shipped }
elseif (-not (Test-Path 'C:\msys64\mingw64\include\vulkan\vulkan.h')) { Fail 'the Vulkan headers are missing (gpu\dlss\vk_include)' }
$env:DLSS_SDK = $sdk

Doing 'building out\bbdlss.dll'
& cmd /c (Join-Path $root 'gpu\dlss\build_bbdlss.bat') | Out-Host
if ($LASTEXITCODE -ne 0 -or -not (Test-Path 'out\bbdlss.dll') -or -not (Test-Path 'out\nvngx_dlss.dll')) {
    Fail 'building the DLSS part failed: the lines above give the reason'
}
Write-Host ''
Write-Host 'DLSS is ready: pick it as the upscaler in the in-game menu (Insert) or in the launcher.' -ForegroundColor Green
