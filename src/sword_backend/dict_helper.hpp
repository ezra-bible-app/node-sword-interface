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

#ifndef _DICT_HELPER
#define _DICT_HELPER

#include <string>
#include <vector>

#include "module_store.hpp"

class DictHelper {
public:
    DictHelper(ModuleStore& moduleStore) : _moduleStore(moduleStore) {}

    std::vector<std::string> getKeyList(std::string moduleName);

    virtual ~DictHelper(){}

private:
    ModuleStore& _moduleStore;
};

#endif // _DICT_HELPER