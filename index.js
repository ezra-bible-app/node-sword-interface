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

const path = require('path');
const nodeSwordInterfaceModule = require('./build/Release/node_sword_interface.node');

/**
* An object representation of a bible verse.
* @typedef VerseObject
* @type {Object}
* @property {String} moduleCode - The name/code of the SWORD module
* @property {String} bibleBookShortTitle - The short title of the verses's bible book
* @property {String} chapter - The chapter number
* @property {String} verseNr - The verse number
* @property {Number} absoluteVerseNr - The absolute number of the verse within the book (independent of chapters, starting from 1)
* @property {String} content - The verse content
*/

/**
 * An object representation of a SWORD module.
 * @typedef ModuleObject
 * @type {Object}
 * @property {String} name - The name/code of the SWORD module
 * @property {String} description - The description of the SWORD module
 * @property {String} language - The language code of the SWORD module
 * @property {String} version - The version of the SWORD module
 * @property {String} about - Extended description of the SWORD module
 * @property {String} abbreviation - The abbreviation of the SWORD module
 * @property {Number} size - The file size of the SWORD module (in KB)
 * @property {String} location - The filesystem location where the module is stored
 * @property {Boolean} inUserDir - Information on whether the module is located in the user's directory
 * @property {Boolean} locked - Information on whether the module is locked
 * @property {Boolean} hasStrongs - Information on whether the module has Strong's numbers
 * @property {Boolean} hasFootnotes - Information on whether the module has footnotes
 * @property {Boolean} hasHeadings - Information on whether the module has headings
 * @property {Boolean} hasRedLetterWords - Information on whether the module has red letter words
 * @property {Boolean} hasCrossReferences - Information on whether the module has cross references
 * @property {Boolean} isRightToLeft - Information on whether the module has right to left text direction
 */

/**
* An object representation of a Strong's entry.
* @typedef StrongsEntry
* @type {Object}
* @property {String} rawEntry - The full raw entry from the SWORD Strong's module
* @property {String} key - The key of the Strong's entry
* @property {String} transcription - The transcription of the Strong's entry
* @property {String} phoneticTranscription - The phonetic transcription of the Strong's entry
* @property {String} definition - The Strong's definition
* @property {String[]} references - The "see also" references of the Strong's entry
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
   * Returns an object representation of a SWORD module from a repository.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {ModuleObject}
   */
  getRepoModule(moduleCode) {
    return this.nativeInterface.getRepoModule(moduleCode);
  }

  /**
   * Returns all bible modules installed locally.
   *
   * @return {String[]} An array of strings with the module codes for the locally installed bible modules.
   */
  getAllLocalModules() {
    return this.nativeInterface.getAllLocalModules();
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
   * Returns the results of a module search.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} searchTerm - The term to search for.
   * @param {Boolean} isPhrase - Whether it is a phrase search (otherwise multi-word!)
   * @param {Boolean} isCaseSensitive - Whether the search is case sensitive
   * @return {Promise}
   */
  getModuleSearchResults(moduleCode, searchTerm, isPhrase=false, isCaseSensitive=false) {
    return new Promise(resolve => {
      this.nativeInterface.getModuleSearchResults(moduleCode, searchTerm, isPhrase, isCaseSensitive, function(searchResults) {
        resolve(searchResults);
      });
    });
  }

  /**
   * Checks whether Hebrew Strong's definitions are available
   * 
   * @return {Boolean}
   */
  hebrewStrongsAvailable() {
    try {
      this.nativeInterface.getLocalModule("StrongsHebrew");
      return true;
    } catch (e) {
      return false;
    }
  }

  /**
   * Checks whether Greek Strong's definitions are available
   * 
   * @return {Boolean}
   */
  greekStrongsAvailable() {
    try {
      this.nativeInterface.getLocalModule("StrongsGreek");
      return true;
    } catch (e) {
      return false;
    }
  }

  /**
   * Checks whether Strongs definitions are available
   * 
   * @return {Boolean}
   */
  strongsAvailable() {
    return this.hebrewStrongsAvailable() && this.greekStrongsAvailable();
  }

  /**
   * Returns the Strong's entry for a given key.
   *
   * @param {String} strongsKey - The Strong's key for the requested entry.
   * @return {StrongsEntry} A StrongsEntry object.
   */
  getStrongsEntry(strongsKey) {
    return this.nativeInterface.getStrongsEntry(strongsKey);
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

  /**
   * Checks whether the module resides in the user directory.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {Boolean}
   */
  isModuleInUserDir(moduleCode) {
    return this.nativeInterface.isModuleInUserDir(moduleCode);
  }

 /**
   * Checks whether the module is available in any repository.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {Boolean}
   */
  isModuleAvailableInRepo(moduleCode) {
    return this.nativeInterface.isModuleAvailableInRepo(moduleCode);
  }

  /**
   * Uses the Sword LocaleMgr to translate a string. This can be used to translate book names for example.
   * 
   * @param {String} originalString 
   * @param {String} localeCode 
   */
  getSwordTranslation(originalString, localeCode) {
    var localesDir = path.join(__dirname, './locales.d');
    return this.nativeInterface.getSwordTranslation(localesDir, originalString, localeCode);
  }

  /**
   * Returns the version of the SWORD library
   * @return {String} SWORD library version.
   */  
  getSwordVersion() {
    return this.nativeInterface.getSwordVersion();
  }
}

module.exports = NodeSwordInterface;

