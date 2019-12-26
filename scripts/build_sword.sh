#!/bin/sh

# CHECKOUT
git clone https://github.com/bibletime/crosswire-sword-mirror sword
git -C sword checkout tags/sword-1-8-1

# PATCHES
case "$(uname -s)" in
    Linux)
    # We only apply the Sword ICU patch on Linux
        patch --batch --forward -d sword -p 0 < patch/sword_icu.patch
    ;;
    Darwin)
    # We only apply the following patch on macOS
        export CMAKE_PREFIX_PATH=/usr/local/opt/icu4c
        patch --batch --forward -d sword -p 0 < patch/sword_mac.patch
    ;;
esac

patch --batch --forward -d sword -p 0 < patch/sword_globconf.patch

# BUILD
mkdir -p sword_build
cd sword_build
cmake -DLIBSWORD_LIBRARY_TYPE=Static -DCMAKE_CXX_STANDARD=11 ../sword
make -j2 sword_static
