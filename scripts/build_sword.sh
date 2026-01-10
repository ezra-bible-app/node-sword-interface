#!/bin/sh

#   This file is part of node-sword-interface.
#
#   Copyright (C) 2019 - 2025 Tobias Klein <contact@tklein.info>
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

# CLONE SWORD
git clone https://github.com/ezra-bible-app/crosswire-sword-mirror sword
git -C sword checkout 1.9.0-2025-12-30
patch --batch --forward -d sword -p 0 < patch/sword_globconf.patch

# PATCHES
case "$(uname -s)" in
    Darwin)
    # We only apply the following patch on macOS
        export MACOSX_DEPLOYMENT_TARGET=10.10
    ;;
esac

# BUILD
mkdir -p sword_build

SWORD_BUILD_TYPE="Release"

# Newer CMake versions have removed compatibility with CMake < 3.5.
# SWORD is configured with CMake minimum 2.6.0, which leads to a build error with newer CMake versions (e.g. 4.0.3).
export CMAKE_POLICY_VERSION_MINIMUM=3.5

if [ "$1" = "--android" ] ; then
  git clone https://github.com/karlkleinpaste/biblesync.git
  git -C biblesync checkout 2.1.0

  echo "-- TARGET ARCH: $2"
  TARGET_ARCH=$2

  if [ "$TARGET_ARCH" = "arm64" ] ; then
    ANDROID_ABI="arm64-v8a"
  elif [ "$TARGET_ARCH" = "arm" ]; then
    ANDROID_ABI="armeabi-v7a"
  elif [ "$TARGET_ARCH" = "x86" ]; then
    ANDROID_ABI="x86"
  elif [ "$TARGET_ARCH" = "x86_64" ]; then
    ANDROID_ABI="x86_64"
  fi

  cd sword_build

  cmake -DLIBSWORD_LIBRARY_TYPE=Static -DCMAKE_CXX_STANDARD=11 \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
  -DANDROID_NDK=$ANDROID_NDK_HOME \
  -DCMAKE_BUILD_TYPE=$SWORD_BUILD_TYPE \
  -DANDROID_ABI="$ANDROID_ABI" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,-z,max-page-size=16384" \
  -DCMAKE_SHARED_LINKER_FLAGS="-Wl,-z,max-page-size=16384" \
  -DNODYNCAST=1 \
  ../sword
elif [ "$1" = "--ios" ] ; then
  git clone https://github.com/karlkleinpaste/biblesync.git
  git -C biblesync checkout 2.1.0

  IOS_TARGET="$2"
  IOS_VER="$3"
  IOS_ARCH="$4"

  if [ -z "$IOS_VER" ]; then
      IOS_VER="13.0"
  fi

  if [ -z "$IOS_ARCH" ]; then
      IOS_ARCH="arm64"
  fi

  # Replace spaces with semicolons for CMake list
  IOS_ARCH=$(echo "$IOS_ARCH" | tr ' ' ';')

  SYSROOT="iphoneos"
  if echo "$IOS_TARGET" | grep -q "simulator"; then
      SYSROOT="iphonesimulator"
  fi

  cd sword_build

  cmake -DCMAKE_SYSTEM_NAME=iOS \
  -DLIBSWORD_LIBRARY_TYPE=Static \
  -DCMAKE_CXX_STANDARD=11 \
  -DCMAKE_OSX_ARCHITECTURES="$IOS_ARCH" \
  -DCMAKE_OSX_SYSROOT=$SYSROOT \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=$IOS_VER \
  -DCMAKE_BUILD_TYPE=$SWORD_BUILD_TYPE \
  ../sword
else
  # macOS & Linux

  cd sword_build

  if [ "$(uname -s)" = "Darwin" ]; then
    cmake -DLIBSWORD_LIBRARY_TYPE=Static -DCMAKE_CXX_STANDARD=11 \
    -DCMAKE_BUILD_TYPE=$SWORD_BUILD_TYPE \
    -DCMAKE_DISABLE_FIND_PACKAGE_ICU=TRUE \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    ../sword
  else
    cmake -DLIBSWORD_LIBRARY_TYPE=Static -DCMAKE_CXX_STANDARD=11 \
    -DCMAKE_BUILD_TYPE=$SWORD_BUILD_TYPE \
    -DCMAKE_DISABLE_FIND_PACKAGE_ICU=TRUE \
    ../sword
  fi
fi

make -j4 sword_static
