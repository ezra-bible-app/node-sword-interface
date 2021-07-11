/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2021 Tobias Klein <contact@tklein.info>

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

#ifndef _REPOSITORY_INTERFACE
#define _REPOSITORY_INTERFACE

#include <vector>
#include <string>
#include <future>

#include "common_defs.hpp"
#include "file_system_helper.hpp"
#include "module_helper.hpp"
#include "sword_status_reporter.hpp"
#include "installmgr.h"

namespace sword {
    class InstallSource;
    class SWModule;
    class SWMGr;
};

class ModuleHelper;

class RepositoryInterface {
public:
    RepositoryInterface(SwordStatusReporter& statusReporter, ModuleHelper& moduleHelper, std::string customHomeDir="");
    virtual ~RepositoryInterface(){}

    void resetMgr();

    int refreshRepositoryConfig();
    int refreshRemoteSources(bool force=false, std::function<void(unsigned int progress)>* progressCallback=0);

    std::vector<std::string> getRepoNames();
    sword::SWModule* getRepoModule(std::string moduleName, std::string repoName="all");
    std::vector<sword::SWModule*> getAllRepoModules(std::string repoName, ModuleType moduleType=ModuleType::bible);
    std::vector<sword::SWModule*> getRepoModulesByLang(std::string repoName,  
                                                       std::string languageCode,
                                                       ModuleType moduleType=ModuleType::bible,
                                                       bool headersFilter=false,
                                                       bool strongsFilter=false,
                                                       bool hebrewStrongsKeys=false,
                                                       bool greekStrongsKeys=false);
    unsigned int getRepoModuleCount(std::string repoName, ModuleType moduleType=ModuleType::bible);
    std::vector<std::string> getRepoLanguages(std::string repoName, ModuleType moduleType=ModuleType::bible);
    unsigned int getRepoLanguageModuleCount(std::string repoName, std::string languageCode, ModuleType moduleType=ModuleType::bible);
    bool isModuleAvailableInRepo(std::string moduleName, std::string repoName="all");
    std::string getModuleRepo(std::string moduleName);
    sword::InstallSource* getRemoteSource(std::string remoteSourceName);

    sword::InstallMgr* getInstallMgr();
    
    SwordStatusReporter& getStatusReporter() {
        return this->_statusReporter;
    }

    static std::string getModuleTypeString(ModuleType moduleType)
    {
        std::string moduleTypeFilter = "";

        switch (moduleType) {
            case ModuleType::bible:
                moduleTypeFilter = "Biblical Texts";
                break;
            case ModuleType::dict:
                moduleTypeFilter = "Lexicons / Dictionaries";
                break;
            default:
                moduleTypeFilter = "ANY";
        }

        return moduleTypeFilter;
    }

private:
    int refreshIndividualRemoteSource(std::string remoteSourceName, std::function<void(unsigned int progress)>* progressCallback=0);
    std::future<int> getRemoteSourceRefreshFuture(std::string remoteSourceName, std::function<void(unsigned int progress)>* progressCallback=0);

    int getRepoCount();
    std::vector<std::string> getRepoModuleIds(std::string repoName);
    std::vector<std::string> getAllRepoModuleIds();
    std::string getModuleIdFromLine(std::string line);
    std::string getModuleIdFromFile(std::string moduleFileName);
    sword::SWModule* getModuleFromList(std::vector<sword::SWModule*>& moduleList, std::string moduleName);

    unsigned int _remoteSourceCount = 0;
    unsigned int _remoteSourceUpdateCount = 0;
    sword::InstallMgr* _installMgr = 0;
    SwordStatusReporter& _statusReporter;
    FileSystemHelper _fileSystemHelper;
    ModuleHelper& _moduleHelper;
};

#endif // _REPOSITORY_INTERFACE