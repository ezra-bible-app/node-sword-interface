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

/**
* An object representation of a bible verse.
* @typedef VerseObject
* @type {Object}
* @property {String} bibleBookShortTitle - The short title of the verses's bible book.
* @property {String} chapter - The chapter number.
* @property {String} verseNr - The verse number.
* @property {Number} absoluteVerseNr - The absolute number of the verse within the book (independent of chapters, starting from 1)
* @property {String} content - The verse content.
*/

/**
 * An object representation of a SWORD module.
 * @typedef ModuleObject
 * @type {Object}
 * @property {String} name - The name/code of the SWORD module.
 * @property {String} description - The description of the SWORD module.
 * @property {String} language - The language code of the SWORD module.
 * @property {String} version - The version of the SWORD module.
 * @property {String} about - Extended description of the SWORD module.
 * @property {String} abbreviation - The abbreviation of the SWORD module.
 * @property {Number} size - The file size of the SWORD module (in KB).
 * @property {Boolean} locked - Information on whether the module is locked
 * @property {Boolean} hasStrongs - Information on whether the module has Strong's numbers
 * @property {Boolean} hasFootnotes - Information on whether the module has footnotes
 * @property {Boolean} hasHeadings - Information on whether the module has headings
 * @property {Boolean} hasRedLetterWords - Information on whether the module has red letter words
 * @property {Boolean} hasCrossReferences - Information on whether the module has cross references
 */

/** This is the main class of node-sword-interface and it provides a set of static functions that wrap SWORD library functionality. */
class NodeSwordInterface {
  constructor() {
    this.nativeInterface = new nodeSwordInterfaceModule.NodeSwordInterface();
  }

  /**
   * Checks whether a repository configuration is already existing locally.
   * @return {Boolean}
   */
  repositoryConfigExisting() {
    return this.nativeInterface.repositoryConfigExisting();
  }

  /**
   * Creates or updates the repository configuration and stores the information persistently. The repository configuration contains information
   * about all SWORD repositories and the available modules. After this operation the local cached repository configuration will
   * contain the latest master repository list and updated lists of SWORD modules available from each repository.
   *
   * This function must be called initially before using any other repository-related functions. Once the repository configuration has been
   * initialized, the use of this function is optional. However, the cached repository configuration may not contain the latest information
   * if this function is not called.
   *
   * This function works asynchronously and returns a Promise object.
   *
   * @return {Promise}
   */
  updateRepositoryConfig() {
    return new Promise(resolve => {
      this.nativeInterface.refreshRemoteSources(true, function() {
        resolve();
      });
    });
  }

  /**
   * Returns the names of all available SWORD repositories.
   *
   * @return {String[]} An array of strings with the names of the available SWORD repositories.
   */
  getRepoNames() {
    return this.nativeInterface.getRepoNames();
  }

  /**
   * Returns the available languages for the bible modules from a given repository.
   * Note that the languages are returned as language codes.
   *
   * @param {String} repositoryName - The name of the given repository.
   * @return {String[]} An array of strings with the languages codes for the bible modules from the given repository.
   */
  getRepoLanguages(repositoryName) {
    return this.nativeInterface.getRepoLanguages(repositoryName);
  }

  /**
   * Returns all bible modules for the given repository.
   *
   * @param {String} repositoryName - The name of the given repository.
   * @return {String[]} An array of strings with the module codes for the bible modules of the given repository.
   */
  getAllRepoModules(repositoryName) {
    return this.nativeInterface.getAllRepoModules(repositoryName);
  }

  /**
   * Returns all bible modules for the given repository and language
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} language - The language code that shall be used as a filter.
   * @return {String[]} An array of strings with the module codes for the bible modules of the given repository and language.
   */
  getRepoModulesByLang(repositoryName, language) {
    return this.nativeInterface.getRepoModulesByLang(repositoryName, language);
  }

  /**
   * Returns the number of bible modules for a given repository and language.
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} language - The language code that shall be used as a filter.
   * @return {Number} The number of bible modules for the given repository and language.
   */
  getRepoLanguageTranslationCount(repositoryName, language) {
    return this.nativeInterface.getRepoLanguageTranslationCount(repositoryName, language);
  }

  /**
   * Installs a module. The repository is automatically determined. The module is downloaded
   * from the corresponding repository and then installed in the local SWORD directory.
   * This operation may take some time depending on the available bandwidth and geographical
   * distance to the SWORD repository server.
   *
   * This function works asynchronously and returns a Promise object.
   *
   * @param {String} moduleCode - The module code of the SWORD module that shall be installed.
   * @return {Promise}
   */
  installModule(moduleCode) {
    return new Promise(resolve => {
      this.nativeInterface.installModule(moduleCode, function() {
        resolve();
      });
    });
  }

  /**
   * Uninstalls a module.
   *
   * This function works asynchronously and returns a Promise object.
   *
   * @param {String} moduleCode - The module code of the SWORD module that shall be uninstalled.
   * @return {Promise}
   */
  uninstallModule(moduleCode) {
    return new Promise(resolve => {
      this.nativeInterface.uninstallModule(moduleCode, function() {
        resolve();
      });
    });
  }

  /**
   * Returns the description of a module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {String} The description of the respective module.
   */
  getModuleDescription(moduleCode) {
    return this.nativeInterface.getModuleDescription(moduleCode);
  }

  /**
   * Enables available markup (like Strongs, foot notes, etc.)
   * This influences the output for getBookText and getBibleText.
   */
  enableMarkup() {
    return this.nativeInterface.enableMarkup();
  }

  /**
   * Returns the text of a book for the given module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} bookCode - The book code of the SWORD module.
   * @return {VerseObject[]} An array of verse objects.
   */
  getBookText(moduleCode, bookCode) {
    return this.nativeInterface.getBookText(moduleCode, bookCode);
  }

  /**
   * Returns the bible text of a module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {VerseObject[]} An array of verse objects.
   */
  getBibleText(moduleCode) {
    return this.nativeInterface.getBibleText(moduleCode);
  }

  /**
   * Returns an object representation of a locally installed SWORD module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {ModuleObject}
   */
  getLocalModule(moduleCode) {
    return this.nativeInterface.getLocalModule(moduleCode);
  }
}

module.exports = NodeSwordInterface;

