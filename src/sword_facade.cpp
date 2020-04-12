/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>

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

// System includes
#include <stdlib.h>

// STD C++ includes
#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <fstream>
#include <map>
#include <mutex>

// Sword includes
#include <installmgr.h>
#include <swmodule.h>
#include <swmgr.h>
#include <remotetrans.h>
#include <versekey.h>
#include <swlog.h>
#include <swversion.h>
#include <localemgr.h>

// Own includes
#include "sword_facade.hpp"
#include "sword_status_reporter.hpp"
#include "string_helper.hpp"
#include "strongs_entry.hpp"
#include "percentage_calc.hpp"

/* REGEX definitions from regex.h */
/* POSIX `cflags' bits (i.e., information for `regcomp').  */

/* If this bit is set, then use extended regular expression syntax.
   If not set, then use basic regular expression syntax.  */
#ifndef REG_EXTENDED
#define REG_EXTENDED 1
#endif

/* If this bit is set, then ignore case when matching.
   If not set, then case is significant.  */
#ifndef REG_ICASE
#define REG_ICASE (REG_EXTENDED << 1)
#endif

using namespace std;
using namespace sword;

#if defined(_WIN32)
// For some reason this symbol is missing in the sword.dll on Windows, hence we include it here.
char * sword::SWBuf::nullStr = (char *)"";
#endif

SwordFacade::SwordFacade(SwordStatusReporter& statusReporter)
    : _statusReporter(statusReporter)
{
    //SWLog::getSystemLog()->setLogLevel(SWLog::LOG_DEBUG);
    this->_fileSystemHelper.createBasicDirectories();
    this->resetMgr();
}

SwordFacade::~SwordFacade()
{
    if (this->_strongsHebrew != 0) delete this->_strongsHebrew;
    if (this->_strongsGreek != 0) delete this->_strongsGreek;
}

void SwordFacade::initStrongs()
{
    if (this->_strongsHebrew == 0 || this->_strongsGreek == 0) {
        this->_strongsHebrew = this->getLocalModule("StrongsHebrew");
        this->_strongsGreek = this->getLocalModule("StrongsGreek");
    }
}

void SwordFacade::resetMgr()
{
    this->_strongsHebrew = 0;
    this->_strongsGreek = 0;

    if (this->_mgr != 0) {
        delete this->_mgr;
    }

    if (this->_mgrForInstall != 0) {
        delete this->_mgrForInstall;
    }

    if (this->_installMgr != 0) {
        delete this->_installMgr;
    }

#ifdef _WIN32
    this->_mgr = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str());
    this->_mgr->augmentModules(this->_fileSystemHelper.getSystemSwordDir().c_str());
#else
    this->_mgr = new SWMgr();
#endif

    this->_mgrForInstall = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str());
    
    this->_installMgr = new InstallMgr(this->_fileSystemHelper.getInstallMgrDir().c_str(), &this->_statusReporter);
    this->_installMgr->setUserDisclaimerConfirmed(true);

    this->initStrongs();
}

int SwordFacade::refreshRepositoryConfig()
{
    //cout << "Refreshing repository configuration ... ";

    int ret = this->_installMgr->refreshRemoteSourceConfiguration();
    if (ret != 0) {
        cout << endl << "refreshRemoteSourceConfiguration returned " << ret << endl;
        return ret;
    }

    this->_installMgr->saveInstallConf();
    //cout << "done." << endl;
    return 0;
}

int SwordFacade::refreshRemoteSources(bool force, std::function<void(unsigned int progress)>* progressCallback)
{
    vector<thread> refreshThreads;
    this->_remoteSourceUpdateCount = 0;

    if (this->getRepoNames().size() == 0 || force) {
        int ret = this->refreshRepositoryConfig();
        if (ret != 0) {
            return -1;
        }

        vector<string> sourceNames = this->getRepoNames();
        this->_remoteSourceCount = sourceNames.size();

        // Create worker threads
        for (unsigned int i = 0; i < sourceNames.size(); i++) {
            refreshThreads.push_back(this->getRemoteSourceRefreshThread(sourceNames[i], progressCallback));
        }

        // Wait for threads to finish
        for (unsigned int i = 0; i < refreshThreads.size(); i++) {
            refreshThreads[i].join();
        }
    }

    return 0;
}

static std::mutex remoteSourceUpdateMutex;

int SwordFacade::refreshIndividualRemoteSource(string remoteSourceName, std::function<void(unsigned int progress)>* progressCallback)
{
    //cout << "Refreshing source " << remoteSourceName << endl << flush;
    InstallSource* source = this->getRemoteSource(remoteSourceName);
    int result = this->_installMgr->refreshRemoteSource(source);
    if (result != 0) {
        cerr << "Failed to refresh source " << remoteSourceName << endl << flush;
    }

    remoteSourceUpdateMutex.lock();
    this->_remoteSourceUpdateCount++;
    unsigned int totalPercent = (unsigned int)calculateIntPercentage<double>(this->_remoteSourceUpdateCount,
                                                                     this->_remoteSourceCount);
    
    if (progressCallback != 0) {
        (*progressCallback)(totalPercent);
    }
    remoteSourceUpdateMutex.unlock();

    return result;
}

