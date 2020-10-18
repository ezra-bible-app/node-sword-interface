#!/bin/sh

#   This file is part of node-sword-interface.
#
#   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>
#
#   node-sword-interface is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 2 of the License, or
#   (at your option) any later version.
#
#   node-sword-interface is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#   See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with node-sword-interface. See the file COPYING.
#   If not, see <http://www.gnu.org/licenses/>.

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
git -C sword checkout 81e3f3

# PATCHES
case "$(uname -s)" in
#    Linux)
#    # We only apply the Sword ICU patch on Linux
#        patch --batch --forward -d sword -p 0 < patch/sword_icu.patch
#    ;;
    Darwin)
    # We only apply the following patch on macOS
        export MACOSX_DEPLOYMENT_TARGET=10.10
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
