/* This file is part of node-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@ezra-project.net>

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

using namespace std;

namespace sword {
    class SWModule;
}

class SwordFacade;

class NapiSwordHelper {
public:
    NapiSwordHelper();
    virtual ~NapiSwordHelper();

    Napi::Array getNapiVerseObjectsFromRawList(const Napi::Env& env, std::string& moduleCode, std::vector<std::string>& verses);
    void swordModuleToNapiObject(const Napi::Env& env, sword::SWModule* swModule, Napi::Object& object);

private:
    void verseTextToNapiObject(std::string& moduleCode, std::string& rawVerse, unsigned int absoluteVerseNr, Napi::Object& object);
    std::vector<std::string> split(const std::string& s, char separator);

    SwordFacade* _swordFacade;
};

#endif // _NAPI_SWORD_HELPER