thread SwordFacade::getRemoteSourceRefreshThread(string remoteSourceName, std::function<void(unsigned int progress)>* progressCallback)
{
    return thread(&SwordFacade::refreshIndividualRemoteSource, this, remoteSourceName, progressCallback);
}

int SwordFacade::getRepoCount()
{
    int repoCount = 0;

    if (this->_installMgr != 0) {
        for (InstallSourceMap::iterator it = this->_installMgr->sources.begin();
             it != this->_installMgr->sources.end();
             ++it) {

            repoCount++;
        }
    }

    return repoCount;
}

vector<string> SwordFacade::getRepoNames()
{
    if (this->getRepoCount() == 0) {
        this->resetMgr();
    }

    vector<string> sourceNames;

    for (InstallSourceMap::iterator it = this->_installMgr->sources.begin();
         it != this->_installMgr->sources.end();
         ++it) {

        string source = string(it->second->caption);
        sourceNames.push_back(source);
    }

    return sourceNames;
}

InstallSource* SwordFacade::getRemoteSource(string remoteSourceName)
{
    InstallSourceMap::iterator source = this->_installMgr->sources.find(remoteSourceName.c_str());
    if (source == this->_installMgr->sources.end()) {
        cerr << "Could not find remote source " << remoteSourceName << endl;
    } else {
        return source->second;
    }

    return 0;
}

vector<SWModule*> SwordFacade::getAllRemoteModules()
{
    vector<string> repoNames = this->getRepoNames();
    vector<SWModule*> allModules;

    for (unsigned int i = 0; i < repoNames.size(); i++) {
        string currentRepo = repoNames[i];
        vector<SWModule*> repoModules = this->getAllRepoModules(currentRepo);

        for (unsigned int j = 0; j < repoModules.size(); j++) {
            allModules.push_back(repoModules[j]);
        }
    }

    return allModules;
}

SWModule* SwordFacade::getModuleFromList(vector<SWModule*>& moduleList, string moduleName)
{
    for (unsigned int i = 0; i < moduleList.size(); i++) {
      SWModule* currentModule = moduleList[i];
      if (currentModule != 0 && currentModule->getName() != 0) {
          string currentModuleName = string(currentModule->getName());
          if (currentModuleName == moduleName) {
              return currentModule;
          }
      } else {
          cerr << "Could not access module at index " << i << endl;
      }
    }

    return 0;
}

SWModule* SwordFacade::getRepoModule(string moduleName, string repoName)
{    
    if (repoName == "all") {
        repoName = this->getModuleRepo(moduleName);
    }
    
    vector<SWModule*> modules = this->getAllRepoModules(repoName);
    return this->getModuleFromList(modules, moduleName);
}

string SwordFacade::getModuleIdFromFile(string moduleFileName)
{
    static regex parentheses = regex("[\\[\\]]");
    static regex lineBreaks = regex("[\\r\\n]");
    ifstream moduleFile(moduleFileName);
    string moduleId = "";

    if (moduleFile.is_open()) {
        string line;
        std::getline(moduleFile, line);
        char firstChar = line[0];
        char lastChar = line[line.size() - 1];
        
        if (firstChar == '[' && lastChar == ']') {
            // Remove parentheses and line breaks from the first line
            // What's left is the module id
            moduleId = regex_replace(line, parentheses, "");
            moduleId = regex_replace(moduleId, lineBreaks, "");
        }
    }

    moduleFile.close();
    return moduleId;
}

vector<string> SwordFacade::getRepoModuleIds(string repoName)
{
    vector<string> moduleIds;
    InstallSource* remoteSource = this->getRemoteSource(repoName);
    FileSystemHelper fs;
    stringstream repoModuleDir;

    if (remoteSource != 0) {
        //cout << remoteSource->localShadow << endl;
        repoModuleDir << remoteSource->localShadow << fs.getPathSeparator() << "mods.d";
        vector<string> filesInRepoDir = fs.getFilesInDir(repoModuleDir.str());

        for (unsigned int i = 0; i < filesInRepoDir.size(); i++) {
            // Skip files that do not end with .conf
            if (!StringHelper::hasEnding(filesInRepoDir[i], ".conf")) {
                continue;
            }

            stringstream moduleFileName;
            moduleFileName << repoModuleDir.str() << fs.getPathSeparator() << filesInRepoDir[i];
            string currentModuleId = this->getModuleIdFromFile(moduleFileName.str());

            if (currentModuleId != "") {
                moduleIds.push_back(currentModuleId);
            }
        }
    }

    return moduleIds; 
}

