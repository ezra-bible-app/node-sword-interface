#!/bin/sh
# This script is used by binding.gyp to dynamically determine the include path of the SWORD library.
# It returns the system include path of the SWORD library if the environment variable
# LINK_SYSTEM_SWORD has the value 1 or otherwise the path to the locally cloned sword/include directory.

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    # This command turns something like -I/usr/include/sword into /usr/include/sword (it cuts out all character from the 3rd one)
    pkg-config --cflags sword | cut -c3-
else
    echo "sword/include"
fi