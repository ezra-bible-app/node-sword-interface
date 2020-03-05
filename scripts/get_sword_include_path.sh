#!/bin/sh

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    echo "/usr/include/sword"
else
    echo "sword/include"
fi