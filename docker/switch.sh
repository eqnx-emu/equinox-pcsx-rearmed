#!/usr/bin/env bash
# Builds equinox-pcsx-rearmed.nro inside the devkitPro container.
# The CMake build tree lives in a Docker volume (/build) because bind mounts on
# Windows are slow; only the final files are copied back to the repo.
set -euo pipefail

SRC=/src
BUILD=/build/switch
OUT="$SRC/build/docker"
JOBS="${JOBS:-$(nproc)}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
CORE="$SRC/extern/pcsx_rearmed"

git config --global --add safe.directory '*'

if [ ! -f "$CORE/Makefile.libretro" ]; then
    echo "Submodules are missing. Run: git submodule update --init --recursive" >&2
    exit 1
fi

# SDL2 and SDL2_ttf come from devkitPro portlibs.
for pkg in switch-sdl2 switch-sdl2_ttf switch-freetype switch-libpng switch-bzip2 switch-zlib; do
    dkp-pacman -Q "$pkg" >/dev/null 2>&1 || dkp-pacman -S --noconfirm "$pkg"
done

# PCSX ReARMed core as a partially linked static library (only retro_* symbols
# stay global): extern/pcsx_rearmed/pcsx_rearmed_libretro_libnx.a
make -C "$CORE" -f Makefile.libretro platform=libnx -j"$JOBS"

cmake -S "$SRC" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD" --target equinox-pcsx-rearmed_nro -j"$JOBS"

mkdir -p "$OUT"
cp "$BUILD/equinox-pcsx-rearmed.nro" "$OUT/equinox-pcsx-rearmed.nro"
cp "$BUILD/equinox-pcsx-rearmed.elf" "$OUT/equinox-pcsx-rearmed.elf"
echo "Done: build/docker/equinox-pcsx-rearmed.nro"