vector<string> SwordFacade::getAllRepoModuleIds()
{
    vector<string> repoNames = this->getRepoNames();
    vector<string> allModuleIds;

    for (unsigned int i = 0; i < repoNames.size(); i++) {
        string currentRepo = repoNames[i];

        vector<string> currentRepoModuleIds = this->getRepoModuleIds(currentRepo);
        for (unsigned int j = 0; j < currentRepoModuleIds.size(); j++) {
            allModuleIds.push_back(currentRepoModuleIds[j]);
        }
    }

    return allModuleIds;
}

vector<SWModule*> SwordFacade::getAllRepoModules(string repoName)
{
    vector<SWModule*> modules;
    InstallSource* remoteSource = this->getRemoteSource(repoName);

    if (remoteSource != 0) {
        SWMgr* mgr = remoteSource->getMgr();

        for (ModMap::const_iterator it = mgr->Modules.begin(); it != mgr->Modules.end(); it++) {
            SWModule* currentModule = it->second;
            string moduleType = currentModule->getType();

            if (moduleType == "Biblical Texts") {
                modules.push_back(currentModule);
            }
        }
    }

    return modules;
}

vector<SWModule*> SwordFacade::getRepoModulesByLang(string repoName, string languageCode, bool headersFilter, bool strongsFilter)
{
    vector<SWModule*> allModules = this->getAllRepoModules(repoName);
    vector<SWModule*> selectedLanguageModules;

    for (unsigned int i = 0; i < allModules.size(); i++) {
      SWModule* currentModule = allModules[i];

      if ((currentModule->getType() == string("Biblical Texts")) &&
          (currentModule->getLanguage() == languageCode)) {

        bool hasHeadings = this->moduleHasGlobalOption(currentModule, "Headings");
        bool hasStrongs = this->moduleHasGlobalOption(currentModule, "Strongs");
        
        if (headersFilter && !hasHeadings) {
            continue;
        }

        if (strongsFilter && !hasStrongs) {
            continue;
        }

        selectedLanguageModules.push_back(currentModule);
      }
    }

    return selectedLanguageModules;
}

unsigned int SwordFacade::getRepoTranslationCount(string repoName)
{
    vector<SWModule*> allModules = this->getAllRepoModules(repoName);
    return (unsigned int)allModules.size();
}

unsigned int SwordFacade::getRepoLanguageTranslationCount(string repoName, string languageCode)
{
    vector<SWModule*> allModules = this->getRepoModulesByLang(repoName, languageCode);
    return (unsigned int)allModules.size();
}

vector<string> SwordFacade::getRepoLanguages(string repoName)
{
    vector<SWModule*> modules;
    vector<string> languages;

    modules = this->getAllRepoModules(repoName);

    for (unsigned int i = 0; i < modules.size(); i++) {
        SWModule* currentModule = modules[i];
        string currentLanguage = string(currentModule->getLanguage());
        
        if (find(languages.begin(), languages.end(), currentLanguage) == languages.end()) {
            // Only add the language if it is not already in the list
            languages.push_back(currentLanguage);
        }
    }

    return languages;
}

string SwordFacade::getModuleRepo(string moduleName)
{
    vector<string> repositories = this->getRepoNames();

    for (unsigned int i = 0; i < repositories.size(); i++) {
        string repo = repositories[i];
        vector<string> repoModuleIds = this->getRepoModuleIds(repo);

        for (unsigned int j = 0; j < repoModuleIds.size(); j++) {
            string currentId = repoModuleIds[j];
            if (currentId == moduleName) {
                return repo;
            }
        }
    }

    return "";
}

vector<SWModule*> SwordFacade::getAllLocalModules()
{
    vector<SWModule*> allLocalModules;

    for (ModMap::iterator modIterator = this->_mgr->Modules.begin();
         modIterator != this->_mgr->Modules.end();
         modIterator++) {
        
        SWModule* currentModule = (SWModule*)modIterator->second;
        string moduleType = string(currentModule->getType());

        if (moduleType == string("Biblical Texts")) {
            allLocalModules.push_back(currentModule);
        }
    }

    return allLocalModules;
}

SWModule* SwordFacade::getLocalModule(string moduleName)
{
    return this->_mgr->getModule(moduleName.c_str());
}

bool SwordFacade::isModuleInUserDir(sword::SWModule* module)
{
    if (module == 0) {
        cerr << "isModuleInUserDir: module is zero pointer!" << endl;
        return false;
    } else {
        string modulePath = this->getModuleDataPath(module);

        if (modulePath == "") {
            return false;
        } else {
            string userDir = this->_fileSystemHelper.getUserSwordDir();
            return (modulePath.find(userDir) != string::npos);
        }
    }
}

