#!/bin/sh

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    pkg-config --libs sword
else
    echo $1
fi