#!/bin/sh
# This script is used by binding.gyp to dynamically determine which SWORD library to link against.
# It returns the system-installed SWORD library (using pkg-config) if the environment variable LINK_SYSTEM_SWORD has the value 1
# or the content of the first parameter in all other cases.

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    pkg-config --libs sword
else
    echo $1
fi