bool SwordFacade::isModuleInUserDir(string moduleName)
{
    SWModule* module = this->getLocalModule(moduleName);
    return this->isModuleInUserDir(module);
}

bool SwordFacade::isModuleAvailableInRepo(string moduleName, string repoName)
{
    vector<string> moduleIds;
    
    if (repoName == "all") {
        moduleIds = this->getAllRepoModuleIds();
    } else {
        moduleIds = this->getRepoModuleIds(repoName);
    }

    for (unsigned int i = 0; i < moduleIds.size(); i++) {
        if (moduleIds[i] == moduleName) {
            return true;
        }
    }

    return false;
}

string SwordFacade::replaceSpacesInStrongs(const string& text)
{
    string input = text;
    static regex strongsWText = regex(">[^<]*</w>");
    static std::regex space(" ");
    smatch m;
    string filteredText;

    // Search for Strongs pattern and then replace all spaces within each occurance
    while (std::regex_search(input, m, strongsWText)) {
        filteredText += m.prefix();
        filteredText += std::regex_replace(m[0].str(), space, "&nbsp;");
        input = m.suffix();
    }
    filteredText += input;

    return filteredText;
}

bool SwordFacade::moduleHasStrongsZeroPrefixes(sword::SWModule* module)
{
    string key = "Gen 1:1";
    module->setKey(key.c_str());
    this->enableMarkup();
    string verseText = this->getCurrentVerseText(module, true);
    
    return verseText.find("strong:H0") != string::npos;
}

string SwordFacade::getModuleDataPath(sword::SWModule* module)
{
    if (module == 0) {
        return "";
    }

    string dataPath = string(module->getConfigEntry("AbsoluteDataPath"));

    #if _WIN32
        static regex slash("/");
        dataPath = regex_replace(dataPath, slash, "\\");
    #endif

    return dataPath;
}

bool SwordFacade::isModuleReadable(sword::SWModule* module, std::string key)
{
    module->setKey(key.c_str());
    string verseText = this->getCurrentVerseText(module, false);
    return verseText.size() > 0;
}

string SwordFacade::getFilteredText(const string& text, bool hasStrongs)
{
    static regex schlachterMarkupFilter = regex("<H.*> ");
    static regex chapterFilter = regex("<chapter.*/>");
    static regex lbBeginParagraph = regex("<lb type=\"x-begin-paragraph\"/>");
    static regex lbEndParagraph = regex("<lb type=\"x-end-paragraph\"/>");
    static regex lbElementFilter = regex("<lb ");
    static regex lElementFilter = regex("<l ");
    static regex lgElementFilter = regex("<lg ");
    static regex noteStartElementFilter = regex("<note");
    static regex noteEndElementFilter = regex("</note>");
    static regex headStartElementFilter = regex("<head");
    static regex headEndElementFilter = regex("</head>");
    static regex appStartElementFilter = regex("<app");
    static regex appEndElementFilter = regex("</app>");
    static regex rtxtFilter = regex("<rtxt type=\"omit\"/>");
    static regex quoteJesusElementFilter = regex("<q marker=\"\" who=\"Jesus\">");
    static regex quoteElementFilter = regex("<q ");
    static regex titleStartElementFilter = regex("<title");
    static regex titleEndElementFilter = regex("</title>");
    static regex divMilestoneFilter = regex("<div type=\"x-milestone\"");
    static regex milestoneFilter = regex("<milestone");
    static regex xBrFilter = regex("x-br\"/>");
    static regex divSIDFilter = regex("<div sID=");
    static regex divEIDFilter = regex("<div eID=");
    static regex divineNameStartElement = regex("<divineName>");
    static regex divineNameEndElement = regex("</divineName>");
    static regex strongsWElement = regex("<w lemma=");

    static regex fullStopWithoutSpace = regex("[.]<");
    static regex questionMarkWithoutSpace = regex("[?]<");
    static regex exclamationMarkWithoutSpace = regex("!<");
    static regex commaWithoutSpace = regex(",<");
    static regex semiColonWithoutSpace = regex(";<");
    static regex colonWithoutSpace = regex(":<");

    string filteredText = text;
    filteredText = regex_replace(filteredText, schlachterMarkupFilter, "");
    filteredText = regex_replace(filteredText, chapterFilter, "");
    filteredText = regex_replace(filteredText, lbBeginParagraph, "");
    filteredText = regex_replace(filteredText, lbEndParagraph, "&nbsp;<div class=\"sword-markup sword-paragraph-end\"><br/></div>");
    filteredText = regex_replace(filteredText, lbElementFilter, "<div class=\"sword-markup sword-lb\" ");
    filteredText = regex_replace(filteredText, lElementFilter, "<div class=\"sword-markup sword-l\" ");
    filteredText = regex_replace(filteredText, lgElementFilter, "<div class=\"sword-markup sword-lg\" ");
    filteredText = regex_replace(filteredText, noteStartElementFilter, "<div class=\"sword-markup sword-note\" ");
    filteredText = regex_replace(filteredText, noteEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, headStartElementFilter, "<div class=\"sword-markup sword-head\" ");
    filteredText = regex_replace(filteredText, headEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, appStartElementFilter, "<div class=\"sword-markup sword-app\" ");
    filteredText = regex_replace(filteredText, appEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, rtxtFilter, "");

    filteredText = regex_replace(filteredText, titleStartElementFilter, "<div class=\"sword-markup sword-section-title\"");
    filteredText = regex_replace(filteredText, titleEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, divMilestoneFilter, "<div class=\"sword-markup sword-x-milestone\"");
    filteredText = regex_replace(filteredText, milestoneFilter, "<div class=\"sword-markup sword-milestone\"");
    filteredText = regex_replace(filteredText, xBrFilter, "x-br\"/> ");
    filteredText = regex_replace(filteredText, divSIDFilter, "<div class=\"sword-markup sword-sid\" sID=");
    filteredText = regex_replace(filteredText, divEIDFilter, "<div class=\"sword-markup sword-eid\" eID=");
    filteredText = regex_replace(filteredText, quoteJesusElementFilter, "<div class=\"sword-markup sword-quote-jesus\"/>");
    filteredText = regex_replace(filteredText, quoteElementFilter, "&quot;<div class=\"sword-markup sword-quote\" ");
    filteredText = regex_replace(filteredText, divineNameStartElement, "");
    filteredText = regex_replace(filteredText, divineNameEndElement, "");
    filteredText = regex_replace(filteredText, strongsWElement, "<w class=");

    filteredText = regex_replace(filteredText, fullStopWithoutSpace, ". <");
    filteredText = regex_replace(filteredText, questionMarkWithoutSpace, "? <");
    filteredText = regex_replace(filteredText, exclamationMarkWithoutSpace, "! <");
    filteredText = regex_replace(filteredText, commaWithoutSpace, ", <");
    filteredText = regex_replace(filteredText, semiColonWithoutSpace, "; <");
    filteredText = regex_replace(filteredText, colonWithoutSpace, ": <");

    if (hasStrongs) {
        filteredText = this->replaceSpacesInStrongs(filteredText);
    }

    return filteredText;
}

