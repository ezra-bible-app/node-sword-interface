/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2025 Tobias Klein <contact@tklein.info>

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
#include <regex>
#include <sstream>

// sword includes
#include <swmgr.h>
#include <swmodule.h>
#include <versekey.h>

// Own includes
#include "module_search.hpp"
#include "module_store.hpp"
#include "module_helper.hpp"
#include "text_processor.hpp"
#include "string_helper.hpp"

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

ListKey ModuleSearch::getScopeKey(SWModule* module, SearchScope scope)
{
    ListKey key;

    if (module == 0) {
      cerr << "ModuleSearch::getScopeKey / received 0 pointer for module!!!";
      return key;
    }

    VerseKey verseKey = module->getKey();

    switch (scope) {
        case SearchScope::OT:
        {
            const char* otBooks = "Genesis;Exodus;Leviticus;Numbers;Joshua;Judges;"
                                  "Ruth;I Samuel;II Samuel;I Kings;II Kings;I Chronicles;II Chronicles;"
                                  "Ezra;Nehemiah;Esther;Job;Psalms;Proverbs;Ecclesiastes;Song of Solomon;"
                                  "Isaiah;Jeremiah;Lamentations;Ezekiel;Daniel;Hosea;Joel;Amos;Obadiah;"
                                  "Jonah;Micah;Nahum;Habakkuk;Zephaniah;Haggai;Zechariah;Malachi;";

            key = verseKey.parseVerseList(otBooks, "", true);
            break;
        }
        
        case SearchScope::NT:
        {
            const char* ntBooks = "Matthew;Mark;Luke;John;Acts;Romans;I Corinthians;II Corinthians;"
                                  "Galatians;Ephesians;Philippians;Colossians;I Thessalonians;II Thessalonians;"
                                  "I Timothy;II Timothy;Titus;Philemon;Hebrews;James;I Peter;II Peter;"
                                  "I John;II John;III John;Jude;Revelation of John;";

            key = verseKey.parseVerseList(ntBooks, "", true);
            break;
        }
        
        default:
            break;
    }

    return key;
}

inline bool isDisallowedCharacter(char c) {
    static const string disallowedPunctuation = ",;.:'´‘’\"“”?!()-=<>/";
    return disallowedPunctuation.find(c) != string::npos;
}

