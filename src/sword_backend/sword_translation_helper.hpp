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

#ifndef _SWORD_TRANSLATION_HELPER
#define _SWORD_TRANSLATION_HELPER

#include <string>
#include <localemgr.h>

class SwordTranslationHelper
{
public:
    inline std::string getSwordTranslation(std::string configPath, std::string originalString, std::string localeCode)
    {
        // We only initialize this at the first execution
        if (this->_localeMgr == 0) {
            this->_localeMgr = new sword::LocaleMgr(configPath.c_str());
        }
        
        std::string translation = std::string(this->_localeMgr->translate(originalString.c_str(), localeCode.c_str()));
        return translation;
    }

private:
    sword::LocaleMgr* _localeMgr = 0;
};


#endif // _SWORD_TRANSLATION_HELPER