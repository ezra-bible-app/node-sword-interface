## Classes

<dl>
<dt><a href="#NodeSwordInterface">NodeSwordInterface</a></dt>
<dd><p>This is the main class of node-sword-interface and it provides a set of static functions that wrap SWORD library functionality.</p>
</dd>
</dl>

## Typedefs

<dl>
<dt><a href="#VerseObject">VerseObject</a> : <code>Object</code></dt>
<dd><p>An object representation of a bible verse.</p>
</dd>
<dt><a href="#ModuleObject">ModuleObject</a> : <code>Object</code></dt>
<dd><p>An object representation of a SWORD module.</p>
</dd>
</dl>

<a name="NodeSwordInterface"></a>

## NodeSwordInterface
This is the main class of node-sword-interface and it provides a set of static functions that wrap SWORD library functionality.

**Kind**: global class  

* [NodeSwordInterface](#NodeSwordInterface)
    * [.repositoryConfigExisting()](#NodeSwordInterface+repositoryConfigExisting) ⇒ <code>Boolean</code>
    * [.updateRepositoryConfig()](#NodeSwordInterface+updateRepositoryConfig) ⇒ <code>Promise</code>
    * [.getRepoNames()](#NodeSwordInterface+getRepoNames) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getRepoLanguages(repositoryName)](#NodeSwordInterface+getRepoLanguages) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getAllRepoModules(repositoryName)](#NodeSwordInterface+getAllRepoModules) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getRepoModulesByLang(repositoryName, language)](#NodeSwordInterface+getRepoModulesByLang) ⇒ <code>Array.&lt;String&gt;</code>
    * [.getRepoLanguageTranslationCount(repositoryName, language)](#NodeSwordInterface+getRepoLanguageTranslationCount) ⇒ <code>Number</code>
    * [.installModule(moduleCode)](#NodeSwordInterface+installModule) ⇒ <code>Promise</code>
    * [.uninstallModule(moduleCode)](#NodeSwordInterface+uninstallModule) ⇒ <code>Promise</code>
    * [.getModuleDescription(moduleCode)](#NodeSwordInterface+getModuleDescription) ⇒ <code>String</code>
    * [.getBibleText(moduleCode)](#NodeSwordInterface+getBibleText) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getBookText(moduleCode, bookCode)](#NodeSwordInterface+getBookText) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
    * [.getLocalModule(moduleCode)](#NodeSwordInterface+getLocalModule) ⇒ [<code>ModuleObject</code>](#ModuleObject)

<a name="NodeSwordInterface+repositoryConfigExisting"></a>

### nodeSwordInterface.repositoryConfigExisting() ⇒ <code>Boolean</code>
Checks whether a repository configuration is already existing locally.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+updateRepositoryConfig"></a>

### nodeSwordInterface.updateRepositoryConfig() ⇒ <code>Promise</code>
Creates or updates the repository configuration and stores the information persistently. The repository configuration contains information
about all SWORD repositories and the available modules. After this operation the local cached repository configuration will
contain the latest master repository list and updated lists of SWORD modules available from each repository.

This function must be called initially before using any other repository-related functions. Once the repository configuration has been
initialized, the use of this function is optional. However, the cached repository configuration may not contain the latest information
if this function is not called.

This function works asynchronously and returns a Promise object.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
<a name="NodeSwordInterface+getRepoNames"></a>

### nodeSwordInterface.getRepoNames() ⇒ <code>Array.&lt;String&gt;</code>
Returns the names of all available SWORD repositories.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of strings with the names of the available SWORD repositories.  
<a name="NodeSwordInterface+getRepoLanguages"></a>

### nodeSwordInterface.getRepoLanguages(repositoryName) ⇒ <code>Array.&lt;String&gt;</code>
Returns the available languages for the bible modules from a given repository.
Note that the languages are returned as language codes.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of strings with the languages codes for the bible modules from the given repository.  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the given repository. |

<a name="NodeSwordInterface+getAllRepoModules"></a>

### nodeSwordInterface.getAllRepoModules(repositoryName) ⇒ <code>Array.&lt;String&gt;</code>
Returns all bible modules for the given repository.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of strings with the module codes for the bible modules of the given repository.  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the given repository. |

<a name="NodeSwordInterface+getRepoModulesByLang"></a>

### nodeSwordInterface.getRepoModulesByLang(repositoryName, language) ⇒ <code>Array.&lt;String&gt;</code>
Returns all bible modules for the given repository and language

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Array.&lt;String&gt;</code> - An array of strings with the module codes for the bible modules of the given repository and language.  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the given repository. |
| language | <code>String</code> | The language code that shall be used as a filter. |

<a name="NodeSwordInterface+getRepoLanguageTranslationCount"></a>

### nodeSwordInterface.getRepoLanguageTranslationCount(repositoryName, language) ⇒ <code>Number</code>
Returns the number of bible modules for a given repository and language.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>Number</code> - The number of bible modules for the given repository and language.  

| Param | Type | Description |
| --- | --- | --- |
| repositoryName | <code>String</code> | The name of the given repository. |
| language | <code>String</code> | The language code that shall be used as a filter. |

<a name="NodeSwordInterface+installModule"></a>

### nodeSwordInterface.installModule(moduleCode) ⇒ <code>Promise</code>
Installs a module. The repository is automatically determined. The module is downloaded
from the corresponding repository and then installed in the local SWORD directory.
This operation may take some time depending on the available bandwidth and geographical
distance to the SWORD repository server.

This function works asynchronously and returns a Promise object.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module that shall be installed. |

<a name="NodeSwordInterface+uninstallModule"></a>

### nodeSwordInterface.uninstallModule(moduleCode) ⇒ <code>Promise</code>
Uninstalls a module.

This function works asynchronously and returns a Promise object.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module that shall be uninstalled. |

<a name="NodeSwordInterface+getModuleDescription"></a>

### nodeSwordInterface.getModuleDescription(moduleCode) ⇒ <code>String</code>
Returns the description of a module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: <code>String</code> - The description of the respective module.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getBibleText"></a>

### nodeSwordInterface.getBibleText(moduleCode) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns the bible text of a module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject) - An array of verse objects.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="NodeSwordInterface+getBookText"></a>

### nodeSwordInterface.getBookText(moduleCode, bookCode) ⇒ [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject)
Returns the text of a book for the given module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  
**Returns**: [<code>Array.&lt;VerseObject&gt;</code>](#VerseObject) - An array of verse objects.  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |
| bookCode | <code>String</code> | The book code of the SWORD module. |

<a name="NodeSwordInterface+getLocalModule"></a>

### nodeSwordInterface.getLocalModule(moduleCode) ⇒ [<code>ModuleObject</code>](#ModuleObject)
Returns an object representation of a locally installed SWORD module.

**Kind**: instance method of [<code>NodeSwordInterface</code>](#NodeSwordInterface)  

| Param | Type | Description |
| --- | --- | --- |
| moduleCode | <code>String</code> | The module code of the SWORD module. |

<a name="VerseObject"></a>

## VerseObject : <code>Object</code>
An object representation of a bible verse.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| bibleBookShortTitle | <code>String</code> | The short title of the verses's bible book. |
| chapter | <code>String</code> | The chapter number. |
| verseNr | <code>String</code> | The verse number. |
| absoluteVerseNr | <code>Number</code> | The absolute number of the verse within the book (independent of chapters, starting from 1) |
| content | <code>String</code> | The verse content. |

<a name="ModuleObject"></a>

## ModuleObject : <code>Object</code>
An object representation of a SWORD module.

**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| name | <code>String</code> | The name/code of the SWORD module. |
| description | <code>String</code> | The description of the SWORD module. |
| language | <code>String</code> | The language code of the SWORD module. |
| version | <code>String</code> | The version of the SWORD module. |
| locked | <code>String</code> | Information on whether the module is locked or not (Values: 'true' / 'false') |
| about | <code>String</code> | Extended description of the SWORD module. |
| size | <code>String</code> | The file size of the SWORD module (in KB). |
| abbreviation | <code>String</code> | The abbreviation of the SWORD module. |
