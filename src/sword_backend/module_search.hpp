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

#ifndef _MODULE_SEARCH
#define _MODULE_SEARCH

#include <functional>
#include <map>

#include "verse.hpp"

void setModuleSearchProgressCB(std::function<void(char, void*)>* moduleSearchProgressCB);
void internalModuleSearchProgressCB(char percent, void* userData);

namespace sword {
    class SWModule;
};

enum class SearchType {
    phrase = -1,
    multiWord = -2,
    strongsNumber = -3
};

class ModuleSearch
{
public:
    ModuleSearch() {}
    virtual ~ModuleSearch() {}

    std::vector<Verse> getModuleSearchResults(std::string moduleName,
                                              std::string searchTerm,
                                              SearchType searchType=SearchType::multiWord,
                                              bool isCaseSensitive=false);

private:
    std::map<std::string, int> getAbsoluteVerseNumberMap(sword::SWModule* module);
};

#endif // _MODULE_SEARCH