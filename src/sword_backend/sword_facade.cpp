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
}
