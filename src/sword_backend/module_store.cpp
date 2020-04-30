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

// Std includes
#include <regex>
#include <string>

// Sword includes
#include <swmgr.h>
#include <swmodule.h>

// Own includes
#include "module_store.hpp"

using namespace std;
using namespace sword;

ModuleStore::ModuleStore()
{
    this->_fileSystemHelper.createBasicDirectories();
    this->resetMgr();
}

ModuleStore::~ModuleStore()
{
    if (this->_mgr != 0) {
        delete this->_mgr;
    }
}

void ModuleStore::resetMgr()
{
    if (this->_mgr != 0) {
        delete this->_mgr;
    }

    #ifdef _WIN32
        this->_mgr = new SWMgr(this->_fileSystemHelper.getUserSwordDir().c_str());
        this->_mgr->augmentModules(this->_fileSystemHelper.getSystemSwordDir().c_str());
    #else
        this->_mgr = new SWMgr();
    #endif
}

SWModule* ModuleStore::getLocalModule(string moduleName)
{
    return this->_mgr->getModule(moduleName.c_str());
}

vector<SWModule*> ModuleStore::getAllLocalModules()
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