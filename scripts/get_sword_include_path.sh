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

# This script is used by binding.gyp to dynamically determine the include path of the SWORD library.
# It returns the system include path of the SWORD library if the environment variable
# LINK_SYSTEM_SWORD has the value 1 or otherwise the path to the locally cloned sword/include directory.

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    # This command turns something like -I/usr/include/sword into /usr/include/sword (it cuts out all character from the 3rd one)
    pkg-config --cflags sword | cut -c3-
else
    echo "sword/include"
fi