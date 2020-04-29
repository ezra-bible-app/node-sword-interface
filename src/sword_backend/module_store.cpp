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