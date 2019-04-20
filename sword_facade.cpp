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

#include <stdlib.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include <algorithm>
#include <future>
#include <regex>

#include <installmgr.h>
#include <swmodule.h>
#include <swmgr.h>
#include <remotetrans.h>

#include "sword_facade.hpp"

using namespace std;
using namespace sword;

void SwordStatusReporter::update(unsigned long totalBytes, unsigned long completedBytes)
{
    /*int p = (totalBytes > 0) ? (int)(74.0 * ((double)completedBytes / (double)totalBytes)) : 0;
    for (;last < p; ++last) {
        if (!last) {
            SWBuf output;
            output.setFormatted("[ File Bytes: %ld", totalBytes);
            while (output.size() < 75) output += " ";
            output += "]";
            cout << output.c_str() << "\n ";
        }
        cout << "-";
    }
    cout.flush();*/
}

void SwordStatusReporter::preStatus(long totalBytes, long completedBytes, const char *message)
{
    /*SWBuf output;
    output.setFormatted("[ Total Bytes: %ld; Completed Bytes: %ld", totalBytes, completedBytes);
    while (output.size() < 75) output += " ";
    output += "]";
    cout << "\n" << output.c_str() << "\n ";
    int p = (int)(74.0 * (double)completedBytes/totalBytes);
    for (int i = 0; i < p; ++i) { cout << "="; }
    cout << "\n\n" << message << "\n";
    last = 0;*/

    cout << "\n" << message << "\n";
}

SwordFacade::SwordFacade()
{
    this->_fileSystemHelper.createBasicDirectories();
    this->_swConfig = new SWConfig(this->_fileSystemHelper.getSwordConfPath().c_str());

    if (!this->_fileSystemHelper.isSwordConfExisting()) {
        this->_swConfig->Sections.clear();
        (*this->_swConfig)["Install"].insert(std::make_pair(SWBuf("DataPath"), this->_fileSystemHelper.getModuleDir().c_str()));
        this->_swConfig->Save();
    }

    this->_mgr = new SWMgr();
    this->_statusReporter = new SwordStatusReporter();
    this->_installMgr = new InstallMgr(this->_fileSystemHelper.getInstallMgrDir().c_str(), this->_statusReporter);
    this->_installMgr->setUserDisclaimerConfirmed(true);
}

SwordFacade::~SwordFacade()
{
    delete this->_mgr;
    delete this->_installMgr;
    delete this->_swConfig;
    delete this->_statusReporter;
}

int SwordFacade::refreshRepositoryConfig()
{
    cout << "Refreshing repository configuration ... ";

    int ret = this->_installMgr->refreshRemoteSourceConfiguration();
    if (ret != 0) {
        cout << endl << "refreshRemoteSourceConfiguration returned " << ret << endl;
        return ret;
    }

    this->_installMgr->saveInstallConf();
    cout << "done." << endl;
    return 0;
}

void SwordFacade::refreshRemoteSources(bool force)
{
    vector<thread> refreshThreads;

    if (this->getRepoNames().size() == 0 || force) {
        this->refreshRepositoryConfig();
        vector<string> sourceNames = this->getRepoNames();

        // Create worker threads
        for (unsigned int i = 0; i < sourceNames.size(); i++) {
            refreshThreads.push_back(this->getRemoteSourceRefreshThread(sourceNames[i]));
        }

        // Wait for threads to finish
        for (unsigned int i = 0; i < refreshThreads.size(); i++) {
            refreshThreads[i].join();
        }
    }
}

int SwordFacade::refreshIndividualRemoteSource(string remoteSourceName)
{
    //cout << "Refreshing source " << remoteSourceName << endl << flush;
    InstallSource* source = this->getRemoteSource(remoteSourceName);
    int result = this->_installMgr->refreshRemoteSource(source);
    if (result != 0) {
        cout << "Failed to refresh source " << remoteSourceName << endl << flush;
    }

    return result;
}

thread SwordFacade::getRemoteSourceRefreshThread(std::string remoteSourceName)
{
    return thread(&SwordFacade::refreshIndividualRemoteSource, this, remoteSourceName);
}

