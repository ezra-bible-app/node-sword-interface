# node-sword-interface
node-sword-interface is a [nodejs](https://nodejs.org) module that wraps the [SWORD library](http://www.crosswire.org/sword/) which gives access to bible text modules and related ressources. It has been created for use within the [Ezra Project](https://github.com/tobias-klein/ezra-project) bible study software, but it can also be used for any other nodejs-based software that needs to interface with the SWORD library. The focus of node-sword-interface is on bible modules. Other types of SWORD modules are currently not supported. node-sword-interface works on Linux, macOS and Windows.

## Features

node-sword-interface focusses on discovery, installation/removal and text loading of SWORD modules. It currently covers the following usecases:

- Update SWORD repository configuration
- List SWORD repositories
- List remote bible modules
- List local bible modules
- Install a bible module
- Uninstall a bible module
- Get information about a bible module
- Get the text of a bible book
- Get the text of the whole bible

## Example

```javascript
const NodeSwordInterface = require('node-sword-interface');
var interface = new NodeSwordInterface();

interface.updateRepositoryConfig();

// Install the King James Version
interface.installModule('KJV');

// Print some module information
var kjv = interface.getLocalModule('KJV');
console.log(kjv.description);
console.log(kjv.about);

// Get the verses of the Gospel of Matthew
var verses = interface.getBookText('KJV', 'Mat');

// Do something with the verses
for (var i = 0; i < verses.length; i++) {
    var currentVerse = verses[i];
    var verseReference = currentVerse.chapter + ':' + currentVerse.verseNr;
    console.log(verseReference + ' '  + currentVerse.content);
}
```

## API Docs

The Javascript API of node-sword-interface is documented [here](API.md).

## Installation

### Linux

These installation instructions are working on Debian/Ubuntu based Linux distributions.
Before installing node-sword-interface you need to make sure that the following dependencies are installed:

- C++ compiler toolchain
- nodejs
- SQLite 3 library
- ICU library with development headers
- CURL library with development headers
- CMake (for building the SWORD library)
- Subversion (for retrieving the SWORD sources)

To install the dependencies issue the following command on a Debian/Ubuntu based distribution:
    
    sudo apt-get install build-essential nodejs libsqlite3-0 libcurl4-gnutls-dev libicu-dev pkg-config cmake subversion

Once the dependencies are available you can install and build the latest node-sword-interface release from [npmjs.com](https://www.npmjs.com/package/node-sword-interface) by issuing the following command:

    npm install node-sword-interface


### macOS

_Instructions to be added_

### Windows

_Instructions to be added_
