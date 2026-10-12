# System check shown by the Windows installer. Writes one line per item to -Out:
#   OK|text    WARN|text    MISSING|text    and, when Python was found,   PYTHON|full path
# MISSING items stop the installation; WARN items do not.
param(
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$HasDlss = 0
)
$ErrorActionPreference = 'Continue'
$lines = New-Object System.Collections.Generic.List[string]
function Add($state, $text) { $lines.Add("$state|$text") }

try {
    $os = Get-CimInstance Win32_OperatingSystem
    if ([Environment]::Is64BitOperatingSystem -and [version]$os.Version -ge [version]'10.0') { Add OK "64-bit Windows: $($os.Caption)" }
    else { Add MISSING '64-bit Windows 10 or 11 is required' }
} catch { Add WARN 'Windows version not detected' }

try {
    Add-Type -Namespace Bb -Name Cpu -MemberDefinition '[DllImport("kernel32.dll")] public static extern bool IsProcessorFeaturePresent(uint feature);'
    $cpu = (Get-CimInstance Win32_Processor | Select-Object -First 1).Name.Trim()
    if ([Bb.Cpu]::IsProcessorFeaturePresent(40)) { Add OK "CPU with AVX2: $cpu" }
    else { Add MISSING "the CPU does not support AVX2, the game cannot run: $cpu" }
} catch { Add WARN 'CPU features not detected' }

$rtx = $false
try {
    foreach ($gpu in Get-CimInstance Win32_VideoController) {
        Add OK "Graphics card: $($gpu.Name)"
        if ($gpu.Name -match 'RTX') { $rtx = $true }
    }
} catch { Add WARN 'graphics card not detected' }

try {
    $api = $null
    $icd = Get-ChildItem "$env:SystemRoot\System32\DriverStore\FileRepository" -Recurse -Include 'nv-vk64.json', 'amd-vulkan64.json', 'igvk64.json' -ErrorAction SilentlyContinue
    foreach ($f in $icd) {
        try { $v = [version]((Get-Content $f.FullName -Raw | ConvertFrom-Json).ICD.api_version); if (-not $api -or $v -gt $api) { $api = $v } } catch {}
    }
    if ($api -and $api -ge [version]'1.3') { Add OK "Vulkan driver $api" }
    elseif ($api) { Add MISSING "Vulkan driver ${api}: update the graphics driver (1.3 or newer is required)" }
    else { Add WARN 'Vulkan version not detected: make sure the graphics driver is up to date' }
} catch { Add WARN 'Vulkan version not detected' }

Add OK 'Upscalers: FSR 3.1, FSR 4 and TAA are included'
if ($HasDlss -and $rtx) { Add OK 'DLSS: included, NVIDIA RTX card found' }
elseif ($HasDlss) { Add WARN 'DLSS: included, but it needs an NVIDIA RTX card' }
else { Add WARN 'DLSS: not part of this package (build the port from source to get it)' }

# Python runs the launcher and prepares the game files at every start.
$python = $null
$candidates = @(& where.exe python 2>$null)
if (Get-Command py -ErrorAction SilentlyContinue) { $candidates += @(& py -3 -c "import sys;print(sys.executable)" 2>$null) }
foreach ($pattern in "$env:ProgramFiles\Python3*\python.exe", "$env:LocalAppData\Programs\Python\Python3*\python.exe",
                     "$env:LocalAppData\Python\*\python.exe") {
    $candidates += @(Get-ChildItem $pattern -ErrorAction SilentlyContinue | Sort-Object FullName -Descending | ForEach-Object FullName)
}
foreach ($p in $candidates) {
    if (-not $p -or $p -match 'WindowsApps' -or -not (Test-Path $p)) { continue }
    $v = & $p -c "import sys,tkinter;print('%d.%d' % sys.version_info[:2])" 2>$null
    if ($v -and [version]$v -ge [version]'3.10') { $python = $p; Add OK "Python $v with tkinter: $p"; break }
}
if ($python) { $lines.Add("PYTHON|$python") }
else { Add MISSING 'Python 3.10 or newer (with tkinter) was not found' }

[System.IO.File]::WriteAllLines($Out, $lines)
