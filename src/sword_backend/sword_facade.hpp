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

#ifndef _SWORD_FACADE
#define _SWORD_FACADE

#include <vector>
#include <map>
#include <string>
#include <thread>
#include <climits>
#include <functional>
#include <mutex>

#include <remotetrans.h>

#include "file_system_helper.hpp"
#include "module_helper.hpp"
#include "strongs_entry.hpp"
#include "repository_interface.hpp"

namespace sword {
    class InstallMgr;
    class InstallSource;
    class SWModule;
    class SWKey;
    class SWMgr;
    class SWConfig;
    class LocaleMgr;
};

enum class SearchType {
    phrase = -1,
    multiWord = -2,
    strongsNumber = -3
};

enum class QueryLimit {
    none,
    chapter,
    book
};

class Verse
{
public:
    Verse() {}
    virtual ~Verse() {}

    std::string reference;
    int absoluteVerseNumber;
    std::string content;
};

class SwordStatusReporter;

void setModuleSearchProgressCB(std::function<void(char, void*)>* moduleSearchProgressCB);
void internalModuleSearchProgressCB(char percent, void* userData);

class SwordFacade
{
public:
    SwordFacade(SwordStatusReporter& statusReporter);
    virtual ~SwordFacade();

    std::vector<sword::SWModule*> getAllLocalModules();
    sword::SWModule* getLocalModule(std::string moduleName);
    bool isModuleInUserDir(std::string moduleName);
    bool isModuleInUserDir(sword::SWModule* module);

    std::vector<Verse> getBibleText(std::string moduleName);
    std::vector<Verse> getBookText(std::string moduleName, std::string bookCode, int startVerseNumber=-1, int verseCount=-1);
    std::vector<Verse> getChapterText(std::string moduleName, std::string bookCode, int chapter);
    std::string getBookIntroduction(std::string moduleName, std::string bookCode);

    std::vector<std::string> getBookList(std::string moduleName);
    std::map<std::string, std::vector<int>> getBibleChapterVerseCounts(std::string moduleName);

    std::vector<Verse> getModuleSearchResults(std::string moduleName,
                                              std::string searchTerm,
                                              SearchType searchType=SearchType::multiWord,
                                              bool isCaseSensitive=false);

    StrongsEntry* getStrongsEntry(std::string key);

    int installModule(std::string moduleName);
    int installModule(std::string repoName, std::string moduleName);
    int uninstallModule(std::string moduleName);

    int saveModuleUnlockKey(std::string moduleName, std::string key);
    bool isModuleReadable(sword::SWModule* module, std::string key="John 1:1");

    std::string getSwordTranslation(std::string configPath, std::string originalString, std::string localeCode);
    std::string getSwordVersion();

    void enableMarkup() { this->_markupEnabled = true; }
    void disableMarkup() { this->_markupEnabled = false; }

    std::string getModuleDataPath(sword::SWModule* module);

    SwordStatusReporter& getStatusReporter() {
        return this->_statusReporter;
    }

private:
    int getRepoCount();
    std::vector<Verse> getText(std::string moduleName,
                               std::string key,
                               QueryLimit queryLimit=QueryLimit::none,
                               int startVerseNr=-1,
                               int verseCount=-1);

    void resetMgr();
    void initStrongs();

    std::string getCurrentVerseText(sword::SWModule* module, bool hasStrongs, bool forceNoMarkup=false);
    std::string getCurrentChapterHeading(sword::SWModule* module);
    std::string getFilteredText(const std::string& text, bool hasStrongs=false);
    std::string replaceSpacesInStrongs(const std::string& text);
    bool moduleHasStrongsZeroPrefixes(sword::SWModule* module);

    std::map<std::string, int> getAbsoluteVerseNumberMap(sword::SWModule* module);

    RepositoryInterface* _repoInterface = 0;

    sword::SWMgr* _mgr = 0;
    sword::SWMgr* _mgrForInstall = 0;
    sword::LocaleMgr* _localeMgr = 0;

    SwordStatusReporter& _statusReporter;
    FileSystemHelper _fileSystemHelper;
    ModuleHelper _moduleHelper;

    sword::SWModule* _strongsHebrew = 0;
    sword::SWModule* _strongsGreek = 0;

    bool _markupEnabled = false;
};

#endif // _SWORD_FACADE

