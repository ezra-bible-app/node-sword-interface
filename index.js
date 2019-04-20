/* This file is part of node-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@ezra-project.net>

   node-sword-interface is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   node-sword-interface is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with node-sword-interface. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>. */

const nodeSwordInterfaceModule = require('./build/Release/node_sword_interface.node');

class NodeSwordInterface {
  constructor() {
    this.nativeInterface = new nodeSwordInterfaceModule.NodeSwordInterface();
  }

  repositoryConfigExisting() {
    return this.nativeInterface.repositoryConfigExisting();
  }

  refreshRemoteSources() {
    return new Promise(resolve => {
      this.nativeInterface.refreshRemoteSources(true, function() {
        resolve();
      });
    });
  }

  getModuleDescription(translationCode) {
    return this.nativeInterface.getModuleDescription(translationCode);
  }

  getRepoNames() {
    return this.nativeInterface.getRepoNames();
  }

  getRepoLanguages(repositoryName) {
    return this.nativeInterface.getRepoLanguages(repositoryName);
  }

  getAllRepoModules(repositoryName) {
    return this.nativeInterface.getAllRepoModules(repositoryName);
  }

  getRepoModulesByLang(repositoryName, language) {
    return this.nativeInterface.getRepoModulesByLang(repositoryName, language);
  }

  getRepoLanguageTranslationCount(repositoryName, language) {
    return this.nativeInterface.getRepoLanguageTranslationCount(repositoryName, language);
  }

  installModule(moduleCode) {
    return new Promise(resolve => {
      this.nativeInterface.installModule(moduleCode, function() {
        resolve();
      });
    });
  }

  uninstallModule(moduleCode) {
    return new Promise(resolve => {
      this.nativeInterface.uninstallModule(moduleCode, function() {
        resolve();
      });
    });
  }

  getBibleText(translationCode) {
    return this.nativeInterface.getBibleText(translationCode);
  }

  getLocalModule(moduleCode) {
    return this.nativeInterface.getLocalModule(moduleCode);
  }
}

module.exports = NodeSwordInterface;