vector<string> SwordFacade::getRepoNames()
{
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
        cout << "Could not find remote source " << remoteSourceName << endl;
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

SWModule* SwordFacade::getRepoModule(string moduleName)
{
    vector<SWModule*> allModules = this->getAllRemoteModules();
    for (unsigned int i = 0; i < allModules.size(); i++) {
      SWModule* currentModule = allModules[i];
      if (string(currentModule->getName()) == moduleName) {
          return currentModule;
      }
    }

    return 0;
}

vector<SWModule*> SwordFacade::getAllRepoModules(string repoName)
{
    vector<SWModule*> modules;
    InstallSource* remoteSource = this->getRemoteSource(repoName);

    if (remoteSource != 0) {
        SWMgr* mgr = remoteSource->getMgr();

        std::map<SWModule *, int> mods = InstallMgr::getModuleStatus(*mgr, *mgr);
        for (std::map<SWModule *, int>::iterator it = mods.begin(); it != mods.end(); it++) {
            SWModule* currentModule = it->first;
            string moduleType = string(currentModule->getType());

            if (moduleType == string("Biblical Texts")) {
              modules.push_back(currentModule);
            }
        }
    }

    return modules;
}

vector<SWModule*> SwordFacade::getRepoModulesByLang(string repoName, string languageCode)
{
    vector<SWModule*> allModules = this->getAllRepoModules(repoName);
    vector<SWModule*> selectedLanguageModules;

    for (unsigned int i = 0; i < allModules.size(); i++) {
      SWModule* currentModule = allModules[i];
      if ((currentModule->getType() == string("Biblical Texts")) && (currentModule->getLanguage() == languageCode)) {
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

unsigned int SwordFacade::getRepoLanguageTranslationCount(std::string repoName, std::string languageCode)
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
        vector<SWModule*> repoModules = this->getAllRepoModules(repo);

        for (unsigned int j = 0; j < repoModules.size(); j++) {
            SWModule* module = repoModules[j];
            if (string(module->getName()) == moduleName) {
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
        allLocalModules.push_back(currentModule);
    }

    return allLocalModules;
}

SWModule* SwordFacade::getLocalModule(string moduleName)
{
    return this->_mgr->getModule(moduleName.c_str());
}

std::string SwordFacade::rtrim(const std::string& s)
{
    static const std::string WHITESPACE = " \n\r\t\f\v";
	  size_t end = s.find_last_not_of(WHITESPACE);
	  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
} 

vector<string> SwordFacade::getBibleText(std::string moduleName)
{
    SWModule* module = this->getLocalModule(moduleName);

    vector<string> bibleText;
    char key[255];
    memset(key, 0, sizeof(key));
    regex markupFilterRegex = regex("<H.*> ");

    if (module == 0) {
      cout << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        module->setKey("Gen 1:1");
        for (;;) {
            stringstream currentVerse;

            // Stop, once the newly read key is the same as the previously read key
            if (strcmp(module->getKey()->getShortText(), key) == 0) {
                break;
            }

            string rawVerseText = rtrim(string(module->stripText()));
            string filteredText = regex_replace(rawVerseText, markupFilterRegex, "");

            if (filteredText.length() > 0) {
              currentVerse << module->getKey()->getShortText() << "|" << filteredText;
              bibleText.push_back(currentVerse.str());
            }

            strcpy(key, module->getKey()->getShortText());
            module->increment();
        }
    }

    return bibleText;
}

int SwordFacade::installModule(string moduleName)
{
    string repoName = this->getModuleRepo(moduleName);

    if (repoName == "") {
        cout << "Could not find repository for module " << moduleName << endl;
        return -1;
    }

    return this->installModule(repoName, moduleName);
}

int SwordFacade::installModule(string repoName, string moduleName)
{
    InstallSource* remoteSource = this->getRemoteSource(repoName);
    if (remoteSource == 0) {
        cout << "Couldn't find remote source " << repoName << endl;
        return -1;
    }

    SWMgr *remoteMgr = remoteSource->getMgr();
    SWModule *module;
    ModMap::iterator it = remoteMgr->Modules.find(moduleName.c_str());

    if (it == remoteMgr->Modules.end()) {
        cout << "Did not find module " << moduleName << " in repository " << repoName << endl;
        return -1;
    } else {
        module = it->second;

        int error = this->_installMgr->installModule(this->_mgr, 0, module->getName(), remoteSource);
        if (error) {
            cout << "Error installing module: [" << module->getName() << "] (write permissions?)\n";
            return -1;
        } else {
            // Reload the library of SWORD modules after making this change
            this->_mgr->Load();
            cout << "Installed module: [" << module->getName() << "]\n";
            return 0;
        }
    }
}

int SwordFacade::uninstallModule(string moduleName)
{
    int error = this->_installMgr->removeModule(this->_mgr, moduleName.c_str());

    if (error) {
        cout << "Error uninstalling module: [" << moduleName << "] (write permissions?)\n";
        return -1;
    } else {
        // Reload the library of SWORD modules after making this change
        this->_mgr->Load();
        cout << "Uninstalled module: [" << moduleName << "]\n";
        return 0;
    }
}

