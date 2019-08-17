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

namespace sword {
    class InstallMgr;
    class InstallSource;
    class SWModule;
    class SWMgr;
    class SWConfig;
};

class SwordStatusReporter : public sword::StatusReporter
{
public:
	int last;
    virtual void update(unsigned long totalBytes, unsigned long completedBytes);
    virtual void preStatus(long totalBytes, long completedBytes, const char *message);
};

class SwordFacade
{
public:
    SwordFacade();
    virtual ~SwordFacade();

    int refreshRepositoryConfig();
    void refreshRemoteSources(bool force=false);

    std::vector<std::string> getRepoNames();
    std::vector<sword::SWModule*> getAllRemoteModules();
    sword::SWModule* getRepoModule(std::string moduleName);
    std::vector<sword::SWModule*> getAllRepoModules(std::string repoName);
    std::vector<sword::SWModule*> getRepoModulesByLang(std::string repoName, std::string languageCode);
    unsigned int getRepoTranslationCount(std::string repoName);
    std::vector<std::string> getRepoLanguages(std::string repoName);
    unsigned int getRepoLanguageTranslationCount(std::string repoName, std::string languageCode);

    std::vector<sword::SWModule*> getAllLocalModules();
    sword::SWModule* getLocalModule(std::string moduleName);
    bool isModuleInUserDir(std::string moduleName);
    bool isModuleInUserDir(sword::SWModule* module);
    std::vector<std::string> getBibleText(std::string moduleName);
    std::vector<std::string> getBookText(std::string moduleName, std::string bookCode);

    int installModule(std::string moduleName);
    int installModule(std::string repoName, std::string moduleName);
    int uninstallModule(std::string moduleName);

    std::string getSwordVersion();

    void enableMarkup() { this->_markupEnabled = true; }

private:
    int getRepoCount();
    std::vector<std::string> getText(std::string moduleName, std::string key, bool onlyCurrentBook=true);
    sword::InstallSource* getRemoteSource(std::string remoteSourceName);
    std::string getModuleRepo(std::string moduleName);
    int refreshIndividualRemoteSource(std::string remoteSourceName);
    std::thread getRemoteSourceRefreshThread(std::string remoteSourceName);
    void resetMgr();

    std::string getFilteredVerseText(const std::string& verseText);
    std::string rtrim(const std::string& s);

    sword::SWMgr* _mgr = 0;
    sword::SWMgr* _mgrForInstall = 0;
    sword::InstallMgr* _installMgr = 0;
    SwordStatusReporter* _statusReporter = 0;
    FileSystemHelper _fileSystemHelper;

    bool _markupEnabled = false;
};

#endif // _SWORD_FACADE

