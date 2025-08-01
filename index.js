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

const path = require('path');
const nodeSwordInterfaceModule = require('./build/Release/node_sword_interface.node');
const { Mutex } = require('async-mutex');

// Create a mutex instance
const searchMutex = new Mutex();

/**
* An object representation of a Bible verse.
* @typedef VerseObject
* @type {Object}
* @property {String} moduleCode - The name/code of the SWORD module
* @property {String} bibleBookShortTitle - The short title of the verses's Bible book
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
 * @property {String} type - The type of the SWORD module (Currently the following types are supported: Biblical Texts, Lexicons / Dictionaries, Commentaries)
 * @property {String} description - The description of the SWORD module
 * @property {String} language - The language code of the SWORD module
 * @property {String} distributionLicense - The distribution license of the SWORD module
 * @property {String} shortCopyright - The short copyright information of the SWORD module
 * @property {String} version - The version of the SWORD module
 * @property {String} lastUpdate - The date of the last version update of the SWORD module
 * @property {String[]} history - The history of the SWORD module. A list of strings where each looks like this: <version>=<version-info>
 * @property {String} category - The category of the SWORD module
 * @property {String} repository - The repository of the SWORD module
 * @property {String} about - Extended description of the SWORD module
 * @property {String} abbreviation - The abbreviation of the SWORD module
 * @property {Number} size - The file size of the SWORD module (in KB)
 * @property {String} location - The filesystem location where the module is stored
 * @property {String} unlockInfo - Information about how to retrieve an unlock key (in case the module is locked)
 * @property {Boolean} inUserDir - Information on whether the module is located in the user's directory
 * @property {Boolean} locked - Information on whether the module is locked
 * @property {Boolean} hasStrongs - Information on whether the module has Strong's numbers
 * @property {Boolean} hasGreekStrongsKeys - Information on whether the module has Strong's based keys (Greek)
 * @property {Boolean} hasHebrewStrongsKeys - Information on whether the module has Strong's based keys (Hebrew)
 * @property {Boolean} hasFootnotes - Information on whether the module has footnotes
 * @property {Boolean} hasHeadings - Information on whether the module has headings
 * @property {Boolean} hasRedLetterWords - Information on whether the module has red letter words
 * @property {Boolean} hasCrossReferences - Information on whether the module has cross references
 * @property {Boolean} isRightToLeft - Information on whether the module has right to left text direction
 */

/**
 * An object representation of a Strongs reference
 * @typedef StrongsReference
 * @type {Object}
 * @property {String} text - The full text of the reference
 * @property {String} key - The reference key
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
* @property {StrongsReference[]} references - The "see also" references of the Strong's entry
*/

