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

// std includes
#include <map>
#include <string>
#include <vector>
#include <regex>

// sword includes
#include <swmodule.h>
#include <versekey.h>

// Own includes
#include "module_search.hpp"
#include "module_store.hpp"
#include "module_helper.hpp"
#include "text_processor.hpp"

/* If this bit is set, then ignore case when matching.
   If not set, then case is significant.  */
#ifndef REG_ICASE
#define REG_ICASE std::regex::icase
#endif

using namespace std;
using namespace sword;

map<string, int> ModuleSearch::getAbsoluteVerseNumberMap(SWModule* module)
{
    string lastBookName = "";
    string lastKey = "";
    int currentAbsoluteVerseNumber = 1;

    std::map<std::string, int> absoluteVerseNumbers;
    module->setKey("Gen 1:1");

    for (;;) {
        VerseKey currentVerseKey(module->getKey());
        string currentBookName(currentVerseKey.getBookAbbrev());
        string currentKey(module->getKey()->getShortText());

        // Stop, once the newly read key is the same as the previously read key
        if (currentKey == lastKey) { break; }

        // Reset the currentAbsoluteVerseNumber when a new book is started
        if ((currentAbsoluteVerseNumber > 0) && (currentBookName != lastBookName)) { currentAbsoluteVerseNumber = 1; }

        absoluteVerseNumbers[currentKey] = currentAbsoluteVerseNumber;

        module->increment();
        currentAbsoluteVerseNumber++;
        lastBookName = currentBookName;
        lastKey = currentKey;
    }

    return absoluteVerseNumbers;
}

vector<Verse> ModuleSearch::getModuleSearchResults(string moduleName,
                                                   string searchTerm,
                                                   SearchType searchType,
                                                   bool isCaseSensitive)
{
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    ListKey listKey;
    ListKey *scope = 0;
    int flags = 0;
    // This holds the text that we will return
    vector<Verse> searchResults;

    if (!isCaseSensitive) {
        // for case insensitivity
        flags |= REG_ICASE;
    }

    // Use strict search boundaries (only search within individual verses). TODO: Make this configurable.
    flags |= SEARCHFLAG_STRICTBOUNDARIES;

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        bool hasStrongs = this->_moduleHelper.moduleHasGlobalOption(module, "Strongs");

        if (searchType == SearchType::strongsNumber) {
            if (!hasStrongs) {
                // Return immediately if search type is Strong's, but the module does not have Strong's support
                return searchResults;
            }

            // If the Strong's key is OT we need to insert a zero in front of the key
            // This is necessary because the Sword modules with Strong's have a zero in front of the Hebrew Strong's numbers
            if (searchTerm[0] == 'H' && this->_textProcessor.moduleHasStrongsZeroPrefixes(module)) {
                // Cut out the number from the Strong's key (starting at index 1 until end of string)
                string strongsKey = searchTerm.substr(1, searchTerm.size());
                // Overwrite the searchTerm with an inserted 0
                searchTerm = "H0" + strongsKey;
            }

            // from swmodule.h api docs:
            // for use with entryAttrib search type to match whole entry to value, e.g., G1234 and not G12345
            flags |= SEARCHFLAG_MATCHWHOLEENTRY;
            searchTerm = "Word//Lemma./" + searchTerm;
        }

        map<string, int> absoluteVerseNumbers = this->getAbsoluteVerseNumberMap(module);

        // Perform search
        listKey = module->search(searchTerm.c_str(), int(searchType), flags, scope, 0, internalModuleSearchProgressCB);

        // Populate searchResults vector
        while (!listKey.popError()) {
            module->setKey(listKey.getElement());

            string verseText = this->_textProcessor.getCurrentVerseText(module, hasStrongs);
            Verse currentVerse;
            currentVerse.reference = module->getKey()->getShortText();
            currentVerse.absoluteVerseNumber = absoluteVerseNumbers[currentVerse.reference];
            currentVerse.content = verseText;
            searchResults.push_back(currentVerse);

            listKey++;
        }
    }

    return searchResults;
}

static std::function<void(char, void*)>* _moduleSearchProgressCB = 0;

void internalModuleSearchProgressCB(char percent, void* userData)
{
    if (_moduleSearchProgressCB != 0) {
        //cout << "internal cb: " << (int)percent << endl;

        (*_moduleSearchProgressCB)(percent, userData);
    }
}

void setModuleSearchProgressCB(std::function<void(char, void*)>* moduleSearchProgressCB)
{
    _moduleSearchProgressCB = moduleSearchProgressCB;
}