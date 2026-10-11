#!/bin/bash
# MSYS2 MinGW64 packages for the Bloodborne PC port, plus the three libraries MSYS2 does not
# package (magic_enum, xbyak headers; miniz static library), installed into /mingw64.
# Usage: bash install_deps.sh [work dir for the library sources, default /tmp/bbport-deps]
set -e
WORK="${1:-/tmp/bbport-deps}"
pacman -S --needed --noconfirm git patch \
    mingw-w64-x86_64-{gcc,cmake,ninja,pkgconf,binutils,vulkan-headers,vulkan-loader} \
    mingw-w64-x86_64-{vulkan-memory-allocator,sdl3,ffmpeg,boost,fmt,robin-map,xxhash} \
    mingw-w64-x86_64-{glslang,shaderc,spirv-cross,spirv-headers,zydis}
export PATH=/mingw64/bin:$PATH
mkdir -p "$WORK" && cd "$WORK"
[ -d magic_enum ] || git -c advice.detachedHead=false clone -q --depth 1 --branch v0.9.8 https://github.com/Neargye/magic_enum.git
[ -d xbyak ] || git -c advice.detachedHead=false clone -q --depth 1 --branch v7.43 https://github.com/herumi/xbyak.git
[ -d miniz ] || git -c advice.detachedHead=false clone -q --depth 1 --branch 3.1.2 https://github.com/richgel999/miniz.git
cp -r magic_enum/include/magic_enum /mingw64/include/
cp -r xbyak/xbyak /mingw64/include/
cmake -S miniz -B miniz/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_FUZZERS=OFF -DCMAKE_INSTALL_PREFIX=/mingw64
cmake --build miniz/build
cmake --install miniz/build
cp /mingw64/include/miniz/*.h /mingw64/include/
echo "== Dependencies ready"
