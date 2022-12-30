const glob = require('glob');
const fs = require('fs');

function postInstall() {
  if (process.platform.indexOf("win") != -1) {
    console.log("Copying build dependencies to build output folder ...");

    // options is optional
    glob("build/sword-build-win32/lib/*.*", options, function (er, files) {
      // files is an array of filenames.
      // If the `nonull` option is set, and nothing
      // was found, then files is ["**/*.js"]
      // er is an error object or null.

      files.forEach((file) => {
        console.log(file);
      });
    });
  }
}

postInstall();