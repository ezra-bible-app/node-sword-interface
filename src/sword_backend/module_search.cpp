/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2021 Tobias Klein <contact@tklein.info>

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

// std includes
#include <map>
#include <string>
#include <vector>
#include <algorithm>

// sword includes
#include <swmodule.h>
#include <versekey.h>

// Own includes
#include "module_search.hpp"
#include "module_store.hpp"
#include "module_helper.hpp"
#include "text_processor.hpp"

/* REGEX definitions from regex.h */
/* POSIX `cflags' bits (i.e., information for `regcomp').  */

/* If this bit is set, then use extended regular expression syntax.
   If not set, then use basic regular expression syntax.  */
#ifndef REG_EXTENDED
#define REG_EXTENDED 1
#endif

/* If this bit is set, then ignore case when matching.
   If not set, then case is significant.  */
#ifndef REG_ICASE
#define REG_ICASE (REG_EXTENDED << 1)
#endif

using namespace std;
using namespace sword;

ListKey ModuleSearch::getScopeKey(SearchScope scope)
{
    ListKey key;

    switch (scope) {
        case SearchScope::OT:
        {
            const char* otBooks = "Genesis;Exodus;Leviticus;Numbers;Joshua;Judges;"
                                  "Ruth;I Samuel;II Samuel;I Kings;II Kings;I Chronicles;II Chronicles;"
                                  "Ezra;Nehemiah;Esther;Job;Psalms;Proverbs;Ecclesiastes;Song of Solomon;"
                                  "Isaiah;Jeremiah;Lamentations;Ezekiel;Daniel;Hosea;Joel;Amos;Obadiah;"
                                  "Jonah;Micah;Nahum;Habakkuk;Zephaniah;Haggai;Zechariah;Malachi;";

            key = VerseKey().parseVerseList(otBooks, "", true);
            break;
        }
        
        case SearchScope::NT:
        {
            const char* ntBooks = "Matthew;Mark;Luke;John;Acts;Romans;I Corinthians;II Corinthians;"
                                  "Galatians;Ephesians;Philippians;Colossians;I Thessalonians;II Thessalonians;"
                                  "I Timothy;II Timothy;Titus;Philemon;Hebrews;James;I Peter;II Peter;"
                                  "I John;II John;III John;Jude;Revelation of John;";

            key = VerseKey().parseVerseList(ntBooks, "", true);
            break;
        }
        
        default:
            break;
    }

    return key;
}

vector<Verse> ModuleSearch::getModuleSearchResults(string moduleName,
                                                   string searchTerm,
                                                   SearchType searchType,
                                                   SearchScope searchScope,
                                                   bool isCaseSensitive,
                                                   bool useExtendedVerseBoundaries)
{
    this->_currentModuleName = moduleName;
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    ListKey listKey;
    SWKey* scope = 0;
    int flags = 0;
    // This holds the text that we will return
    vector<Verse> searchResults;
    vector<string> searchResultReferences;

    if (!isCaseSensitive) {
        // for case insensitivity
        flags |= REG_ICASE;
    }

    if (!useExtendedVerseBoundaries) {
        // Use strict search boundaries (only search within individual verses). TODO: Make this configurable.
        flags |= SWModule::SEARCHFLAG_STRICTBOUNDARIES;
    }

    if (module == 0) {
        cerr << "ModuleSearch::getModuleSearchResults: getLocalModule returned zero pointer for " << moduleName << endl;
    } else if (searchTerm == "") {
        cerr << "ModuleSearch::getModuleSearchResults: cannot work with empty search term!" << endl;
    } else {
        ListKey scopeKey;

        if (searchScope != SearchScope::BIBLE) {
            scopeKey = this->getScopeKey(searchScope);
            scope = &scopeKey;
        }

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
            flags |= SWModule::SEARCHFLAG_MATCHWHOLEENTRY;

            searchTerm = "Word//Lemma./" + searchTerm;
        }

        map<string, int> absoluteVerseNumbers = this->_moduleHelper.getAbsoluteVerseNumberMap(module);

        // Perform search
        listKey = module->search(searchTerm.c_str(), int(searchType), flags, scope, 0, internalModuleSearchProgressCB);

        // Populate searchResults vector
        while (!listKey.popError()) {
            module->setKey(listKey.getElement());

            string verseText = this->_textProcessor.getCurrentVerseText(module, hasStrongs);
            string currentReference = module->getKey()->getShortText();

            if (std::find(searchResultReferences.begin(),
                          searchResultReferences.end(),                         // Only accept the result if we do not
                          currentReference) == searchResultReferences.end()) {  // have it yet!
            
                Verse currentVerse;
                currentVerse.reference = module->getKey()->getShortText();
                currentVerse.absoluteVerseNumber = absoluteVerseNumbers[currentVerse.reference];
                currentVerse.content = verseText;
                searchResults.push_back(currentVerse);
                searchResultReferences.push_back(currentReference);
            }

            listKey++;
        }
    }

    this->_currentModuleName = "";

    return searchResults;
}

void ModuleSearch::terminate()
{
    if (this->_currentModuleName != "") {
        SWModule* module = this->_moduleStore.getLocalModule(this->_currentModuleName);
        module->terminateSearch = true;
        this->_currentModuleName = "";
    }
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