string SwordFacade::getCurrentChapterHeading(sword::SWModule* module)
{
    string chapterHeading = "";
    VerseKey currentVerseKey(module->getKey());

    if (currentVerseKey.getVerse() == 1) { // X:1, set key to X:0
        // Include chapter/book/testament/module intros
        currentVerseKey.setIntros(true);
        currentVerseKey.setVerse(0);

        module->setKey(currentVerseKey);
        chapterHeading = string(module->getRawEntry());
        StringHelper::trim(chapterHeading);

        // Set verse back to X:1
        currentVerseKey.setVerse(1);
        module->setKey(currentVerseKey);
    }

    chapterHeading = this->getFilteredText(chapterHeading);
    return chapterHeading;
}

string SwordFacade::getCurrentVerseText(sword::SWModule* module, bool hasStrongs, bool forceNoMarkup)
{
    string verseText;
    string filteredText;

    if (this->_markupEnabled && !forceNoMarkup) {
        verseText = string(module->getRawEntry());
        StringHelper::trim(verseText);
        filteredText = this->getFilteredText(verseText, hasStrongs);
    } else {
        verseText = string(module->stripText());
        StringHelper::trim(verseText);
        filteredText = verseText;
    }
    
    return filteredText;
}

vector<Verse> SwordFacade::getBibleText(string moduleName)
{
    return this->getText(moduleName, "Gen 1:1");
}

vector<Verse> SwordFacade::getBookText(string moduleName, string bookCode, int startVerseNumber, int verseCount)
{
    stringstream key;
    key << bookCode;
    key << " 1:1";

    return this->getText(moduleName, key.str(), QueryLimit::book, startVerseNumber, verseCount);
}

vector<Verse> SwordFacade::getChapterText(string moduleName, string bookCode, int chapter)
{
    stringstream key;
    key << bookCode << " " << chapter << ":1";

    return this->getText(moduleName, key.str(), QueryLimit::chapter);
}

