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

#ifndef _MODULE_INSTALLER
#define _MODULE_INSTALLER

#include <string>
#include "file_system_helper.hpp"
#include "string_helper.hpp"

namespace sword {
    class SWModule;
    class SWMgr;
};

class RepositoryInterface;
class ModuleStore;

class ModuleInstaller
{
public:
    ModuleInstaller(RepositoryInterface& repoInterface, ModuleStore& moduleStore);
    virtual ~ModuleInstaller();

    int installModule(std::string moduleName);
    int installModule(std::string repoName, std::string moduleName);
    int uninstallModule(std::string moduleName);

    int saveModuleUnlockKey(std::string moduleName, std::string key);
    bool isModuleReadable(sword::SWModule* module, std::string key="John 1:1");

private:
    void refreshMgr();
    void resetAllMgrs();

    RepositoryInterface& _repoInterface;
    ModuleStore& _moduleStore;
    FileSystemHelper _fileSystemHelper;
    StringHelper _stringHelper;

    sword::SWMgr* _mgrForInstall = 0;
};

#endif // _MODULE_INSTALLER