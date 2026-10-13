# Runs the tests that work on Windows without a graphics card and without the game. Used by the
# GitHub Action and by hand before a release. The port must already be built (setup.bat).
# Usage:  powershell -ExecutionPolicy Bypass -File scripts\windows\run_tests.ps1
# Exit code 0 when every test passed.
param([string]$Msys = 'C:\msys64')
$ErrorActionPreference = 'Continue'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $repo
$failed = New-Object System.Collections.Generic.List[string]
function Result($name, $ok) {
    if ($ok) { Write-Host "[PASS] $name" -ForegroundColor Green }
    else { Write-Host "[FAIL] $name" -ForegroundColor Red; $failed.Add($name) }
}
function Bash($command) {
    $env:CHERE_INVOKING = '1'; $env:MSYSTEM = 'MINGW64'; $env:MSYS2_PATH_TYPE = ''
    & "$Msys\usr\bin\bash.exe" -lc $command | Out-Host
    return $LASTEXITCODE
}

if (-not (Test-Path 'out\gpu\build.ninja')) { Write-Host '[ERROR] the port is not built: run setup.bat first' -ForegroundColor Red; exit 1 }
$py = $env:PYTHON
if (-not $py -and (Test-Path 'python_path.txt')) { $py = (Get-Content 'python_path.txt' -TotalCount 1).Trim() }
if (-not $py) { $py = (& where.exe python 2>$null | Where-Object { $_ -notmatch 'WindowsApps' } | Select-Object -First 1) }
if (-not $py) { $py = 'python' }

Write-Host '== Menu translations' -ForegroundColor Cyan
& $py tests\check_i18n.py gpu\shim\bbport_i18n.cpp | Out-Host
Result 'menu table: one text per language, same placeholders' ($LASTEXITCODE -eq 0)

Write-Host '== Python sources' -ForegroundColor Cyan
& $py -m py_compile launcher.py (Get-ChildItem scripts\*.py | ForEach-Object FullName) | Out-Host
Result 'launcher.py and scripts compile' ($LASTEXITCODE -eq 0)

Write-Host '== Python tests' -ForegroundColor Cyan
# The other tests in tests\ drive run.sh or Linux behaviour and do not pass on Windows.
foreach ($test in 'test_patches', 'test_prepare', 'test_content', 'test_upscaler_assets', 'test_savedata_import', 'test_bb_guard') {
    Push-Location tests
    & $py -m unittest $test 2>&1 | Select-Object -Last 4 | Out-Host
    $ok = $LASTEXITCODE -eq 0
    Pop-Location
    Result "python: $test" $ok
}

Write-Host '== C++ tests' -ForegroundColor Cyan
# Only the targets that need no Vulkan device (the GitHub runners have none).
foreach ($test in 'upscaler-support-test', 'shader-user-data-test', 'motion-history-test', 'motion-shader-test', 'ui-composition-test') {
    $ok = (Bash "cmake --build out/gpu --target $test > out/test_$test.log 2>&1") -eq 0
    if (-not $ok) { Get-Content "out\test_$test.log" -Tail 20 | Out-Host }
    else { $ok = (Bash "cd out/gpu && ./$test.exe > ../test_run_$test.log 2>&1; code=`$?; tail -2 ../test_run_$test.log; exit `$code") -eq 0 }
    Result "c++: $test" $ok
}

Write-Host ''
if ($failed.Count) { Write-Host "$($failed.Count) test(s) failed: $($failed -join ', ')" -ForegroundColor Red; exit 1 }
Write-Host 'All tests passed.' -ForegroundColor Green
