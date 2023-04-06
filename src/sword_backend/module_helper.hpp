/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2023 Tobias Klein <contact@tklein.info>

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

#ifndef _MODULE_HELPER
#define _MODULE_HELPER

#include <string>
#include <map>

#include "module_store.hpp"

namespace sword {
    class SWModule;
};

class ModuleHelper {
public:
    ModuleHelper(ModuleStore& moduleStore) : _moduleStore(moduleStore) {
        // For modules in this list we will enforce the stripped version of the text and not use the markup
        this->_brokenMarkupModules = { "ASV" };

        // For modules in this list we will balance the closing end divs in the markup
        this->_inconsistentClosingEndDivModules = { "NETtext", "NASB" };
    }

    virtual ~ModuleHelper(){}

    bool moduleHasGlobalOption(sword::SWModule* module, std::string globalOption);
    bool moduleHasFeature(sword::SWModule* module, std::string feature);
    bool moduleHasStrongsKeys(sword::SWModule* module);
    bool moduleHasBook(sword::SWModule* module, std::string bookCode);

    std::vector<std::string> getBookList(std::string moduleName);
    int getBookChapterCount(std::string moduleName, std::string bookCode);
    int getChapterVerseCount(std::string moduleName, std::string bookCode, int chapter);
    std::map<std::string, int> getAbsoluteVerseNumberMap(sword::SWModule* module, std::vector<std::string> bookList={});
    bool isBrokenMarkupModule(std::string moduleName);
    bool isInconsistentClosingEndDivModule(std::string moduleName);

private:
    bool moduleHasKeyValuePair(sword::SWModule* module, std::string key, std::string value);
    ModuleStore& _moduleStore;
    std::vector<std::string> _brokenMarkupModules;
    std::vector<std::string> _inconsistentClosingEndDivModules;
};

#endif // _MODULE_HELPER