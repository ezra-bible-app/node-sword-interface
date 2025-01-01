/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2025 Tobias Klein <contact@tklein.info>

   node-sword-interface is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   node-sword-interface is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with node-sword-interface. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>. */

const glob = require('glob');
const path = require('path');
const fs = require('fs');

function postInstall() {
  if (process.platform.indexOf("win") != -1) {
    console.log("Copying build dependencies to build output folder.");

    glob('build/sword-build-win32/lib/*.*', null, function (er, files) {
      files.forEach((srcPath) => {
        let fileName = path.basename(srcPath);
        let destPath = path.join('build/Release', fileName);
        fs.copyFileSync(srcPath, destPath);
      });
    });
  }
}

postInstall();