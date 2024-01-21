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

#ifndef _MODULE_SEARCH
#define _MODULE_SEARCH

#include <functional>
#include <map>

#include "common_defs.hpp"

void setModuleSearchProgressCB(std::function<void(char, void*)>* moduleSearchProgressCB);
void internalModuleSearchProgressCB(char percent, void* userData);

namespace sword {
    class SWModule;
    class ListKey;
};

enum class SearchType {
    phrase = -1,
    multiWord = -2,
    strongsNumber = -3
};

enum class SearchScope {
    BIBLE = 0,
    OT = 1,
    NT = 2
};

class ModuleStore;
class ModuleHelper;
class TextProcessor;

class ModuleSearch
{
public:
    ModuleSearch(ModuleStore& moduleStore, ModuleHelper& moduleHelper, TextProcessor& textProcessor)
        : _moduleStore(moduleStore), _moduleHelper(moduleHelper), _textProcessor(textProcessor), _currentModuleName("") {}
    virtual ~ModuleSearch() {}

    std::vector<Verse> getModuleSearchResults(std::string moduleName,
                                              std::string searchTerm,
                                              SearchType searchType=SearchType::multiWord,
                                              SearchScope searchScope=SearchScope::BIBLE,
                                              bool isCaseSensitive=false,
                                              bool useExtendedVerseBoundaries=false);
    
    void terminate();

private:
    sword::ListKey getScopeKey(sword::SWModule* module, SearchScope scope);

    ModuleStore& _moduleStore;
    ModuleHelper& _moduleHelper;
    TextProcessor& _textProcessor;
    std::string _currentModuleName;
};

#endif // _MODULE_SEARCH