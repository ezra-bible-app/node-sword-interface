#!/bin/sh
# This script gets the SWORD sources from the Crosswire SWORD mirror repo (set up by the BibleTime folks)
# and then builds it as static library.

# If the environment variable LINK_SYSTEM_SWORD is set to 1 then we directly exit without building SWORD.
if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    echo "Linking system SWORD library!"
    exit 0
else
    echo "Linking self-compiled SWORD library"
fi

# CHECKOUT
git clone https://github.com/bibletime/crosswire-sword-mirror sword
git -C sword checkout c8129c

# PATCHES
case "$(uname -s)" in
#    Linux)
#    # We only apply the Sword ICU patch on Linux
#        patch --batch --forward -d sword -p 0 < patch/sword_icu.patch
#    ;;
    Darwin)
    # We only apply the following patch on macOS
        export CMAKE_PREFIX_PATH=/usr/local/opt/icu4c
#        patch --batch --forward -d sword -p 0 < patch/sword_mac.patch
    ;;
esac

patch --batch --forward -d sword -p 0 < patch/sword_globconf.patch

# BUILD
mkdir -p sword_build
cd sword_build
cmake -DLIBSWORD_LIBRARY_TYPE=Static -DCMAKE_CXX_STANDARD=11 ../sword
make -j2 sword_static
