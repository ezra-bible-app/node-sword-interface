## Classes

<dl>
<dt><a href="#NodeSwordInterface">NodeSwordInterface</a></dt>
<dd><p>This is the main class of node-sword-interface and it provides a set of static functions that wrap SWORD library functionality.</p>
</dd>
</dl>

## Typedefs

<dl>
<dt><a href="#VerseObject">VerseObject</a> : <code>Object</code></dt>
<dd><p>An object representation of a Bible verse.</p>
</dd>
<dt><a href="#ModuleObject">ModuleObject</a> : <code>Object</code></dt>
<dd><p>An object representation of a SWORD module.</p>
</dd>
<dt><a href="#StrongsReference">StrongsReference</a> : <code>Object</code></dt>
<dd><p>An object representation of a Strongs reference</p>
</dd>
<dt><a href="#StrongsEntry">StrongsEntry</a> : <code>Object</code></dt>
<dd><p>An object representation of a Strong&#39;s entry.</p>
</dd>
</dl>

<a name="NodeSwordInterface"></a>

## NodeSwordInterface
This is the main class of node-sword-interface and it provides a set of static functions that wrap SWORD library functionality.

**Kind**: global class  

* [NodeSwordInterface](#NodeSwordInterface)
    * [new NodeSwordInterface(customHomeDir, localesBasePath, timeoutMillis)](#new_NodeSwordInterface_new)
    * [.repositoryConfigExisting()](#NodeSwordInterface+repositoryConfigExisting) ⇒ <code>Boolean</code>
    * [.updateRepositoryConfig(progressCB)](#NodeSwordInterface+updateRepositoryConfig) ⇒ <code>Promise</code>
    * [.getRepoNames()](#NodeSwordInterface+getRepoNames) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getRepoLanguages(repositoryName, moduleType)](#NodeSwordInterface+getRepoLanguages) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getAllRepoModules(repositoryName, moduleType)](#NodeSwordInterface+getAllRepoModules) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
    * [.getRepoModulesByLang(repositoryName, language, moduleType, headersFilter, strongsFilter, hebrewStrongsKeys, greekStrongsKeys)](#NodeSwordInterface+getRepoModulesByLang) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
    * [.getUpdatedRepoModules(repositoryName, includeBeta)](#NodeSwordInterface+getUpdatedRepoModules) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
    * [.getRepoModule(repositoryName, moduleCode)](#NodeSwordInterface+getRepoModule) ⇒ [<code>ModuleObject</code>](#ModuleObject)
    * [.getAllLocalModules(moduleType)](#NodeSwordInterface+getAllLocalModules) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
    * [.getRepoModuleCount(repositoryName, moduleType)](#NodeSwordInterface+getRepoModuleCount) ⇒ <code>Number</code>
    * [.getRepoLanguageModuleCount(repositoryName, language, moduleType)](#NodeSwordInterface+getRepoLanguageModuleCount) ⇒ <code>Number</code>
    * [.installModule(repositoryName, moduleCode, progressCB)](#NodeSwordInterface+installModule) ⇒ <code>Promise</code>
    * [.cancelInstallation()](#NodeSwordInterface+cancelInstallation)
    * [.uninstallModule(repositoryName, moduleCode)](#NodeSwordInterface+uninstallModule) ⇒ <code>Promise</code>
    * [.refreshLocalModules()](#NodeSwordInterface+refreshLocalModules)
    * [.saveModuleUnlockKey(moduleCode, key)](#NodeSwordInterface+saveModuleUnlockKey)
    * [.isModuleReadable(moduleCode)](#NodeSwordInterface+isModuleReadable) ⇒ <code>Boolean</code>
    * [.getModuleDescription(repositoryName, moduleCode)](#NodeSwordInterface+getModuleDescription) ⇒ <code>String</code>
    * [.enableMarkup()](#NodeSwordInterface+enableMarkup)
    * [.disableMarkup()](#NodeSwordInterface+disableMarkup)
    * [.enableStrongsWithNbsp()](#NodeSwordInterface+enableStrongsWithNbsp)
    * [.getRawModuleEntry(moduleCode, key)](#NodeSwordInterface+getRawModuleEntry) ⇒ <code>String</code>
    * [.getReferenceText(moduleCode, key)](#NodeSwordInterface+getReferenceText) ⇒ [<code>VerseObject</code>](#VerseObject)
    * [.getChapterText(moduleCode, bookCode, chapter)](#NodeSwordInterface+getChapterText) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getBookText(moduleCode, bookCode, startVerseNr, verseCount)](#NodeSwordInterface+getBookText) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getVersesFromReferences(moduleCode, references)](#NodeSwordInterface+getVersesFromReferences) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getReferencesFromReferenceRange(referenceRange)](#NodeSwordInterface+getReferencesFromReferenceRange) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getBookList(moduleCode)](#NodeSwordInterface+getBookList) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getBookHeaderList(moduleCode, bookCode, startVerseNumber, verseCount)](#NodeSwordInterface+getBookHeaderList) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getBookChapterCount(moduleCode, bookCode)](#NodeSwordInterface+getBookChapterCount)
    * [.getChapterVerseCount(moduleCode, bookCode, chapter)](#NodeSwordInterface+getChapterVerseCount)
    * [.getAllChapterVerseCounts(moduleCode, bookCode)](#NodeSwordInterface+getAllChapterVerseCounts) ⇒ <code>Number</code>
    * [.getBookVerseCount(moduleCode, bookCode)](#NodeSwordInterface+getBookVerseCount) ⇒ <code>Number</code>
    * [.getBibleText(moduleCode)](#NodeSwordInterface+getBibleText) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getBookIntroduction(moduleCode, bookCode)](#NodeSwordInterface+getBookIntroduction) ⇒ <code>String</code>
    * [.moduleHasBook(moduleCode, bookCode)](#NodeSwordInterface+moduleHasBook) ⇒ <code>Boolean</code>
    * [.getDictModuleKeys(moduleCode)](#NodeSwordInterface+getDictModuleKeys) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getModuleSearchResults(moduleCode, searchTerm, progressCB, searchType, searchScope, isCaseSensitive, useExtendedVerseBoundaries, filterOnWordBoundaries)](#NodeSwordInterface+getModuleSearchResults) ⇒ <code>Promise</code>
    * [.terminateModuleSearch()](#NodeSwordInterface+terminateModuleSearch)
    * [.hebrewStrongsAvailable()](#NodeSwordInterface+hebrewStrongsAvailable) ⇒ <code>Boolean</code>
    * [.greekStrongsAvailable()](#NodeSwordInterface+greekStrongsAvailable) ⇒ <code>Boolean</code>
    * [.strongsAvailable()](#NodeSwordInterface+strongsAvailable) ⇒ <code>Boolean</code>
    * [.getStrongsEntry(strongsKey)](#NodeSwordInterface+getStrongsEntry) ⇒ [<code>StrongsEntry</code>](#StrongsEntry)
    * [.getLocalModule(moduleCode)](#NodeSwordInterface+getLocalModule) ⇒ [<code>ModuleObject</code>](#ModuleObject)
    * [.isModuleInUserDir(moduleCode)](#NodeSwordInterface+isModuleInUserDir) ⇒ <code>Boolean</code>
    * [.isModuleAvailableInRepo(repositoryName, moduleCode)](#NodeSwordInterface+isModuleAvailableInRepo) ⇒ <code>Boolean</code>
    * [.getSwordTranslation(originalString, localeCode)](#NodeSwordInterface+getSwordTranslation)
    * [.getBookAbbreviation(moduleName, bookCode, localeCode)](#NodeSwordInterface+getBookAbbreviation)
    * [.unTarGZ(filePath, destPath)](#NodeSwordInterface+unTarGZ) ⇒ <code>Boolean</code>
    * [.unZip(filePath, destPath)](#NodeSwordInterface+unZip) ⇒ <code>Boolean</code>
    * [.getSwordVersion()](#NodeSwordInterface+getSwordVersion) ⇒ <code>String</code>
    * [.getSwordPath()](#NodeSwordInterface+getSwordPath) ⇒ <code>String</code>

<a name="new_NodeSwordInterface_new"></a>

### new NodeSwordInterface(customHomeDir, localesBasePath, timeoutMillis)
Creates an instance of NodeSwordInterface.


| Param | Type | Default | Description |
| --- | --- | --- | --- |
| customHomeDir | <code>String</code> |  | Optional custom home directory for SWORD data. |
| localesBasePath | <code>String</code> |  | Optional base path for locales (default: __dirname). |
| timeoutMillis | <code>Number</code> | <code>20000</code> | Optional timeout in milliseconds for repository operations (default: 20000). |

<a name="NodeSwordInterface+repositoryConfigExisting"></a>

### nodeSwordInterface.repositoryConfigExisting() ⇒ <code>Boolean</code>
Checks whether a repository configuration is already existing locally.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+updateRepositoryConfig"></a>

### nodeSwordInterface.updateRepositoryConfig(progressCB) ⇒ <code>Promise</code>
Creates or updates the repository configuration and stores the information persistently. The repository configuration contains information
about all SWORD repositories and the available modules. After this operation the local cached repository configuration will
contain the latest master repository list and updated lists of SWORD modules available from each repository.

This function must be called initially before using any other repository-related functions. Once the repository configuration has been
initialized, the use of this function is optional. However, the cached repository configuration may not contain the latest information
if this function is not called.

This function works asynchronously and returns a Promise object. The Promise delivers a detailed status object which contains one
entry for each of the repositories of the master repo list as well as one result entry.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| progressCB | <code>function</code> | Optional callback function that is called on progress events. |

<a name="NodeSwordInterface+getRepoNames"></a>

### nodeSwordInterface.getRepoNames() ⇒ <code>Array.&lt;String&gt;</code>
Returns the names of all available SWORD repositories.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of strings with the names of the available SWORD repositories.  
<a name="NodeSwordInterface+getRepoLanguages"></a>

### nodeSwordInterface.getRepoLanguages(repositoryName, moduleType) ⇒ <code>Array.&lt;String&gt;</code>
Returns the available languages for the modules from a given repository (default: Bible modules).
Note that the languages are returned as language codes.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of strings with the languages codes for the Bible modules from the given repository.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| repositoryName | <code>String</code> |  | The name of the given repository. |
| moduleType | <code>String</code> | <code>BIBLE</code> | A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY) |

<a name="NodeSwordInterface+getAllRepoModules"></a>

### nodeSwordInterface.getAllRepoModules(repositoryName, moduleType) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
Returns all modules for the given repository (default: Bible modules).

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject) - An array of strings with the module codes for the Bible modules of the given repository.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| repositoryName | <code>String</code> |  | The name of the given repository. |
| moduleType | <code>String</code> | <code>BIBLE</code> | A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY) |

<a name="NodeSwordInterface+getRepoModulesByLang"></a>

### nodeSwordInterface.getRepoModulesByLang(repositoryName, language, moduleType, headersFilter, strongsFilter, hebrewStrongsKeys, greekStrongsKeys) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
Returns all Bible modules for the given repository and language

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject) - An array of module objects.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| repositoryName | <code>String</code> |  | The name of the given repository. |
| language | <code>String</code> |  | The language code that shall be used as a filter. |
| moduleType | <code>String</code> | <code>BIBLE</code> | A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY) |
| headersFilter | <code>Boolean</code> | <code>false</code> | Whether only modules with Headers shall be returned. |
| strongsFilter | <code>Boolean</code> | <code>false</code> | Whether only modules with Strong's shall be returned. |
| hebrewStrongsKeys | <code>Boolean</code> | <code>false</code> | Whether only modules with Hebrew Strong's keys shall be returned (only applies to dictionaries). |
| greekStrongsKeys | <code>Boolean</code> | <code>false</code> | Whether only modules with Greek Strong's keys shall be returned (only applies to dictionaries). |

<a name="NodeSwordInterface+getUpdatedRepoModules"></a>

### nodeSwordInterface.getUpdatedRepoModules(repositoryName, includeBeta) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
Returns all updated modules from all repositories or one specific repository.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject) - An array of module objects.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| repositoryName | <code>String</code> | <code>all</code> | The name of the repository from which updates shall retrieved. Default: 'all' |
| includeBeta | <code>Boolean</code> | <code>false</code> | Whether modules from the CrossWire Beta repository should also be included. |

<a name="NodeSwordInterface+getRepoModule"></a>

### nodeSwordInterface.getRepoModule(repositoryName, moduleCode) ⇒ [<code>ModuleObject</code>](#ModuleObject)
Returns an object representation of a SWORD module from a repository.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the repository to search in. If undefined/null, searches all repositories. |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getAllLocalModules"></a>

### nodeSwordInterface.getAllLocalModules(moduleType) ⇒ [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject)
Returns all modules installed locally (default: Bible modules).

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;ModuleObject&gt;</code>](#ModuleObject) - An array of ModuleObjects which represents the locally installed Bible modules.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| moduleType | <code>String</code> | <code>BIBLE</code> | A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY) |

<a name="NodeSwordInterface+getRepoModuleCount"></a>

### nodeSwordInterface.getRepoModuleCount(repositoryName, moduleType) ⇒ <code>Number</code>
Returns the number of modules for a given repository (default: Bible modules).

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Number</code> - The number of Bible modules for the given repository and module type.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| repositoryName | <code>String</code> |  | The name of the given repository. |
| moduleType | <code>String</code> | <code>BIBLE</code> | A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY) |

<a name="NodeSwordInterface+getRepoLanguageModuleCount"></a>

### nodeSwordInterface.getRepoLanguageModuleCount(repositoryName, language, moduleType) ⇒ <code>Number</code>
Returns the number of modules for a given repository and language (default: Bible modules).

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Number</code> - The number of Bible modules for the given repository, language and module type.  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| repositoryName | <code>String</code> |  | The name of the given repository. |
| language | <code>String</code> |  | The language code that shall be used as a filter. |
| moduleType | <code>String</code> | <code>BIBLE</code> | A filter parameter that defines the moduleType (Options: BIBLE, DICT, COMMENTARY) |

<a name="NodeSwordInterface+installModule"></a>

### nodeSwordInterface.installModule(repositoryName, moduleCode, progressCB) ⇒ <code>Promise</code>
Installs a module. The module is downloaded from the corresponding repository 
and then installed in the local SWORD directory.
This operation may take some time depending on the available bandwidth and geographical
distance to the SWORD repository server.

This function works asynchronously and returns a Promise object.

If the installation fails, the Promise will be rejected with the following status codes (based on SWORD):
-1: General installation issue
-9: Installation cancelled by user or internet connection suddenly interrupted

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the repository from which to install. If undefined/null, the repository is automatically determined. |
| moduleCode | <code>String</code> | The module code of the SWORD module that shall be installed. |
| progressCB | <code>function</code> | Callback function that is called on progress events. |

<a name="NodeSwordInterface+cancelInstallation"></a>

### nodeSwordInterface.cancelInstallation()
Cancels an ongoing module installation.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+uninstallModule"></a>

### nodeSwordInterface.uninstallModule(repositoryName, moduleCode) ⇒ <code>Promise</code>
Uninstalls a module.

This function works asynchronously and returns a Promise object.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the repository from which the module was installed. If undefined/null, all modules with this name will be uninstalled. |
| moduleCode | <code>String</code> | The module code of the SWORD module that shall be uninstalled. |

<a name="NodeSwordInterface+refreshLocalModules"></a>

### nodeSwordInterface.refreshLocalModules()
Refresh the local module database. This function is purely for testing.
It will usually be called after changing the SWORD module database outside of the actual application.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+saveModuleUnlockKey"></a>

### nodeSwordInterface.saveModuleUnlockKey(moduleCode, key)
Persistently saves the unlock key of the corresponding module in the module's .conf file
(in ~/.sword/mods.d/<modname>.conf)

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| key | <code>String</code> | The unlock key. |

<a name="NodeSwordInterface+isModuleReadable"></a>

### nodeSwordInterface.isModuleReadable(moduleCode) ⇒ <code>Boolean</code>
Checks whether the module is readable.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getModuleDescription"></a>

### nodeSwordInterface.getModuleDescription(repositoryName, moduleCode) ⇒ <code>String</code>
Returns the description of a module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>String</code> - The description of the respective module.  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the repository to search in. If undefined/null, searches all repositories. |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+enableMarkup"></a>

### nodeSwordInterface.enableMarkup()
Enables available markup (like Strongs, foot notes, etc.)
This influences the output for getChapterText, getBookText and getBibleText.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+disableMarkup"></a>

### nodeSwordInterface.disableMarkup()
Disable available markup (like Strongs, foot notes, etc.)
This influences the output for getChapterText, getBookText and getBibleText.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+enableStrongsWithNbsp"></a>

### nodeSwordInterface.enableStrongsWithNbsp()
Enables rendering of Strongs elements with non-breaking spaces.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+getRawModuleEntry"></a>

### nodeSwordInterface.getRawModuleEntry(moduleCode, key) ⇒ <code>String</code>
Returns the raw text of an entry for the given module and key.
If no entry exists for the given key the return value is undefined.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| key | <code>String</code> | The key of the entry. |

<a name="NodeSwordInterface+getReferenceText"></a>

### nodeSwordInterface.getReferenceText(moduleCode, key) ⇒ [<code>VerseObject</code>](#VerseObject)
Returns the text of an entry for the given module and key.
If no entry exists for the given key the return value is a verse with empty content.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| key | <code>String</code> | The key of the entry. |

<a name="NodeSwordInterface+getChapterText"></a>

### nodeSwordInterface.getChapterText(moduleCode, bookCode, chapter) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns the text of a chapter for the given module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject) - An array of verse objects.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |
| chapter | <code>Number</code> | The chapter that shall be returned. |

<a name="NodeSwordInterface+getBookText"></a>

### nodeSwordInterface.getBookText(moduleCode, bookCode, startVerseNr, verseCount) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns the text of a book for the given module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject) - An array of verse objects.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |
| startVerseNr | <code>Number</code> | The start verse number (absolute) where we start reading (Optional) |
| verseCount | <code>Number</code> | The number of verses that shall be returned (Optional) |

<a name="NodeSwordInterface+getVersesFromReferences"></a>

### nodeSwordInterface.getVersesFromReferences(moduleCode, references) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns an array of verses based on the given array of verse references

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject) - An array of verse objects.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| references | <code>Array</code> | A list of OSIS references. (like ['Gal.5.1', '1Cor.2.4']) |

<a name="NodeSwordInterface+getReferencesFromReferenceRange"></a>

### nodeSwordInterface.getReferencesFromReferenceRange(referenceRange) ⇒ <code>Array.&lt;String&gt;</code>
Returns an array of individual verse references based on an OSIS reference range expression.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of OSIS references.  

| Param | Type | Description |
| --- | --- | --- |
| referenceRange | <code>String</code> | An OSIS reference range expression. (like 'Gal.1.15-Gal.1.16') |

<a name="NodeSwordInterface+getBookList"></a>

### nodeSwordInterface.getBookList(moduleCode) ⇒ <code>Array.&lt;String&gt;</code>
Returns the list of books available in the given module. By default the book codes will be in OSIS format.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of book codes.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getBookHeaderList"></a>

### nodeSwordInterface.getBookHeaderList(moduleCode, bookCode, startVerseNumber, verseCount) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns the list of headers available in the given book. The headers are returned as an array of VerseObjects.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type |
| --- | --- |
| moduleCode | <code>String</code> | 
| bookCode | <code>String</code> | 
| startVerseNumber | <code>Number</code> | 
| verseCount | <code>Number</code> | 

<a name="NodeSwordInterface+getBookChapterCount"></a>

### nodeSwordInterface.getBookChapterCount(moduleCode, bookCode)
Returns the chapter count of the given book.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |

<a name="NodeSwordInterface+getChapterVerseCount"></a>

### nodeSwordInterface.getChapterVerseCount(moduleCode, bookCode, chapter)
Returns the number of verses in the given chapter.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |
| chapter | <code>Number</code> | The chapter of the book. |

<a name="NodeSwordInterface+getAllChapterVerseCounts"></a>

### nodeSwordInterface.getAllChapterVerseCounts(moduleCode, bookCode) ⇒ <code>Number</code>
Returns an array with the chapter verse counts of all chapters of a given book.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Number</code> - chapterVerseCounts  

| Param | Type |
| --- | --- |
| moduleCode | <code>String</code> | 
| bookCode | <code>String</code> | 

<a name="NodeSwordInterface+getBookVerseCount"></a>

### nodeSwordInterface.getBookVerseCount(moduleCode, bookCode) ⇒ <code>Number</code>
Returns the number verses in the given book.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Number</code> - The verse count of the book.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |

<a name="NodeSwordInterface+getBibleText"></a>

### nodeSwordInterface.getBibleText(moduleCode) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns the Bible text of a module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject) - An array of verse objects.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getBookIntroduction"></a>

### nodeSwordInterface.getBookIntroduction(moduleCode, bookCode) ⇒ <code>String</code>
Returns the introduction of the given book.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |

<a name="NodeSwordInterface+moduleHasBook"></a>

### nodeSwordInterface.moduleHasBook(moduleCode, bookCode) ⇒ <code>Boolean</code>
Checks whether a module has a certain book.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type |
| --- | --- |
| moduleCode | <code>String</code> | 
| bookCode | <code>String</code> | 

<a name="NodeSwordInterface+getDictModuleKeys"></a>

### nodeSwordInterface.getDictModuleKeys(moduleCode) ⇒ <code>Array.&lt;String&gt;</code>
Returns the keys of a dictionary module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type |
| --- | --- |
| moduleCode | <code>String</code> | 

<a name="NodeSwordInterface+getModuleSearchResults"></a>

### nodeSwordInterface.getModuleSearchResults(moduleCode, searchTerm, progressCB, searchType, searchScope, isCaseSensitive, useExtendedVerseBoundaries, filterOnWordBoundaries) ⇒ <code>Promise</code>
Returns the results of a module search.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Default | Description |
| --- | --- | --- | --- |
| moduleCode | <code>String</code> |  | The module code of the SWORD module. |
| searchTerm | <code>String</code> |  | The term to search for. |
| progressCB | <code>function</code> |  | Optional callback function that is called on progress events. |
| searchType | <code>String</code> | <code>phrase</code> | Options: phrase, multiWord, strongsNumber |
| searchScope | <code>String</code> | <code>BIBLE</code> | Options: BIBLE, OT, NT |
| isCaseSensitive | <code>Boolean</code> | <code>false</code> | Whether the search is case sensitive |
| useExtendedVerseBoundaries | <code>Boolean</code> | <code>false</code> | Whether the search should use extended verse boundaries (Two verses instead of one) in case of a multi word search. |
| filterOnWordBoundaries | <code>Boolean</code> | <code>false</code> | Whether to filter results based on word boundaries. |

<a name="NodeSwordInterface+terminateModuleSearch"></a>

### nodeSwordInterface.terminateModuleSearch()
Terminates the currently ongoing module search.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+hebrewStrongsAvailable"></a>

### nodeSwordInterface.hebrewStrongsAvailable() ⇒ <code>Boolean</code>
Checks whether Hebrew Strong's definitions are available.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+greekStrongsAvailable"></a>

### nodeSwordInterface.greekStrongsAvailable() ⇒ <code>Boolean</code>
Checks whether Greek Strong's definitions are available.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+strongsAvailable"></a>

### nodeSwordInterface.strongsAvailable() ⇒ <code>Boolean</code>
Checks whether Strongs definitions are available

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+getStrongsEntry"></a>

### nodeSwordInterface.getStrongsEntry(strongsKey) ⇒ [<code>StrongsEntry</code>](#StrongsEntry)
Returns the Strong's entry for a given key.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>StrongsEntry</code>](#StrongsEntry) - A StrongsEntry object.  

| Param | Type | Description |
| --- | --- | --- |
| strongsKey | <code>String</code> | The Strong's key for the requested entry. |

<a name="NodeSwordInterface+getLocalModule"></a>

### nodeSwordInterface.getLocalModule(moduleCode) ⇒ [<code>ModuleObject</code>](#ModuleObject)
Returns an object representation of a locally installed SWORD module. If the requested `moduleCode` is not available
`undefined` will be returned.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+isModuleInUserDir"></a>

### nodeSwordInterface.isModuleInUserDir(moduleCode) ⇒ <code>Boolean</code>
Checks whether the module resides in the user directory.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+isModuleAvailableInRepo"></a>

### nodeSwordInterface.isModuleAvailableInRepo(repositoryName, moduleCode) ⇒ <code>Boolean</code>
Checks whether the module is available in any repository.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the repository to check. If undefined/null, checks all repositories. |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getSwordTranslation"></a>

### nodeSwordInterface.getSwordTranslation(originalString, localeCode)
Uses the Sword LocaleMgr to translate a string. This can be used to translate book names for example.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type |
| --- | --- |
| originalString | <code>String</code> | 
| localeCode | <code>String</code> | 

<a name="NodeSwordInterface+getBookAbbreviation"></a>

### nodeSwordInterface.getBookAbbreviation(moduleName, bookCode, localeCode)
Uses the Sword LocaleMgr to translate a book abbreviation.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type |
| --- | --- |
| moduleName | <code>String</code> | 
| bookCode | <code>String</code> | 
| localeCode | <code>String</code> | 

<a name="NodeSwordInterface+unTarGZ"></a>

### nodeSwordInterface.unTarGZ(filePath, destPath) ⇒ <code>Boolean</code>
Extracts a tar.gz file to a destination path.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Boolean</code> - True if successful, false otherwise.  

| Param | Type | Description |
| --- | --- | --- |
| filePath | <code>String</code> | The path to the tar.gz file. |
| destPath | <code>String</code> | The destination path where the file should be extracted. |

<a name="NodeSwordInterface+unZip"></a>

### nodeSwordInterface.unZip(filePath, destPath) ⇒ <code>Boolean</code>
Extracts a zip file to a destination path.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Boolean</code> - True if successful, false otherwise.  

| Param | Type | Description |
| --- | --- | --- |
| filePath | <code>String</code> | The path to the zip file. |
| destPath | <code>String</code> | The destination path where the file should be extracted. |

<a name="NodeSwordInterface+getSwordVersion"></a>

### nodeSwordInterface.getSwordVersion() ⇒ <code>String</code>
Returns the version of the SWORD library

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>String</code> - SWORD library version.  
<a name="NodeSwordInterface+getSwordPath"></a>

### nodeSwordInterface.getSwordPath() ⇒ <code>String</code>
Returns the platform-specific path where SWORD accesses and stores its modules.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>String</code> - Platform-specific SWORD path.  
<a name="VerseObject"></a>

## VerseObject : <code>Object</code>
An object representation of a Bible verse.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The name/code of the SWORD module |
| bibleBookShortTitle | <code>String</code> | The short title of the verses's Bible book |
| chapter | <code>String</code> | The chapter number |
| verseNr | <code>String</code> | The verse number |
| absoluteVerseNr | <code>Number</code> | The absolute number of the verse within the book (independent of chapters, starting from 1) |
| content | <code>String</code> | The verse content |

<a name="ModuleObject"></a>

## ModuleObject : <code>Object</code>
An object representation of a SWORD module.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| name | <code>String</code> | The name/code of the SWORD module |
| type | <code>String</code> | The type of the SWORD module (Currently the following types are supported: Biblical Texts, Lexicons / Dictionaries, Commentaries) |
| description | <code>String</code> | The description of the SWORD module |
| language | <code>String</code> | The language code of the SWORD module |
| distributionLicense | <code>String</code> | The distribution license of the SWORD module |
| shortCopyright | <code>String</code> | The short copyright information of the SWORD module |
| version | <code>String</code> | The version of the SWORD module |
| lastUpdate | <code>String</code> | The date of the last version update of the SWORD module |
| history | <code>Array.&lt;String&gt;</code> | The history of the SWORD module. A list of strings where each looks like this: <version>=<version-info> |
| category | <code>String</code> | The category of the SWORD module |
| repository | <code>String</code> | The repository of the SWORD module |
| about | <code>String</code> | Extended description of the SWORD module |
| abbreviation | <code>String</code> | The abbreviation of the SWORD module |
| size | <code>Number</code> | The file size of the SWORD module (in KB) |
| location | <code>String</code> | The filesystem location where the module is stored |
| unlockInfo | <code>String</code> | Information about how to retrieve an unlock key (in case the module is locked) |
| inUserDir | <code>Boolean</code> | Information on whether the module is located in the user's directory |
| locked | <code>Boolean</code> | Information on whether the module is locked |
| hasStrongs | <code>Boolean</code> | Information on whether the module has Strong's numbers |
| hasGreekStrongsKeys | <code>Boolean</code> | Information on whether the module has Strong's based keys (Greek) |
| hasHebrewStrongsKeys | <code>Boolean</code> | Information on whether the module has Strong's based keys (Hebrew) |
| hasFootnotes | <code>Boolean</code> | Information on whether the module has footnotes |
| hasHeadings | <code>Boolean</code> | Information on whether the module has headings |
| hasRedLetterWords | <code>Boolean</code> | Information on whether the module has red letter words |
| hasCrossReferences | <code>Boolean</code> | Information on whether the module has cross references |
| isRightToLeft | <code>Boolean</code> | Information on whether the module has right to left text direction |

<a name="StrongsReference"></a>

## StrongsReference : <code>Object</code>
An object representation of a Strongs reference

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| text | <code>String</code> | The full text of the reference |
| key | <code>String</code> | The reference key |

<a name="StrongsEntry"></a>

## StrongsEntry : <code>Object</code>
An object representation of a Strong's entry.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| rawEntry | <code>String</code> | The full raw entry from the SWORD Strong's module |
| key | <code>String</code> | The key of the Strong's entry |
| transcription | <code>String</code> | The transcription of the Strong's entry |
| phoneticTranscription | <code>String</code> | The phonetic transcription of the Strong's entry |
| definition | <code>String</code> | The Strong's definition |
| references | [<code>Array.&lt;StrongsReference&gt;</code>](#StrongsReference) | The "see also" references of the Strong's entry |