vector<Verse> SwordFacade::getText(string moduleName, string key, QueryLimit queryLimit, int startVerseNumber, int verseCount)
{
    SWModule* module = this->getLocalModule(moduleName);
    string lastKey;
    int index = 0;
    string lastBookName = "";
    int lastChapter = -1;
    bool currentBookExisting = true;

    // This holds the text that we will return
    vector<Verse> text;

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        bool hasStrongs = this->moduleHasGlobalOption(module, "Strongs");

        module->setKey(key.c_str());

        if (startVerseNumber >= 1) {
          module->increment(startVerseNumber - 1);
        } else {
          startVerseNumber = 1;
        }

        for (;;) {
            VerseKey currentVerseKey(module->getKey());
            string currentBookName(currentVerseKey.getBookAbbrev());
            int currentChapter = currentVerseKey.getChapter();
            bool firstVerseInBook = false;
            bool firstVerseInChapter = (currentVerseKey.getVerse() == 1);
            string verseText = "";
            string currentKey(module->getKey()->getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }
            // Stop, once the newly ready key is a different book than the previously read key
            if (queryLimit == QueryLimit::book && (index > 0) && (currentBookName != lastBookName)) { break; }
            // Stop, once the newly ready key is a different chapter than the previously read key
            if (queryLimit == QueryLimit::chapter && (index > 0) && (currentChapter != lastChapter)) { break; }
            // Stop once the maximum number of verses is reached
            if (startVerseNumber >= 1 && verseCount >= 1 && (index == verseCount)) { break; }

            if (currentBookName != lastBookName) {
                currentBookExisting = true;
                firstVerseInBook = true;
            }

            // Chapter heading
            if (firstVerseInChapter) {
                verseText += this->getCurrentChapterHeading(module);
            }

            // Current verse text
            verseText += this->getCurrentVerseText(module, hasStrongs);

            // If the current verse does not have any content and if it is the first verse in this book
            // we assume that the book is not existing.
            if (verseText.length() == 0 && firstVerseInBook) { currentBookExisting = false; }

            if (currentBookExisting) {
                Verse currentVerse;
                currentVerse.reference = module->getKey()->getShortText();
                currentVerse.absoluteVerseNumber = startVerseNumber + index;
                currentVerse.content = verseText;
                text.push_back(currentVerse);
            }

            lastKey = currentKey;
            lastBookName = currentBookName;
            lastChapter = currentChapter;
            module->increment();
            index++;
        }
    }

    return text;
}

string SwordFacade::getBookIntroduction(string moduleName, string bookCode)
{
    string bookIntroText = "";
    string filteredText = "";
    SWModule* module = this->getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        VerseKey verseKey(bookCode.c_str());

        // Include chapter/book/testament/module intros
        verseKey.setIntros(true);
        verseKey.setChapter(0);
        verseKey.setVerse(0);
        module->setKey(verseKey);

        bookIntroText = string(module->getRawEntry());
        StringHelper::trim(bookIntroText);

        static regex titleStartElementFilter = regex("<title");
        static regex titleEndElementFilter = regex("</title>");
        static regex noteStartElementFilter = regex("<note");
        static regex noteEndElementFilter = regex("</note>");
        static regex headStartElementFilter = regex("<head");
        static regex headEndElementFilter = regex("</head>");
        static regex chapterDivFilter = regex("<div type=\"chapter\" n=\"[0-9]{1}\" id=\"[-A-Z0-9]{1,8}\">");

        filteredText = bookIntroText;
        filteredText = regex_replace(filteredText, titleStartElementFilter, "<div class=\"sword-markup sword-book-title\"");
        filteredText = regex_replace(filteredText, titleEndElementFilter, "</div>");
        filteredText = regex_replace(filteredText, noteStartElementFilter, "<div class=\"sword-markup sword-note\"");
        filteredText = regex_replace(filteredText, noteEndElementFilter, "</div>");
        filteredText = regex_replace(filteredText, headStartElementFilter, "<div class=\"sword-markup sword-head\"");
        filteredText = regex_replace(filteredText, headEndElementFilter, "</div>");
        filteredText = regex_replace(filteredText, chapterDivFilter, "");
    }

    return filteredText;
}

vector<string> SwordFacade::getBookList(string moduleName)
{
    string currentBookName = "";
    string lastBookName = "";
    string lastKey = "";
    vector<string> bookList;
    SWModule* module = this->getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        module->setKey("Gen 1:1");

        for (;;) {
            VerseKey currentVerseKey(module->getKey());
            currentBookName = currentVerseKey.getBookAbbrev();
            string currentKey(module->getKey()->getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }

            if (currentBookName != lastBookName) {
                string firstVerseText = string(module->getRawEntry());

                if (firstVerseText.length() != 0) {
                    // We assume the book is existing if the first verse has content
                    bookList.push_back(currentBookName);
                }
            }

            module->increment();
            lastBookName = currentBookName;
            lastKey = currentKey;
        }
    }

    return bookList;
}

