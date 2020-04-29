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

#ifndef _NAPI_SWORD_HELPER
#define _NAPI_SWORD_HELPER

#include <napi.h>
#include <iostream>

#include "strongs_entry.hpp"
#include "common_defs.hpp"

using namespace std;

namespace sword {
    class SWModule;
}

class SwordFacade;
class ModuleHelper;
class ModuleStore;

class NapiSwordHelper {
public:
    NapiSwordHelper(SwordFacade& swordFacade, ModuleHelper& moduleHelper, ModuleStore& moduleStore)
        : _swordFacade(swordFacade), _moduleHelper(moduleHelper), _moduleStore(moduleStore) {}

    virtual ~NapiSwordHelper() {}

    Napi::Array getNapiArrayFromStringVector(const Napi::Env& env, std::vector<std::string>& stringVector);
    Napi::Array getNapiVerseObjectsFromRawList(const Napi::Env& env, std::string moduleCode, std::vector<Verse>& verses);
    void swordModuleToNapiObject(const Napi::Env& env, sword::SWModule* swModule, Napi::Object& object);
    void strongsEntryToNapiObject(const Napi::Env& env, StrongsEntry* strongsEntry, Napi::Object& object);

private:
    void verseTextToNapiObject(std::string moduleCode, Verse rawVerse, Napi::Object& object);
    Napi::String getConfigEntry(sword::SWModule* swModule, std::string key, const Napi::Env& env);

    SwordFacade& _swordFacade;
    ModuleHelper& _moduleHelper;
    ModuleStore& _moduleStore;
};

#endif // _NAPI_SWORD_HELPER
