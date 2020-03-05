#!/bin/sh
# This script is used by binding.gyp to dynamically determine the include path of the SWORD library.
# It returns the system include path of the SWORD library if the environment variable
# LINK_SYSTEM_SWORD has the value 1 or otherwise the path to the locally cloned sword/include directory.

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    echo "/usr/include/sword"
else
    echo "sword/include"
fi