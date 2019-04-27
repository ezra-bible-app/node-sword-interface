# node-sword-interface
node-sword-interface is a Javascript nodejs library (using [N-API](https://nodejs.github.io/node-addon-api/)) that wraps the [SWORD library](http://www.crosswire.org/sword/) for use within nodejs applications. It has been created for use within the [Ezra Project](https://github.com/tobias-klein/ezra-project) bible study software, but it can also be used for any other Javascript-based software that needs to interface with the SWORD library. The focus is on bible modules, this library currently does not support other types of SWORD modules.

## Features

node-sword-interface focusses on discovery, installation and uninstallation of SWORD modules and on bible text loading for individual modules. It currently covers the following usecases:

- List SWORD repositories
- List modules within a given SWORD repository
- Install a module
- Uninstall a module
- Retrieve information about a local module
- Load the text of the whole bible
- Load the text of a bible book

## Installation

Before installing node-sword-interface you need to make sure that the following dependencies are installed:

- C++ compiler toolchain
- nodejs
- SWORD library with development headers
- SQLite 3 library
- CURL library with development headers

To install the dependencies issue the following command on a Debian/Ubuntu based distribution:
    
    sudo apt-get install build-essential nodejs libsword-dev libsqlite3-0 libcurl4-gnutls-dev

Once the dependencies are available you can install and build the latest node-sword-interface release from [npmjs.com](https://www.npmjs.com/package/node-sword-interface) by issuing the following command:

    npm install node-sword-interface

## API

The Javascript API of node-sword-interface is documented [here](API.md).
