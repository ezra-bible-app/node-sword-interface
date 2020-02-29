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

#ifndef _SWORD_FACADE
#define _SWORD_FACADE

#include <vector>
#include <string>
#include <thread>
#include <climits>

#include <remotetrans.h>

#include "file_system_helper.hpp"
#include "strongs_entry.hpp"

namespace sword {
    class InstallMgr;
    class InstallSource;
    class SWModule;
    class SWMgr;
    class SWConfig;
    class LocaleMgr;
};

class SwordStatusReporter : public sword::StatusReporter
{
public:
    int last;
    virtual void update(unsigned long totalBytes, unsigned long completedBytes);
    virtual void preStatus(long totalBytes, long completedBytes, const char *message);
};

enum class SearchType {
    phrase = -1,
    multiWord = -2,
    strongsNumber = -3
};

class Verse
{
public:
    Verse() {}
    virtual ~Verse() {}

    std::string reference;
    std::string content;
};

class SwordFacade
{
public:
    SwordFacade();
    virtual ~SwordFacade();

    int refreshRepositoryConfig();
    int refreshRemoteSources(bool force=false);

    std::vector<std::string> getRepoNames();
    std::vector<sword::SWModule*> getAllRemoteModules();
    sword::SWModule* getRepoModule(std::string moduleName, std::string repoName="all");
    std::vector<sword::SWModule*> getAllRepoModules(std::string repoName);
    std::vector<sword::SWModule*> getRepoModulesByLang(std::string repoName, std::string languageCode, bool headersFilter=false, bool strongsFilter=false);
    unsigned int getRepoTranslationCount(std::string repoName);
    std::vector<std::string> getRepoLanguages(std::string repoName);
    unsigned int getRepoLanguageTranslationCount(std::string repoName, std::string languageCode);

    std::vector<sword::SWModule*> getAllLocalModules();
    sword::SWModule* getLocalModule(std::string moduleName);
    bool isModuleInUserDir(std::string moduleName);
    bool isModuleInUserDir(sword::SWModule* module);
    bool isModuleAvailableInRepo(std::string moduleName, std::string repoName="all");

    std::vector<Verse> getBibleText(std::string moduleName);
    std::vector<Verse> getBookText(std::string moduleName, std::string bookCode, int startVerseNumber=-1, int verseCount=-1);
    std::string getBookIntroduction(std::string moduleName, std::string bookCode);

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

    bool moduleHasGlobalOption(sword::SWModule* module, std::string globalOption);

private:
    int getRepoCount();
    std::vector<Verse> getText(std::string moduleName,
                               std::string bookCode,
                               bool onlyCurrentBook=true,
                               int startVerseNr=-1,
                               int verseCount=-1);

    sword::InstallSource* getRemoteSource(std::string remoteSourceName);
    std::string getModuleRepo(std::string moduleName);
    std::vector<std::string> getRepoModuleIds(std::string repoName);
    std::vector<std::string> getAllRepoModuleIds();
    std::string getModuleIdFromFile(std::string moduleFileName);
    int refreshIndividualRemoteSource(std::string remoteSourceName);
    std::thread getRemoteSourceRefreshThread(std::string remoteSourceName);
    void resetMgr();
    void initStrongs();

    sword::SWModule* getModuleFromList(std::vector<sword::SWModule*>& moduleList, std::string moduleName);

    std::string getCurrentVerseText(sword::SWModule* module, bool hasStrongs, bool forceNoMarkup=false);
    std::string getCurrentChapterHeading(sword::SWModule* module);
    std::string getFilteredText(const std::string& text, bool hasStrongs=false);
    std::string replaceSpacesInStrongs(const std::string& text);
    bool moduleHasStrongsZeroPrefixes(sword::SWModule* module);

    sword::SWMgr* _mgr = 0;
    sword::SWMgr* _mgrForInstall = 0;
    sword::InstallMgr* _installMgr = 0;
    sword::LocaleMgr* _localeMgr = 0;
    SwordStatusReporter* _statusReporter = 0;
    FileSystemHelper _fileSystemHelper;

    sword::SWModule* _strongsHebrew = 0;
    sword::SWModule* _strongsGreek = 0;

    bool _markupEnabled = false;
};

#endif // _SWORD_FACADE