map<string, vector<int>> SwordFacade::getBibleChapterVerseCounts(std::string moduleName)
{
    string currentBookName = "";
    string lastBookName = "";
    string lastKey = "";
    int lastChapter = -1;
    int currentChapterCount = 0;
    map<string, vector<int>> bibleChapterVerseCounts;
    SWModule* module = this->getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        module->setKey("Gen 1:1");

        for (;;) {
            VerseKey currentVerseKey(module->getKey());
            currentBookName = currentVerseKey.getBookAbbrev();
            int currentChapter = currentVerseKey.getChapter();
            string currentKey(module->getKey()->getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }

            if (currentBookName != lastBookName) {
                // Init a new chapter verse count vector for the new book
                vector<int> currentChapterVerseCounts;
                bibleChapterVerseCounts[currentBookName] = currentChapterVerseCounts;
            }

            if (lastChapter != -1 && (currentChapter != lastChapter || currentBookName != lastBookName)) {
                bibleChapterVerseCounts[lastBookName].push_back(currentChapterCount);
                currentChapterCount = 0;
            }

            currentChapterCount++;

            module->increment();
            lastChapter = currentChapter;
            lastBookName = currentBookName;
            lastKey = currentKey;
        }

        bibleChapterVerseCounts[currentBookName].push_back(currentChapterCount);
    }

    return bibleChapterVerseCounts;
}

map<string, int> SwordFacade::getAbsoluteVerseNumberMap(SWModule* module)
{
    string lastBookName = "";
    string lastKey = "";
    int currentAbsoluteVerseNumber = 1;

    std::map<std::string, int> absoluteVerseNumbers;
    module->setKey("Gen 1:1");

    for (;;) {
        VerseKey currentVerseKey(module->getKey());
        string currentBookName(currentVerseKey.getBookAbbrev());
        string currentKey(module->getKey()->getShortText());

        // Stop, once the newly read key is the same as the previously read key
        if (currentKey == lastKey) { break; }

        // Reset the currentAbsoluteVerseNumber when a new book is started
        if ((currentAbsoluteVerseNumber > 0) && (currentBookName != lastBookName)) { currentAbsoluteVerseNumber = 1; }

        absoluteVerseNumbers[currentKey] = currentAbsoluteVerseNumber;

        module->increment();
        currentAbsoluteVerseNumber++;
        lastBookName = currentBookName;
        lastKey = currentKey;
    }

    return absoluteVerseNumbers;
}

vector<Verse> SwordFacade::getModuleSearchResults(string moduleName,
                                                  string searchTerm,
                                                  SearchType searchType,
                                                  bool isCaseSensitive)
{
    SWModule* module = this->getLocalModule(moduleName);
    ListKey listKey;
    ListKey *scope = 0;
    int flags = 0;
    // This holds the text that we will return
    vector<Verse> searchResults;

    if (!isCaseSensitive) {
        // for case insensitivity
        flags |= REG_ICASE;
    }

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        bool hasStrongs = this->moduleHasGlobalOption(module, "Strongs");

        if (searchType == SearchType::strongsNumber) {
            if (!hasStrongs) {
                // Return immediately if search type is Strong's, but the module does not have Strong's support
                return searchResults;
            }

            // If the Strong's key is OT we need to insert a zero in front of the key
            // This is necessary because the Sword modules with Strong's have a zero in front of the Hebrew Strong's numbers
            if (searchTerm[0] == 'H' && this->moduleHasStrongsZeroPrefixes(module)) {
                // Cut out the number from the Strong's key (starting at index 1 until end of string)
                string strongsKey = searchTerm.substr(1, searchTerm.size());
                // Overwrite the searchTerm with an inserted 0
                searchTerm = "H0" + strongsKey;
            }

            // from swmodule.h api docs:
            // for use with entryAttrib search type to match whole entry to value, e.g., G1234 and not G12345
            flags |= SEARCHFLAG_MATCHWHOLEENTRY;
            searchTerm = "Word//Lemma./" + searchTerm;
        }

        map<string, int> absoluteVerseNumbers = this->getAbsoluteVerseNumberMap(module);

        // Perform search
        listKey = module->search(searchTerm.c_str(), int(searchType), flags, scope, 0, internalModuleSearchProgressCB);

        // Populate searchResults vector
        while (!listKey.popError()) {
            module->setKey(listKey.getElement());

            string verseText = this->getCurrentVerseText(module, hasStrongs);
            Verse currentVerse;
            currentVerse.reference = module->getKey()->getShortText();
            currentVerse.absoluteVerseNumber = absoluteVerseNumbers[currentVerse.reference];
            currentVerse.content = verseText;
            searchResults.push_back(currentVerse);

            listKey++;
        }
    }

    return searchResults;
}

StrongsEntry* SwordFacade::getStrongsEntry(string key)
{
    SWModule* module = 0;
    char strongsType = key[0];

    if (strongsType == 'H') {
        module = this->_strongsHebrew;
    } else if (strongsType == 'G') {
        module = this->_strongsGreek;
    } else {
        return 0;
    }

    if (module == 0) {
        cerr << "No valid Strong's module available!" << endl;
        return 0;
    }

    StrongsEntry* entry = StrongsEntry::getStrongsEntry(module, key);
    return entry;
}