vector<Verse> ModuleSearch::getModuleSearchResults(string moduleName,
                                                   string searchTerm,
                                                   SearchType searchType,
                                                   SearchScope searchScope,
                                                   bool isCaseSensitive,
                                                   bool useExtendedVerseBoundaries,
                                                   bool filterOnWordBoundaries)
{
    this->_currentModuleName = moduleName;
    SWModule* module = this->_moduleStore.getSearchSwMgr()->getModule(moduleName.c_str());
    ListKey listKey;
    SWKey* scope = 0;
    int flags = 0;
    vector<Verse> searchResults;
    vector<string> searchResultReferences;

    if (!isCaseSensitive) {
        flags |= REG_ICASE;
    }

    if (!useExtendedVerseBoundaries) {
        flags |= SWModule::SEARCHFLAG_STRICTBOUNDARIES;
    }

    if (module == 0) {
        cerr << "ModuleSearch::getModuleSearchResults: getLocalModule returned zero pointer for " << moduleName << endl;
    } else if (searchTerm == "") {
        cerr << "ModuleSearch::getModuleSearchResults: cannot work with empty search term!" << endl;
    } else {
        ListKey scopeKey;

        if (searchScope != SearchScope::BIBLE) {
            scopeKey = this->getScopeKey(module, searchScope);
            scope = &scopeKey;
        }

        bool hasStrongs = this->_moduleHelper.moduleHasGlobalOption(module, "Strongs");
        bool moduleMarkupIsBroken = this->_moduleHelper.isBrokenMarkupModule(moduleName);
        bool hasInconsistentClosingEndDivs = this->_moduleHelper.isInconsistentClosingEndDivModule(moduleName);

        if (searchType == SearchType::strongsNumber) {
            if (!hasStrongs) {
                return searchResults;
            }

            string strongsNumber = searchTerm.substr(1, searchTerm.size());

            if (searchTerm[0] == 'H') {
                if (this->_textProcessor.moduleHasStrongsPaddedZeroPrefixes(module)) {
                    string paddedStrongsNumber = this->_textProcessor.padStrongsNumber(strongsNumber);
                    searchTerm = "H" + paddedStrongsNumber;
                } else if (this->_textProcessor.moduleHasStrongsZeroPrefixes(module)) {
                    searchTerm = "H0" + strongsNumber;
                }
            } else if (searchTerm[0] == 'G') {
                if (this->_textProcessor.moduleHasStrongsPaddedZeroPrefixes(module)) {
                    string paddedStrongsNumber = this->_textProcessor.padStrongsNumber(strongsNumber);
                    searchTerm = "G" + paddedStrongsNumber;
                }
            }

            flags |= SWModule::SEARCHFLAG_MATCHWHOLEENTRY;
            searchTerm = "Word//Lemma./" + searchTerm;
        }

        // Perform search
        listKey = module->search(searchTerm.c_str(), int(searchType), flags, scope, 0, internalModuleSearchProgressCB);

        // Disable markup before filtering on word boundaries
        this->_textProcessor.disableMarkup();

        vector<string> filteredReferences;

        // Make the search term lower case if case sensitivity is not required
        string lowerCaseSearchTerm = searchTerm;
        if (!isCaseSensitive) {
            std::transform(lowerCaseSearchTerm.begin(), lowerCaseSearchTerm.end(), lowerCaseSearchTerm.begin(), ::tolower);
        }

        // Split the search term into individual words
        vector<string> searchWords = StringHelper::split(lowerCaseSearchTerm, " ");

        // Filter verses based on word boundaries
        while (!listKey.popError()) {
            module->setKey(listKey.getElement());
            string verseText = this->_textProcessor.getCurrentVerseText(module,
                                                                        hasStrongs,
                                                                        hasInconsistentClosingEndDivs,
                                                                        moduleMarkupIsBroken);

            // Make the verse text lower case if case sensitivity is not required
            string lowerCaseVerseText = verseText;
            if (!isCaseSensitive) {
                std::transform(lowerCaseVerseText.begin(), lowerCaseVerseText.end(), lowerCaseVerseText.begin(), ::tolower);
            }

            if (filterOnWordBoundaries) {
                // Replace disallowed characters with spaces
                std::replace_if(lowerCaseVerseText.begin(), lowerCaseVerseText.end(),
                                [](char c) { return isDisallowedCharacter(c); }, ' ');

                vector<string> words = StringHelper::split(lowerCaseVerseText, " ");

                // Check if all parts of the search term match any word in the verse
                bool allPartsMatch = true;
                for (const auto& searchWord : searchWords) {
                    if (std::find(words.begin(), words.end(), searchWord) == words.end()) {
                        allPartsMatch = false;
                        break;
                    }
                }

                if (allPartsMatch) {
                    filteredReferences.push_back(module->getKey()->getShortText());
                }
            } else {
                filteredReferences.push_back(module->getKey()->getShortText());
            }

            listKey++;
        }

        // Enable markup again after word boundary filtering
        this->_textProcessor.enableMarkup();

        map<string, int> absoluteVerseNumbers = this->_moduleHelper.getAbsoluteVerseNumberMap(module);

        // Populate searchResults vector
        for (const auto& reference : filteredReferences) {
            module->setKey(reference.c_str());
            string verseText = this->_textProcessor.getCurrentVerseText(module,
                                                                        hasStrongs,
                                                                        hasInconsistentClosingEndDivs,
                                                                        moduleMarkupIsBroken);

            Verse currentVerse;
            currentVerse.reference = reference;
            currentVerse.absoluteVerseNumber = absoluteVerseNumbers[reference];
            currentVerse.content = verseText;

            searchResults.push_back(currentVerse);
        }
    }

    this->_currentModuleName = "";

    return searchResults;
}

void ModuleSearch::terminate()
{
    if (this->_currentModuleName != "") {
        SWModule* module = this->_moduleStore.getSearchSwMgr()->getModule(this->_currentModuleName.c_str());
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