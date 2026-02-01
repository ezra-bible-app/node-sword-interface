#!/bin/sh
#   This file is part of node-sword-interface.
#
#   Copyright (C) 2019 - 2026 Tobias Klein <contact@tklein.info>
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

set -e

# Accept arguments from GYP, or fallback to Xcode env vars
# Arg 1: Build Directory (where the binary is)
# Arg 2: Executable Name
BUILD_DIR="${1:-$BUILT_PRODUCTS_DIR}"
EXECUTABLE_NAME="${2:-$EXECUTABLE_PATH}"

# Fallback for executable name if not provided or in env
if [ -z "$EXECUTABLE_NAME" ]; then
    EXECUTABLE_NAME="node_sword_interface"
fi

FRAMEWORK_NAME="node_sword_interface"
FRAMEWORK_DIR="${BUILD_DIR}/${FRAMEWORK_NAME}.framework"
BINARY_SOURCE="${BUILD_DIR}/${EXECUTABLE_NAME}.dylib"

echo "Creating iOS Framework structure at ${FRAMEWORK_DIR}"
echo "Using binary: ${BINARY_SOURCE}"

# Ensure the framework directory exists
mkdir -p "${FRAMEWORK_DIR}"

# Copy the binary
cp "${BINARY_SOURCE}" "${FRAMEWORK_DIR}/${FRAMEWORK_NAME}"

# Create Info.plist
cat > "${FRAMEWORK_DIR}/Info.plist" <<PLIST_EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>${FRAMEWORK_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>net.ezrabibleapp.node-sword-interface</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>${FRAMEWORK_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>FMWK</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>MinimumOSVersion</key>
    <string>${IPHONEOS_DEPLOYMENT_TARGET}</string>
    <key>CFBundleSupportedPlatforms</key>
    <array>
        <string>iPhoneOS</string>
    </array>
</dict>
</plist>
PLIST_EOF

# Sign the framework (ad-hoc)
/usr/bin/codesign --force --sign - --timestamp=none "${FRAMEWORK_DIR}"

# Remove the source binary
rm "${BINARY_SOURCE}"

echo "Framework created successfully."
