/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>

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

// STD C++ includes
#include <iostream>
#include <sstream>

#ifdef __ANDROID__
#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>
#else
#include <fstream>
#endif

#include <regex>

// Own includes
#include "repository_interface.hpp"
#include "sword_status_reporter.hpp"
#include "percentage_calc.hpp"
#include "string_helper.hpp"
#include "module_helper.hpp"
#include "mutex.hpp"

// Sword includes
#include <installmgr.h>
#include <swmodule.h>
#include <swmgr.h>
#include <remotetrans.h>

using namespace std;
using namespace sword;

static Mutex remoteSourceUpdateMutex;

RepositoryInterface::RepositoryInterface(SwordStatusReporter& statusReporter, ModuleHelper& moduleHelper, string customHomeDir) 
    : _statusReporter(statusReporter), _moduleHelper(moduleHelper)
{
    this->_fileSystemHelper.setCustomHomeDir(customHomeDir);
    this->resetMgr();
    remoteSourceUpdateMutex.init();
}

void RepositoryInterface::resetMgr()
{
    if (this->_installMgr != 0) {
        delete this->_installMgr;
    }

    cout << "Initializing InstallMgr at " << this->_fileSystemHelper.getInstallMgrDir() << endl;

    this->_installMgr = new InstallMgr(this->_fileSystemHelper.getInstallMgrDir().c_str(), &this->_statusReporter);
    this->_installMgr->setUserDisclaimerConfirmed(true);
}

int RepositoryInterface::refreshRepositoryConfig()
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

int RepositoryInterface::refreshRemoteSources(bool force, std::function<void(unsigned int progress)>* progressCallback)
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

int RepositoryInterface::refreshIndividualRemoteSource(string remoteSourceName, std::function<void(unsigned int progress)>* progressCallback)
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

thread RepositoryInterface::getRemoteSourceRefreshThread(string remoteSourceName, std::function<void(unsigned int progress)>* progressCallback)
{
    return thread(&RepositoryInterface::refreshIndividualRemoteSource, this, remoteSourceName, progressCallback);
}

InstallSource* RepositoryInterface::getRemoteSource(string remoteSourceName)
{
    InstallSourceMap::iterator source = this->_installMgr->sources.find(remoteSourceName.c_str());
    if (source == this->_installMgr->sources.end()) {
        cerr << "getRemoteSource: Could not find remote source '" << remoteSourceName << "'" << endl;
    } else {
        return source->second;
    }

    return 0;
}

int RepositoryInterface::getRepoCount()
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

vector<string> RepositoryInterface::getRepoNames()
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

vector<SWModule*> RepositoryInterface::getAllRepoModules(string repoName, ModuleType moduleType)
{
    vector<SWModule*> modules;
    InstallSource* remoteSource = this->getRemoteSource(repoName);
    string moduleTypeString = RepositoryInterface::getModuleTypeString(moduleType);

    if (remoteSource != 0) {
        SWMgr* mgr = remoteSource->getMgr();

        for (ModMap::const_iterator it = mgr->Modules.begin(); it != mgr->Modules.end(); it++) {
            SWModule* currentModule = it->second;
            string currentModuleType = currentModule->getType();

            if (moduleType == ModuleType::dict && !this->_moduleHelper.moduleHasStrongsKeys(currentModule)) {
                continue;
            }

            if (moduleTypeString == "ANY" || currentModuleType == moduleTypeString) {
                modules.push_back(currentModule);
            }
        }
    } else {
      cerr << "getAllRepoModules: Could not find remote source for repository '" << repoName << "'" << endl;
    }

    return modules;
}

SWModule* RepositoryInterface::getRepoModule(string moduleName, string repoName)
{    
    if (repoName == "all") {
        repoName = this->getModuleRepo(moduleName);
    }
    
    vector<SWModule*> modules = this->getAllRepoModules(repoName, ModuleType::any);
    return this->getModuleFromList(modules, moduleName);
}

vector<SWModule*> RepositoryInterface::getRepoModulesByLang(string repoName,
                                                            string languageCode,
                                                            ModuleType moduleType,
                                                            bool headersFilter,
                                                            bool strongsFilter,
                                                            bool hebrewStrongsKeys,
                                                            bool greekStrongsKeys)
{
    vector<SWModule*> allModules = this->getAllRepoModules(repoName, moduleType);
    vector<SWModule*> selectedLanguageModules;

    string moduleTypeFilter = RepositoryInterface::getModuleTypeString(moduleType);

    for (unsigned int i = 0; i < allModules.size(); i++) {
      SWModule* currentModule = allModules[i];

      if ((moduleTypeFilter == "ANY" || currentModule->getType() == moduleTypeFilter) &&
          (currentModule->getLanguage() == languageCode)) {

        bool hasHeadings = this->_moduleHelper.moduleHasGlobalOption(currentModule, "Headings");
        bool hasStrongs = this->_moduleHelper.moduleHasGlobalOption(currentModule, "Strongs");

        bool hasHebrewStrongsKeys = this->_moduleHelper.moduleHasFeature(currentModule, "HebrewDef");
        bool hasGreekStrongsKeys = this->_moduleHelper.moduleHasFeature(currentModule, "GreekDef");
        
        if (headersFilter && !hasHeadings) {
            continue;
        }

        if (strongsFilter && !hasStrongs) {
            continue;
        }

        if (moduleType == ModuleType::dict && !hasHebrewStrongsKeys && !hasGreekStrongsKeys) {
            // In case of a dictionary module we ignore it if there are not Strong's keys
            continue;
        }

        if (hebrewStrongsKeys && !hasHebrewStrongsKeys) {
            continue;
        }

        if (greekStrongsKeys && !hasGreekStrongsKeys) {
            continue;
        }

        selectedLanguageModules.push_back(currentModule);
      }
    }

    return selectedLanguageModules;
}

