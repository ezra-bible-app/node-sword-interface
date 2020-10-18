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

# This script is used by binding.gyp to dynamically determine which SWORD library to link against.
# It returns the system-installed SWORD library (using pkg-config) if the environment variable LINK_SYSTEM_SWORD has the value 1
# or the content of the first parameter in all other cases.

if [ "$LINK_SYSTEM_SWORD" = "1" ]; then
    pkg-config --libs sword
else
    echo $1
fi