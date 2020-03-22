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
- Search within a bible module
- Retrieve info based on Strong's number

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

### Dependencies

Before installing node-sword-interface you need to make sure that the following dependencies are installed:
- C++11 compiler toolchain
- nodejs (A version that supports N-API version >= 4, like 8.16.0, 10.16.0 or 12.0.0 (see [N-API Version Matrix](https://nodejs.org/api/n-api.html#n_api_n_api_version_matrix)))
- Git
- Mac/Linux dependencies:
    - ICU library with development headers
    - CURL library with development headers
    - CMake (for building the SWORD library)

Below you find the OS-specific instructions for installing the dependencies.

#### Install dependencies on Linux

These installation instructions are working on Debian/Ubuntu based Linux distributions.

To install the dependencies issue the following command on a Debian/Ubuntu based distribution:
    
    sudo apt-get install build-essential nodejs npm libcurl4-gnutls-dev libicu-dev pkg-config cmake

#### Install dependencies on macOS

1. Install XCode from the App Store
2. Install Command Line Developer Tools (contains Compiler toolchain, git, etc.) by running this command:<br/> `xcode-select --install`   
3. Install the homebrew package manager by running this command:<br/> `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
4. Install some packages with homebrew by running this command:<br/> `brew install pkg-config cmake npm icu4c`

#### Install dependencies on Windows

1. Install [git](https://git-scm.com/download/win) and [nodejs](https://nodejs.org).
2. Install [windows-build-tools][windows-build-tools] (VS 2015 Compiler) by running the following command in an administrator powershell:<br/> `npm install --vs2015 --global windows-build-tools`.

[windows-build-tools]: https://www.npmjs.com/package/windows-build-tools

### Install from npmjs

Once the dependencies are available you can install and build the latest node-sword-interface release from [npmjs.com](https://www.npmjs.com/package/node-sword-interface) by issuing the following command:

    npm install node-sword-interface

On Windows, you need to add `--arch=ia32` at the end of the command:

    npm install node-sword-interface --arch=ia32