unsigned int RepositoryInterface::getRepoModuleCount(string repoName, ModuleType moduleType)
{
    vector<SWModule*> allModules = this->getAllRepoModules(repoName, moduleType);
    return (unsigned int)allModules.size();
}

unsigned int RepositoryInterface::getRepoLanguageModuleCount(string repoName, string languageCode, ModuleType moduleType)
{
    vector<SWModule*> allModules = this->getRepoModulesByLang(repoName, languageCode, moduleType);
    return (unsigned int)allModules.size();
}

vector<string> RepositoryInterface::getRepoLanguages(string repoName, ModuleType moduleType)
{
    vector<SWModule*> modules;
    vector<string> languages;

    modules = this->getAllRepoModules(repoName, moduleType);

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

SWModule* RepositoryInterface::getModuleFromList(vector<SWModule*>& moduleList, string moduleName)
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

    cerr << "getModuleFromList: Did not find module " << moduleName << " in modulelist!" << endl;

    return 0;
}

string RepositoryInterface::getModuleIdFromLine(string line)
{
    static regex parentheses = regex("[\\[\\]]");
    static regex lineBreaks = regex("[\\r\\n]");

    StringHelper::trim(line);

    string moduleId = "";
    char firstChar = line[0];
    char lastChar = line[line.size() - 1];

    if (firstChar == '[' && lastChar == ']') {
        // Remove parentheses and line breaks from the first line
        // What's left is the module id
        moduleId = regex_replace(line, parentheses, "");
        moduleId = regex_replace(moduleId, lineBreaks, "");
    }

    return moduleId;
}

#ifdef __ANDROID__
string RepositoryInterface::getModuleIdFromFile(string moduleFileName)
{
    string moduleId = "";
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read_count;

    fp = fopen(moduleFileName.c_str(), "r");

    if (fp != NULL) {
        while ((read_count = getline(&line, &len, fp)) != -1) {
          if (read_count >= 1) {
              string std_line = string(line);
              StringHelper::trim(std_line);
              char firstChar = std_line[0];

              if (firstChar == '[') {
                moduleId = this->getModuleIdFromLine(std_line);
                break;
              }
          }
        }

        fclose(fp);
        if (line) {
            free(line);
        }
    } else {
      cerr << "getModuleIdFromFile: Could not open file " << moduleFileName << " / errno: " << errno << endl;
    }

    return moduleId;
}
#else
string RepositoryInterface::getModuleIdFromFile(string moduleFileName)
{
    string moduleId = "";

    #if _WIN32
      wstring utf16ModuleFileName = this->_fileSystemHelper.convertUtf8StringToUtf16(moduleFileName);
      ifstream moduleFile(utf16ModuleFileName);
    #else
      ifstream moduleFile(moduleFileName);
    #endif

    if (moduleFile.is_open()) {
        string line;

        while (!moduleFile.eof() && !moduleFile.fail()) {
          std::getline(moduleFile, line);

          if (line.size() >= 1) {
            StringHelper::trim(line);
            char firstChar = line[0];

            if (firstChar == '[') {
              moduleId = this->getModuleIdFromLine(line);
              break;
            }
          }
        }
    }

    moduleFile.close();
    return moduleId;
}
#endif

vector<string> RepositoryInterface::getRepoModuleIds(string repoName)
{
    vector<string> moduleIds;
    InstallSource* remoteSource = this->getRemoteSource(repoName);
    FileSystemHelper fs;
    stringstream repoModuleDir;

    if (remoteSource != 0) {
        //cout << remoteSource->localShadow << endl;
        repoModuleDir << remoteSource->localShadow << fs.getPathSeparator() << "mods.d";

        //cout << "getRepoModuleIds: Looking for files in directory " << repoModuleDir.str() << endl;
        vector<string> filesInRepoDir = fs.getFilesInDir(repoModuleDir.str());

        if (filesInRepoDir.size() == 0) {
            cerr << "getRepoModuleIds: Got 0 files in directory " << repoModuleDir.str() << endl;

            return moduleIds;
        }

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
            } /* else {
                cerr << "getRepoModuleIds: Could not read module id from file " << moduleFileName.str() << endl;
            } */
        }
    } else {
      cerr << "getRepoModuleIds: Could not find remote source for repository '" << repoName << "'" << endl;
    }

    return moduleIds; 
}

vector<string> RepositoryInterface::getAllRepoModuleIds()
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

string RepositoryInterface::getModuleRepo(string moduleName)
{
    vector<string> repositories = this->getRepoNames();

    if (repositories.size() == 0) {
      cerr << "getModuleRepo: Got 0 repository names!" << endl;
      return "";
    }

    for (unsigned int i = 0; i < repositories.size(); i++) {
        string repo = repositories[i];
        vector<string> repoModuleIds = this->getRepoModuleIds(repo);

        if (repoModuleIds.size() == 0) {
          cerr << "getRepoModule: Got 0 repo module ids for repo " << repo << endl;
        }

        for (unsigned int j = 0; j < repoModuleIds.size(); j++) {
            string currentId = repoModuleIds[j];
            if (currentId == moduleName) {
                return repo;
            }
        }
    }

    cerr << "getModuleRepo: Could not find repository for module '" << moduleName << "'" << endl;
    return "";
}

bool RepositoryInterface::isModuleAvailableInRepo(string moduleName, string repoName)
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

sword::InstallMgr* RepositoryInterface::getInstallMgr()
{
    return this->_installMgr;
}