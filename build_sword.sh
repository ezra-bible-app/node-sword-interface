#!/bin/sh

svn co http://crosswire.org/svn/sword/tags/sword-1-8-1/ sword
patch --batch -d sword -p 0 < sword_icu.patch
mkdir -p sword_build
cd sword_build
cmake -DLIBSWORD_LIBRARY_TYPE=Static ../sword
make -j2 sword_static