int SwordFacade::installModule(string moduleName)
{
    string repoName = this->getModuleRepo(moduleName);

    if (repoName == "") {
        cerr << "Could not find repository for module " << moduleName << endl;
        return -1;
    }

    return this->installModule(repoName, moduleName);
}

int SwordFacade::installModule(string repoName, string moduleName)
{
    InstallSource* remoteSource = this->getRemoteSource(repoName);
    if (remoteSource == 0) {
        cerr << "Couldn't find remote source " << repoName << endl;
        return -1;
    }

    SWMgr *remoteMgr = remoteSource->getMgr();
    ModMap::iterator it = remoteMgr->Modules.find(moduleName.c_str());

    if (it == remoteMgr->Modules.end()) {
        cerr << "Did not find module " << moduleName << " in repository " << repoName << endl;
        return -1;
    } else {
        int error = this->_installMgr->installModule(this->_mgrForInstall, 0, moduleName.c_str(), remoteSource);

        progressMutex.lock();

        this->resetMgr();

        progressMutex.unlock();

        if (error) {
            cerr << "Error installing module: " << moduleName << " (write permissions?)" << endl;
            return -1;
        } else {
            cout << "Installed module: " << moduleName << endl;
            return 0;
        }
    }
}

int SwordFacade::uninstallModule(string moduleName)
{
    int error = this->_installMgr->removeModule(this->_mgrForInstall, moduleName.c_str());
    this->resetMgr();

    if (error) {
        cerr << "Error uninstalling module: " << moduleName << " (write permissions?)" << endl;
        return -1;
    } else {
        cout << "Uninstalled module: " << moduleName << endl;
        return 0;
    }
}

int SwordFacade::saveModuleUnlockKey(string moduleName, string key)
{
    if (moduleName.size() == 0 || key.size() == 0) {
        return -1;
    }

    string moduleDir = this->_fileSystemHelper.getModuleDir();
    vector<string> moduleConfFiles = this->_fileSystemHelper.getFilesInDir(moduleDir);

    for (unsigned int i = 0; i < moduleConfFiles.size(); i++) {
        string currentFileName = moduleConfFiles[i];

        // Skip files that do not end with .conf
        if (!StringHelper::hasEnding(currentFileName, ".conf")) {
            continue;
        }

        string fullPath = moduleDir + this->_fileSystemHelper.getPathSeparator() + currentFileName;
        SWConfig *config = new SWConfig(fullPath.c_str());
        SectionMap::iterator section = config->getSections().find(moduleName.c_str());

        if (section != config->getSections().end()) {
            // Found config file for module
            int returnCode = 0;

            ConfigEntMap::iterator cipherKeyEntry = section->second.find("CipherKey");
            if (cipherKeyEntry != section->second.end()) {
                // "Found CipherKey section ... saving new key!"
                //-- set cipher key
                cipherKeyEntry->second = key.c_str();
                //-- save config file
                config->save();
                // Reset the mgr to reload the modules
                this->resetMgr();
                // Without this step we cannot load a remote module afterwards ...
                this->refreshRemoteSources(true);
            } else {
                // Section CipherKey not found!
                returnCode = -2;
            }

            delete config;
            return returnCode;
        }
        
        delete config;
    }

    // Module file not found
    return -3;
}

string SwordFacade::getSwordTranslation(string configPath, string originalString, string localeCode)
{
    // We only initialize this at the first execution
    if (this->_localeMgr == 0) {
        this->_localeMgr = new LocaleMgr(configPath.c_str());
    }
    
    string translation = string(this->_localeMgr->translate(originalString.c_str(), localeCode.c_str()));
    return translation;
}

bool SwordFacade::moduleHasGlobalOption(SWModule* module, string globalOption)
{
    bool hasGlobalOption = false;
    ConfigEntMap::const_iterator it = module->getConfig().lower_bound("GlobalOptionFilter");
    ConfigEntMap::const_iterator end = module->getConfig().upper_bound("GlobalOptionFilter");

    for(; it !=end; ++it) {
        string currentOption = string(it->second.c_str());
        if (currentOption.find(globalOption) != string::npos) {
            hasGlobalOption = true;
            break;
        }
    }

    return hasGlobalOption;
}

string SwordFacade::getSwordVersion()
{
    return string("1.8.900-e34fd3");
}

static std::function<void(char, void*)>* _moduleSearchProgressCB = 0;

void internalModuleSearchProgressCB(char percent, void* userData)
{
    if (_moduleSearchProgressCB != 0) {
        //cout << "internal cb: " << (int)percent << endl;

        (*_moduleSearchProgressCB)(percent, userData);
    }
}

void setModuleSearchProgressCB(std::function<void(char, void*)>* moduleSearchProgressCB)
{
    _moduleSearchProgressCB = moduleSearchProgressCB;
}