/** This is the main class of node-sword-interface and it provides a set of static functions that wrap SWORD library functionality. */
class NodeSwordInterface {
  constructor(customHomeDir=undefined) {
    var localesDir = path.join(__dirname, './locales.d');
    this.nativeInterface = new nodeSwordInterfaceModule.NodeSwordInterface(customHomeDir, localesDir);
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
   * This function works asynchronously and returns a Promise object. The Promise delivers a detailed status object which contains one
   * entry for each of the repositories of the master repo list as well as one result entry.
   *
   * @param {Function} progressCB - Optional callback function that is called on progress events.
   * @return {Promise}
   */
  async updateRepositoryConfig(progressCB=undefined) {
    return new Promise((resolve, reject) => {
      if (progressCB === undefined) {
        progressCB = function(progress) {};
      }

      this.nativeInterface.updateRepositoryConfig(true, progressCB, function(repoUpdateStatus) {
        resolve(repoUpdateStatus);
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
   * Returns the available languages for the modules from a given repository (default: Bible modules).
   * Note that the languages are returned as language codes.
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} moduleType - A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY)
   * @return {String[]} An array of strings with the languages codes for the Bible modules from the given repository.
   */
  getRepoLanguages(repositoryName, moduleType="BIBLE") {
    return this.nativeInterface.getRepoLanguages(repositoryName, moduleType);
  }

  /**
   * Returns all modules for the given repository (default: Bible modules).
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} moduleType - A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY)
   * @return {ModuleObject[]} An array of strings with the module codes for the Bible modules of the given repository.
   */
  getAllRepoModules(repositoryName, moduleType="BIBLE") {
    return this.nativeInterface.getAllRepoModules(repositoryName, moduleType);
  }

  /**
   * Returns all Bible modules for the given repository and language
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} language - The language code that shall be used as a filter.
   * @param {String} moduleType - A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY)
   * @param {Boolean} headersFilter - Whether only modules with Headers shall be returned.
   * @param {Boolean} strongsFilter - Whether only modules with Strong's shall be returned.
   * @param {Boolean} hebrewStrongsKeys - Whether only modules with Hebrew Strong's keys shall be returned (only applies to dictionaries).
   * @param {Boolean} greekStrongsKeys - Whether only modules with Greek Strong's keys shall be returned (only applies to dictionaries).
   * @return {ModuleObject[]} An array of module objects.
   */
  getRepoModulesByLang(repositoryName, language, moduleType="BIBLE", headersFilter=false, strongsFilter=false, hebrewStrongsKeys=false, greekStrongsKeys=false) {
    return this.nativeInterface.getRepoModulesByLang(repositoryName, language, moduleType, headersFilter, strongsFilter, hebrewStrongsKeys, greekStrongsKeys);
  }

  /**
   * Returns all updated modules from all repositories or one specific repository.
   *  
   * @param {String} repositoryName - The name of the repository from which updates shall retrieved. Default: 'all'
   * @param {Boolean} includeBeta - Whether modules from the CrossWire Beta repository should also be included.
   * @returns {ModuleObject[]} An array of module objects.
   */
  getUpdatedRepoModules(repositoryName="all", includeBeta=false) {
    return this.nativeInterface.getUpdatedRepoModules(repositoryName, includeBeta);
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
   * Returns all modules installed locally (default: Bible modules).
   * @param {String} moduleType - A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY)
   * @return {ModuleObject[]} An array of ModuleObjects which represents the locally installed Bible modules.
   */
  getAllLocalModules(moduleType="BIBLE") {
    return this.nativeInterface.getAllLocalModules(moduleType);
  }

  /**
   * Returns the number of modules for a given repository (default: Bible modules).
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} moduleType - A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY)
   * @return {Number} The number of Bible modules for the given repository and module type.
   */
  getRepoModuleCount(repositoryName, moduleType="BIBLE") {
    return this.nativeInterface.getRepoModuleCount(repositoryName, moduleType);
  }

  /**
   * Returns the number of modules for a given repository and language (default: Bible modules).
   *
   * @param {String} repositoryName - The name of the given repository.
   * @param {String} language - The language code that shall be used as a filter.
   * @param {String} moduleType - A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY)
   * @return {Number} The number of Bible modules for the given repository, language and module type.
   */
  getRepoLanguageModuleCount(repositoryName, language, moduleType="BIBLE") {
    return this.nativeInterface.getRepoLanguageModuleCount(repositoryName, language, moduleType);
  }

  /**
   * Installs a module. The repository is automatically determined. The module is downloaded
   * from the corresponding repository and then installed in the local SWORD directory.
   * This operation may take some time depending on the available bandwidth and geographical
   * distance to the SWORD repository server.
   *
   * This function works asynchronously and returns a Promise object.
   * 
   * If the installation fails, the Promise will be rejected with the following status codes (based on SWORD):
   * -1: General installation issue
   * -9: Installation cancelled by user or internet connection suddenly interrupted
   *
   * @param {String} moduleCode - The module code of the SWORD module that shall be installed.
   * @param {Function} progressCB - Callback function that is called on progress events.
   * @return {Promise}
   */
  async installModule(moduleCode, progressCB=undefined) {
    if (progressCB === undefined) {
      progressCB = function(progress) {};
    }

    return new Promise((resolve, reject) => {
      this.nativeInterface.installModule(moduleCode, progressCB, function(result) {
        if (result == 0) {
          resolve();
        } else {
          reject(result);
        }
      });
    });
  }

  /**
   * Cancels an ongoing module installation.
   */
  cancelInstallation() {
    return this.nativeInterface.cancelInstallation();
  }

  /**
   * Uninstalls a module.
   *
   * This function works asynchronously and returns a Promise object.
   *
   * @param {String} moduleCode - The module code of the SWORD module that shall be uninstalled.
   * @return {Promise}
   */
  async uninstallModule(moduleCode) {
    return new Promise((resolve, reject) => {
      this.nativeInterface.uninstallModule(moduleCode, function(uninstallSuccessful) {
        if (uninstallSuccessful) {
          resolve();
        } else {
          reject();
        }
      });
    });
  }

  /**
   * Refresh the local module database. This function is purely for testing.
   * It will usually be called after changing the SWORD module database outside of the actual application.
   */
  refreshLocalModules() {
    this.nativeInterface.refreshLocalModules();
  }

  /**
   * Persistently saves the unlock key of the corresponding module in the module's .conf file
   * (in ~/.sword/mods.d/<modname>.conf)
   * 
   * @param {String} moduleCode - The module code of the SWORD module. 
   * @param {String} key - The unlock key.
   */
  saveModuleUnlockKey(moduleCode, key) {
    return this.nativeInterface.saveModuleUnlockKey(moduleCode, key);
  }

  /**
   * Checks whether the module is readable.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {Boolean}
   */
  isModuleReadable(moduleCode) {
    return this.nativeInterface.isModuleReadable(moduleCode);
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
   * This influences the output for getChapterText, getBookText and getBibleText.
   */
  enableMarkup() {
    return this.nativeInterface.enableMarkup();
  }

  /**
   * Disable available markup (like Strongs, foot notes, etc.)
   * This influences the output for getChapterText, getBookText and getBibleText.
   */
  disableMarkup() {
    return this.nativeInterface.disableMarkup();
  }

  /**
   * Enables rendering of Strongs elements with non-breaking spaces.
   */
  enableStrongsWithNbsp() {
    return this.nativeInterface.enableStrongsWithNbsp();
  }

  /**
   * Returns the raw text of an entry for the given module and key.
   * If no entry exists for the given key the return value is undefined.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} key - The key of the entry.
   * @return {String}
   */
  getRawModuleEntry(moduleCode, key) {
    return this.nativeInterface.getRawModuleEntry(moduleCode, key);
  }

  /**
   * Returns the text of an entry for the given module and key.
   * If no entry exists for the given key the return value is a verse with empty content.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} key - The key of the entry.
   * @return {VerseObject}
   */
  getReferenceText(moduleCode, key) {
    return this.nativeInterface.getReferenceText(moduleCode, key);
  }

  /**
   * Returns the text of a chapter for the given module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} bookCode - The book code of the SWORD module.
   * @param {Number} chapter - The chapter that shall be returned.
   * @return {VerseObject[]} An array of verse objects.
   */
  getChapterText(moduleCode, bookCode, chapter) {
    return this.nativeInterface.getChapterText(moduleCode, bookCode, chapter);
  }

  /**
   * Returns the text of a book for the given module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} bookCode - The book code of the SWORD module.
   * @param {Number} startVerseNr - The start verse number (absolute) where we start reading (Optional)
   * @param {Number} verseCount - The number of verses that shall be returned (Optional)
   * @return {VerseObject[]} An array of verse objects.
   */
  getBookText(moduleCode, bookCode, startVerseNr=-1, verseCount=-1) {
    return this.nativeInterface.getBookText(moduleCode, bookCode, startVerseNr, verseCount);
  }

  /**
   * Returns an array of verses based on the given array of verse references
   * 
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {Array} references - A list of OSIS references. (like ['Gal.5.1', '1Cor.2.4'])
   * @return {VerseObject[]} An array of verse objects.
   */
  getVersesFromReferences(moduleCode, references) {
    return this.nativeInterface.getVersesFromReferences(moduleCode, references);
  }

  /**
   * Returns an array of individual verse references based on an OSIS reference range expression.
   * 
   * @param {String} referenceRange - An OSIS reference range expression. (like 'Gal.1.15-Gal.1.16')
   * @return {String[]} An array of OSIS references.
   */
  getReferencesFromReferenceRange(referenceRange) {
    return this.nativeInterface.getReferencesFromReferenceRange(referenceRange);
  }

  /**
   * Returns the list of books available in the given module. By default the book codes will be in OSIS format.
   * 
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {String[]} An array of book codes.
   */
  getBookList(moduleCode) {
    return this.nativeInterface.getBookList(moduleCode);
  }

  /**
   * Returns the list of headers available in the given book. The headers are returned as an array of VerseObjects.
   * 
   * @param {String} moduleCode 
   * @param {String} bookCode 
   * @param {Number} startVerseNumber
   * @param {Number} verseCount
   * 
   * @return {VerseObject[]}
   */
  getBookHeaderList(moduleCode, bookCode, startVerseNumber=-1, verseCount=-1) {
    const bookTextJson = this.nativeInterface.getBookText(moduleCode, bookCode, startVerseNumber, verseCount);

    let bookTextHtml = "<div>";
    bookTextJson.forEach((verse) => { bookTextHtml += verse.content; });
    bookTextHtml += "</div>";

    const HTMLParser = require('node-html-parser');
    const root = HTMLParser.parse(bookTextHtml);
    const rawSectionHeaders = root.querySelectorAll('.sword-section-title');
    let sectionHeaders = [];

    rawSectionHeaders.forEach((header) => {
      let newSectionHeader = {};

      newSectionHeader['moduleCode'] = moduleCode;
      newSectionHeader['bibleBookShortTitle'] = bookCode;
      newSectionHeader['type'] = header._attrs.type;
      newSectionHeader['subType'] = header._attrs.subtype;
      newSectionHeader['chapter'] = header._attrs.chapter;
      newSectionHeader['verseNr'] = header._attrs.verse;

      let content = "";

      // You may expect that a header only has one child node (the text). But there are modules like the NET Bible
      // where the header actually contains several child nodes and some of them are Strongs elements.
      // Therefore, we have to go through individually and also differentiate between text nodes and other types of nodes.
      for (let i = 0; i < header.childNodes.length; i++) {
        let currentNode = header.childNodes[i];

        if (currentNode.nodeType == HTMLParser.NodeType.TEXT_NODE) {
          content += currentNode._rawText;
        } else {
          content += currentNode.firstChild._rawText;
        }
      }

      newSectionHeader['content'] = content;

      sectionHeaders.push(newSectionHeader);
    });

    return sectionHeaders;
  }

  /**
   * Returns the chapter count of the given book.
   * 
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} bookCode - The book code of the SWORD module.
   */
  getBookChapterCount(moduleCode, bookCode) {
    return this.nativeInterface.getBookChapterCount(moduleCode, bookCode);
  }

  /**
   * Returns the number of verses in the given chapter.
   * 
   * @param {String} moduleCode - The module code of the SWORD module. 
   * @param {String} bookCode - The book code of the SWORD module.
   * @param {Number} chapter - The chapter of the book.
   */
  getChapterVerseCount(moduleCode, bookCode, chapter) {
    return this.nativeInterface.getChapterVerseCount(moduleCode, bookCode, chapter);
  }

  /**
   * Returns an array with the chapter verse counts of all chapters of a given book.
   * 
   * @param {String} moduleCode 
   * @param {String} bookCode 
   * @returns {Number} chapterVerseCounts
   */
  getAllChapterVerseCounts(moduleCode, bookCode) {
    let chapterVerseCounts = [];

    let bookChapterCount = this.nativeInterface.getBookChapterCount(moduleCode, bookCode);
    for (let i = 0; i < bookChapterCount; i++) {
      let currentChapterVerseCount = this.nativeInterface.getChapterVerseCount(moduleCode, bookCode, i);
      chapterVerseCounts.push(currentChapterVerseCount);
    }

    return chapterVerseCounts;
  }

  /**
   * Returns the number verses in the given book.
   * 
   * @param {String} moduleCode - The module code of the SWORD module. 
   * @param {String} bookCode - The book code of the SWORD module.
   * @returns {Number} The verse count of the book.
   */
  getBookVerseCount(moduleCode, bookCode) {
    const bookChapterCount = this.nativeInterface.getBookChapterCount(moduleCode, bookCode);
    let verseCount = 0;

    for (let i = 1; i <= bookChapterCount; i++) {
      const chapterVerseCount = this.nativeInterface.getChapterVerseCount(moduleCode, bookCode, i);
      verseCount += chapterVerseCount;
    }

    return verseCount;
  }

  /**
   * Returns the Bible text of a module.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @return {VerseObject[]} An array of verse objects.
   */
  getBibleText(moduleCode) {
    return this.nativeInterface.getBibleText(moduleCode);
  }

  /**
   * Returns the introduction of the given book.
   * 
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} bookCode - The book code of the SWORD module.
   * @return {String}
   */
  getBookIntroduction(moduleCode, bookCode) {
    return this.nativeInterface.getBookIntroduction(moduleCode, bookCode);
  }

  /**
   * Checks whether a module has a certain book.
   * 
   * @param {String} moduleCode 
   * @param {String} bookCode 
   * @return {Boolean}
   */
  moduleHasBook(moduleCode, bookCode) {
    return this.nativeInterface.moduleHasBook(moduleCode, bookCode);
  }

  /**
   * Returns the keys of a dictionary module.
   * 
   * @param {String} moduleCode 
   * @return {String[]}
   */
  getDictModuleKeys(moduleCode) {
    return this.nativeInterface.getDictModuleKeys(moduleCode);
  }

  /**
   * Returns the results of a module search.
   *
   * @param {String} moduleCode - The module code of the SWORD module.
   * @param {String} searchTerm - The term to search for.
   * @param {Function} progressCB - Optional callback function that is called on progress events.
   * @param {String} searchType - Options: phrase, multiWord, strongsNumber
   * @param {String} searchScope - Options: BIBLE, OT, NT
   * @param {Boolean} isCaseSensitive - Whether the search is case sensitive
   * @param {Boolean} useExtendedVerseBoundaries - Whether the search should use extended verse boundaries (Two verses instead of one) in case of a multi word search.
   * @param {Boolean} filterOnWordBoundaries - Whether to filter results based on word boundaries.
   * @return {Promise}
   */
  async getModuleSearchResults(moduleCode,
                               searchTerm,
                               progressCB = undefined,
                               searchType = "phrase",
                               searchScope = "BIBLE",
                               isCaseSensitive = false,
                               useExtendedVerseBoundaries = false,
                               filterOnWordBoundaries = false) {

    if (progressCB === undefined) {
      progressCB = function(progress) {};
    }

    if (searchMutex.isLocked()) {
      throw new Error("Module search in progress. Wait until it is finished.");
    }

    const release = await searchMutex.acquire();

    try {
      return new Promise((resolve, reject) => {
        this.nativeInterface.getModuleSearchResults(moduleCode,
                                                    searchTerm,
                                                    searchType,
                                                    searchScope,
                                                    isCaseSensitive,
                                                    useExtendedVerseBoundaries,
                                                    filterOnWordBoundaries,
                                                    progressCB,
                                                    function(searchResults) {
          release();
          resolve(searchResults);
        });
      });
    } catch (error) {
      release();
      throw error;
    }
  }

  /**
   * Terminates the currently ongoing module search.
   */
  terminateModuleSearch() {
    return this.nativeInterface.terminateModuleSearch();
  }

  /**
   * Checks whether Hebrew Strong's definitions are available.
   * 
   * @return {Boolean}
   */
  hebrewStrongsAvailable() {
    const strongsDict = this.nativeInterface.getLocalModule("StrongsHebrew");
    return !!strongsDict;
  }

  /**
   * Checks whether Greek Strong's definitions are available.
   * 
   * @return {Boolean}
   */
  greekStrongsAvailable() {
    const strongsDict = this.nativeInterface.getLocalModule("StrongsGreek");
    return !!strongsDict;
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
   * Returns an object representation of a locally installed SWORD module. If the requested `moduleCode` is not available
   * `undefined` will be returned.
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
    return this.nativeInterface.getSwordTranslation(originalString, localeCode);
  }

  /**
   * Uses the Sword LocaleMgr to translate a book abbreviation.
   * 
   * @param {String} moduleName
   * @param {String} bookCode
   * @param {String} localeCode
   */
  getBookAbbreviation(moduleName, bookCode, localeCode) {
    return this.nativeInterface.getBookAbbreviation(moduleName, bookCode, localeCode);
  }

  /**
   * Returns the version of the SWORD library
   * @return {String} SWORD library version.
   */  
  getSwordVersion() {
    return this.nativeInterface.getSwordVersion();
  }

  /**
   * Returns the platform-specific path where SWORD accesses and stores its modules.
   * @returns {String} Platform-specific SWORD path.
   */
  getSwordPath() {
    return this.nativeInterface.getSwordPath();
  }
}

module.exports = NodeSwordInterface;

