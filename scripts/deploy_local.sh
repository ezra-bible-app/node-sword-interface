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

EZRA_PROJECT_DIR="ezra-project-git"
NODE_SWORD_INTERFACE_DIR="node-sword-interface-git"

cd ..
rm -rf ${EZRA_PROJECT_DIR}/node_modules/node-sword-interface
cp -a ${NODE_SWORD_INTERFACE_DIR} ${EZRA_PROJECT_DIR}/node_modules/node-sword-interface
cd ${EZRA_PROJECT_DIR}
npm run rebuild
