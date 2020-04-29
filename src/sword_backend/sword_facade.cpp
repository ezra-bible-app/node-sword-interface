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
#include "module_helper.hpp"
#include "string_helper.hpp"
#include "strongs_entry.hpp"
#include "percentage_calc.hpp"
#include "common_defs.hpp"

using namespace std;
using namespace sword;

#if defined(_WIN32)
// For some reason this symbol is missing in the sword.dll on Windows, hence we include it here.
char * sword::SWBuf::nullStr = (char *)"";
#endif

SwordFacade::SwordFacade(SwordStatusReporter& statusReporter, ModuleHelper& moduleHelper)
    : _statusReporter(statusReporter), _moduleHelper(moduleHelper)
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
        this->_strongsHebrew = this->getLocalSwordModule("StrongsHebrew");
        this->_strongsGreek = this->getLocalSwordModule("StrongsGreek");
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

#ifdef _WIN32
    this->_mgr = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str());
    this->_mgr->augmentModules(this->_fileSystemHelper.getSystemSwordDir().c_str());
#else
    this->_mgr = new SWMgr();
#endif

    this->_mgrForInstall = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str());

    this->initStrongs();
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
    string repoName = this->_repoInterface->getModuleRepo(moduleName);

    if (repoName == "") {
        cerr << "Could not find repository for module " << moduleName << endl;
        return -1;
    }

    return this->installModule(repoName, moduleName);
}

int SwordFacade::installModule(string repoName, string moduleName)
{
    InstallSource* remoteSource = this->_repoInterface->getRemoteSource(repoName);
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
        int error = this->_repoInterface->getInstallMgr()->installModule(this->_mgrForInstall, 0, moduleName.c_str(), remoteSource);
        this->resetMgr();

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
    int error = this->_repoInterface->getInstallMgr()->removeModule(this->_mgrForInstall, moduleName.c_str());
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
                this->_repoInterface->refreshRemoteSources(true);
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

SWModule* SwordFacade::getLocalSwordModule(string moduleName)
{
    return this->_mgr->getModule(moduleName.c_str());
}

string SwordFacade::getSwordVersion()
{
    return string("1.8.900-d5030c1");
}
