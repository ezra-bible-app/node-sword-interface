/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2024 Tobias Klein <contact@tklein.info>

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

#ifndef _SWORD_TRANSLATION_HELPER
#define _SWORD_TRANSLATION_HELPER

#include <string>
#include <sstream>

#include <localemgr.h>
#include <swlocale.h>
#include <swkey.h>
#include <versekey.h>
#include <swmodule.h>

class SwordTranslationHelper
{
public:
    SwordTranslationHelper(std::string localeDir) {
        this->_localeMgr = new sword::LocaleMgr(localeDir.c_str());
        sword::LocaleMgr::setSystemLocaleMgr(this->_localeMgr);
    }

    virtual ~SwordTranslationHelper() {}

    inline std::string getSwordTranslation(std::string originalString, std::string localeCode)
    {
        std::string translation = std::string(this->_localeMgr->translate(originalString.c_str(), localeCode.c_str()));
        return translation;
    }

    inline std::string getBookAbbreviation(sword::SWModule* module, std::string book, std::string localeCode) {
        sword::VerseKey *vk = (sword::VerseKey *)module->getKey();
        vk->setLocale(localeCode.c_str());
        vk->setText(book.c_str());
        std::string translatedAbbreviation = vk->getBookAbbrev();

        return translatedAbbreviation;
    }

private:
    sword::LocaleMgr* _localeMgr = 0;
};


#endif // _SWORD_TRANSLATION_HELPER