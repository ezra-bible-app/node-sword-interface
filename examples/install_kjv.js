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
