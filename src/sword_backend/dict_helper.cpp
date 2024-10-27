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

// Sword includes
#include <swmodule.h>
#include <swld.h>

#include "dict_helper.hpp"

using namespace std;
using namespace sword;

std::vector<std::string> DictHelper::getKeyList(std::string moduleName)
{
    vector<string> keyList;

    if (moduleName.size() == 0) {
        cerr << "getKeyList: Cannot work with empty moduleName!" << endl;
        return keyList;
    }

    SWModule* module = this->_moduleStore.getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;

    } else {
        const char* moduleType = module->getType();

        if (strcmp(moduleType, "Lexicons / Dictionaries") != 0) {

            cerr << "Given module is not a lexicon/dictionary!" << endl;

        } else {
            sword::SWLD* swldModule = static_cast<sword::SWLD*>(module);

            if (swldModule == NULL) {

                cerr << "Could not initialize module " << moduleName << " as SWLD module." << endl;

            } else {
                long entryCount = swldModule->getEntryCount();

                for (long i = 0; i < entryCount; i++) {
                    const char* key = swldModule->getKeyForEntry(i);
                    string stringKey = string(key);
                    keyList.push_back(stringKey);
                }
            }
        }
    }

    return keyList;
}
