/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2025 Tobias Klein <contact@tklein.info>

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

// Std includes
#include <regex>
#include <string>
#include <sstream>

// Sword includes
#include <swmgr.h>
#include <markupfiltmgr.h>
#include <swmodule.h>

// Own includes
#include "module_store.hpp"
#include "repository_interface.hpp"

using namespace std;
using namespace sword;

ModuleStore::ModuleStore(string customHomeDir)
{
    this->_fileSystemHelper.setCustomHomeDir(customHomeDir);
    this->_fileSystemHelper.createBasicDirectories();
    this->customHomeDir = customHomeDir;

    this->_mgr = this->createSWMgr();
    this->_mgr->setGlobalOption("Headings", "On");

    // After creating the searchMgr we turn off features that we are not interested in when searching
    this->_searchMgr = this->createSWMgr();
    this->_searchMgr->setGlobalOption("Headings", "Off");
}

ModuleStore::~ModuleStore()
{
    if (this->_mgr != 0) {
        delete this->_mgr;
    }

    if (this->_searchMgr != 0) {
        delete this->_searchMgr;
    }
}

SWMgr* ModuleStore::createSWMgr()
{
    SWMgr* swMgr = 0;
    bool isAndroid = false;
    #if defined(__ANDROID__)
        isAndroid = true;
    #endif

    MarkupFilterMgr* markupFilterMgr = new MarkupFilterMgr(sword::FMT_OSIS, sword::ENC_UTF8);

    if (customHomeDir != "" || isAndroid) {
        swMgr = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str(),
                          true, // autoload
                          markupFilterMgr,
                          false, // multiMod
                          false); // augmentHome

        if (isAndroid) {
          // Also consider the originally used path for Android, which does not work anymore from Android 11, but is still relevant
          // for existing translations on Android versions < 11.
          swMgr->augmentModules("/sdcard/sword");

          // Also consider /sdcard/Documents/sword, which is the path used by other programs
          swMgr->augmentModules("/sdcard/Documents/sword");
        }
    } else {
        #ifdef _WIN32
            swMgr = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str(),
                              true, // autoload
                              markupFilterMgr,
                              false, // multimod
                              true); // augmentHome

            // This has been disabled because it lead to a crash.
            // We're keeping it here for now in case this becomes relevant again.
            // this->_mgr->augmentModules(this->_fileSystemHelper.getSystemSwordDir().c_str());
        #elif defined(__APPLE__)
            swMgr = new SWMgr(markupFilterMgr);

            stringstream appSupport;
            appSupport << string(getenv("HOME")) << "/Library/Application Support/Sword";            
            swMgr->augmentModules(appSupport.str().c_str());
        #else // LINUX!
            swMgr = new SWMgr(markupFilterMgr);
        #endif
    }

    return swMgr;
}

void ModuleStore::refreshMgr()
{
    this->_mgr->augmentModules(this->_fileSystemHelper.getUserSwordDir().c_str());
    this->_searchMgr->augmentModules(this->_fileSystemHelper.getUserSwordDir().c_str());
}

void ModuleStore::deleteModule(string moduleName)
{
    this->_mgr->deleteModule(moduleName.c_str());
    this->_searchMgr->deleteModule(moduleName.c_str());
}

SWModule* ModuleStore::getLocalModule(string moduleName)
{
    return this->_mgr->getModule(moduleName.c_str());
}

vector<string> ModuleStore::getModuleLanguages(ModuleType moduleType)
{
    vector<string> languages;
    string moduleTypeFilter = RepositoryInterface::getModuleTypeString(moduleType);

    for (ModMap::iterator modIterator = this->_mgr->Modules.begin();
         modIterator != this->_mgr->Modules.end();
         modIterator++) {
        
        SWModule* currentModule = (SWModule*)modIterator->second;
        string moduleType = string(currentModule->getType());
        string moduleLanguage = string(currentModule->getLanguage());

        if (moduleTypeFilter == "ANY" || moduleType == moduleTypeFilter) {

            if (std::find(languages.begin(),
                          languages.end(),                       // Only accept add the language if we do not
                          moduleLanguage) == languages.end()) {  // have it yet!

                languages.push_back(moduleLanguage);
            }
        }
    }

    return languages;
}

vector<SWModule*> ModuleStore::getAllLocalModules(ModuleType moduleType)
{
    vector<SWModule*> allLocalModules;
    string moduleTypeFilter = RepositoryInterface::getModuleTypeString(moduleType);
    vector<string> languages = this->getModuleLanguages(moduleType);

    for (unsigned int i = 0; i < languages.size(); i++) {
        string currentLanguage = languages[i];

        for (ModMap::iterator modIterator = this->_mgr->Modules.begin();
            modIterator != this->_mgr->Modules.end();
            modIterator++) {
            
            SWModule* currentModule = (SWModule*)modIterator->second;
            string moduleType = string(currentModule->getType());
            string moduleLanguage = string(currentModule->getLanguage());

            if ((moduleTypeFilter == "ANY" || moduleType == moduleTypeFilter) && (moduleLanguage == currentLanguage)) {
                allLocalModules.push_back(currentModule);
            }
        }
    }

    return allLocalModules;
}

bool ModuleStore::isModuleInUserDir(sword::SWModule* module)
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

bool ModuleStore::isModuleInUserDir(string moduleName)
{
    SWModule* module = this->getLocalModule(moduleName);
    return this->isModuleInUserDir(module);
}

string ModuleStore::getModuleDataPath(sword::SWModule* module)
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

SWMgr* ModuleStore::getSwMgr()
{
    return this->_mgr;
}

SWMgr* ModuleStore::getSearchSwMgr()
{
    return this->_searchMgr;
}