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

// std includes
#include <iostream>

// Sword includes
#include <swmgr.h>
#include <installmgr.h>

// Own includes
#include "repository_interface.hpp"
#include "module_installer.hpp"
#include "module_store.hpp"

using namespace std;
using namespace sword;

ModuleInstaller::ModuleInstaller(RepositoryInterface& repoInterface, ModuleStore& moduleStore, string customHomeDir)
    : _repoInterface(repoInterface), _moduleStore(moduleStore)
{
    this->_fileSystemHelper.setCustomHomeDir(customHomeDir);
    this->_mgrForInstall = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str());
}

ModuleInstaller::~ModuleInstaller()
{
    if (this->_mgrForInstall != 0) {
        delete this->_mgrForInstall;
    }
}

void ModuleInstaller::refreshMgr()
{
    this->_mgrForInstall->augmentModules(this->_fileSystemHelper.getUserSwordDir().c_str());
}

void ModuleInstaller::resetAllMgrs()
{
    this->_repoInterface.resetMgr();
    this->_moduleStore.refreshMgr();
    this->refreshMgr();
}

int ModuleInstaller::installModule(string moduleName)
{
    string repoName = this->_repoInterface.getModuleRepo(moduleName);

    if (repoName == "") {
        cerr << "Could not find repository for module " << moduleName << endl;
        return -1;
    }

    return this->installModule(repoName, moduleName);
}

int ModuleInstaller::installModule(string repoName, string moduleName)
{
    InstallSource* remoteSource = this->_repoInterface.getRemoteSource(repoName);
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
        int error = this->_repoInterface.getInstallMgr()->installModule(this->_mgrForInstall, 0, moduleName.c_str(), remoteSource);
        this->resetAllMgrs();

        if (error) {
            cerr << "Error installing module: " << moduleName << " (write permissions?)" << endl;
            return -1;
        } else {
            cout << "Installed module: " << moduleName << endl;
            return 0;
        }
    }
}

int ModuleInstaller::uninstallModule(string moduleName)
{
    int error = this->_repoInterface.getInstallMgr()->removeModule(this->_mgrForInstall, moduleName.c_str());
    this->_mgrForInstall->deleteModule(moduleName.c_str());
    this->_moduleStore.deleteModule(moduleName);

    if (error) {
        cerr << "Error uninstalling module: " << moduleName << " (write permissions?)" << endl;
        return -1;
    } else {
        cout << "Uninstalled module: " << moduleName << endl;
        return 0;
    }
}

int ModuleInstaller::saveModuleUnlockKey(string moduleName, string key)
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
                this->resetAllMgrs();
                // Without this step we cannot load a remote module afterwards ...
                this->_repoInterface.refreshRemoteSources(true);
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