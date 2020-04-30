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

#ifndef _SWORD_FACADE
#define _SWORD_FACADE

#include <vector>
#include <map>
#include <string>
#include <climits>
#include <functional>

#include <remotetrans.h>

#include "file_system_helper.hpp"
#include "module_helper.hpp"
#include "strongs_entry.hpp"
#include "repository_interface.hpp"
#include "common_defs.hpp"

namespace sword {
    class InstallMgr;
    class InstallSource;
    class SWModule;
    class SWKey;
    class SWMgr;
    class SWConfig;
    class LocaleMgr;
};

class SwordStatusReporter;
class ModuleHelper;
class Verse;

class SwordFacade
{
public:
    SwordFacade(SwordStatusReporter& statusReporter, ModuleHelper& moduleHelper);
    virtual ~SwordFacade();

    std::string getSwordTranslation(std::string configPath, std::string originalString, std::string localeCode);
    std::string getSwordVersion();

    SwordStatusReporter& getStatusReporter() {
        return this->_statusReporter;
    }

private:
    void resetMgr();
    void initStrongs();
    sword::SWModule* getLocalSwordModule(std::string moduleName);

    RepositoryInterface* _repoInterface = 0;

    sword::SWMgr* _mgr = 0;
    sword::SWMgr* _mgrForInstall = 0;
    sword::LocaleMgr* _localeMgr = 0;

    SwordStatusReporter& _statusReporter;
    FileSystemHelper _fileSystemHelper;
    ModuleHelper& _moduleHelper;

    sword::SWModule* _strongsHebrew = 0;
    sword::SWModule* _strongsGreek = 0;

    bool _markupEnabled = false;
};

#endif // _SWORD_FACADE

