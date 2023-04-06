/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2023 Tobias Klein <contact@tklein.info>

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

#ifndef _MODULE_STORE
#define _MODULE_STORE

#include <string>

#include "common_defs.hpp"
#include "file_system_helper.hpp"

namespace sword {
    class SWModule;
    class SWMgr;
};

class ModuleStore
{
public:
    ModuleStore(std::string customHomeDir="");
    virtual ~ModuleStore();

    sword::SWMgr* createSWMgr();
    sword::SWModule* getLocalModule(std::string moduleName);
    std::vector<sword::SWModule*> getAllLocalModules(ModuleType moduleType=ModuleType::bible);
    
    bool isModuleInUserDir(std::string moduleName);
    bool isModuleInUserDir(sword::SWModule* module);
    std::string getModuleDataPath(sword::SWModule* module);

    void refreshMgr();
    void deleteModule(std::string moduleName);

    sword::SWMgr* getSwMgr();
    sword::SWMgr* getSearchSwMgr();
    
private:
    std::string customHomeDir;
    std::vector<std::string> getModuleLanguages(ModuleType moduleType=ModuleType::bible);
    sword::SWMgr* _mgr = 0;
    sword::SWMgr* _searchMgr = 0;
    FileSystemHelper _fileSystemHelper;
};

#endif // _MODULE_STORE