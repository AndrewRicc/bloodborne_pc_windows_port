#!/bin/bash
# Builds the Bloodborne PC port with MSYS2 MinGW64 (parallel version of build.bat), including
# the workarounds needed with recent MSYS2 packages (GCC 16, VMA header in include/vma).
# Usage (from an MSYS2 shell or bash -lc):  bash build_bbport.sh [/d/path/to/repo] [jobs]
# Without arguments it builds the repository this script belongs to.
set -e
REPO="${1:-$(cd "$(dirname "$0")/../.." && pwd)}"
JOBS="${2:-$(nproc)}"
export PATH=/mingw64/bin:$PATH
cd "$REPO"
mkdir -p out/gpu out/atrac9

if [ ! -f out/libatrac9.a ]; then
    echo "== LibAtrac9"
    for f in third_party/LibAtrac9/C/src/*.c; do
        gcc -std=c99 -O2 -g -w -c "$f" -o "out/atrac9/$(basename "${f%.c}").o"
    done
    ar rcs out/libatrac9.a out/atrac9/*.o
fi

# CMake's build folder remembers its absolute path: after the port's folder is moved or renamed it
# refuses to build. Start it again from scratch in that case.
if [ -f out/gpu/CMakeCache.txt ]; then
    cached=$(sed -n 's/^CMAKE_CACHEFILE_DIR:INTERNAL=//p' out/gpu/CMakeCache.txt | tr -d '\r')
    current=$(cygpath -m "$PWD/out/gpu")
    if [ "${cached,,}" != "${current,,}" ]; then
        echo "== The folder moved (was $cached): rebuilding from scratch"
        rm -rf out/gpu
        mkdir -p out/gpu
    fi
fi

if [ ! -f out/gpu/build.ninja ]; then
    echo "== CMake configure"
    cmake -S gpu -B out/gpu -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_PREFIX_PATH=C:/msys64/mingw64 \
        -DVMA_INC=C:/msys64/mingw64/include/vma \
        "-DCMAKE_C_FLAGS=-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00" \
        "-DCMAKE_CXX_FLAGS=-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00"
fi

echo "== Build ($JOBS jobs)"
cmake --build out/gpu --target bbgpu bbport bb-gpu-capabilities -- -j "$JOBS"
cp -f out/bbport.exe out/bb-probe.exe

# Runtime DLLs next to bbport.exe; Windows' Python (not MSYS2's) as the scripts expect.
# Python is looked up in Windows' own PATH (registry), not inherited into this shell: stage_dlls.py
# copies every imported DLL it finds on PATH, so PATH must hold MSYS2 only. Do not run this script
# with MSYS2_PATH_TYPE=inherit.
if [ "${MSYS2_PATH_TYPE:-}" = "inherit" ]; then
    echo "Run without MSYS2_PATH_TYPE=inherit: foreign DLLs on PATH would be staged next to the game" >&2
    exit 1
fi
PY="${PYTHON:-}"
if [ -z "$PY" ]; then
    PY=$(/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe -NoProfile -Command '
        $p = [Environment]::GetEnvironmentVariable("Path","User") + ";" + [Environment]::GetEnvironmentVariable("Path","Machine")
        foreach ($d in $p -split ";") {
            if ($d -and $d -notmatch "WindowsApps" -and (Test-Path (Join-Path $d "python.exe"))) { Join-Path $d "python.exe"; break }
        }' | tr -d '\r')
fi
if [ -z "$PY" ]; then
    echo "Windows Python not found: set PYTHON=C:/.../python.exe" >&2
    exit 1
fi
MINGW_BIN=C:/msys64/mingw64/bin "$PY" scripts/stage_dlls.py
# stage_dlls.py also copies Windows system DLLs it resolves; the game must use the system ones.
rm -f out/gdiplus.dll out/WSOCK32.dll out/wsock32.dll out/bcryptprimitives.dll out/d2d1.dll
echo "== Build complete: out/bbport.exe"
