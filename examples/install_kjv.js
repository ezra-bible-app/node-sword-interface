/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2021 Tobias Klein <contact@tklein.info>

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

// Note that this uses the sources within this directory structure.
// After installing via NPM you would use this require call: require('node-sword-interface');
const NodeSwordInterface = require('../index.js');
var interface = new NodeSwordInterface();

async function installKJV() {
  console.log("Updating repository configuration ...");
  await interface.updateRepositoryConfig();

  console.log("Installing King James module");
  // Install the King James Version (Uses the internet connection to download and install the module)
  await interface.installModule('KJV');
}

installKJV().then(() => {
  console.log("Installation of KJV successfully completed!");
});
