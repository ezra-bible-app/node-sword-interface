/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2026 Tobias Klein <contact@tklein.info>

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

bool ModuleSearch::validateSearchParameters(SWModule* module, const string& searchTerm)
{
    if (module == 0) {
        cerr << "ModuleSearch::getModuleSearchResults: getLocalModule returned zero pointer for " << _currentModuleName << endl;
        return false;
    } 
    
    if (searchTerm == "") {
        cerr << "ModuleSearch::getModuleSearchResults: cannot work with empty search term!" << endl;
        return false;
    }

    return true;
}

int ModuleSearch::getSearchFlags(bool isCaseSensitive, bool useExtendedVerseBoundaries)
{
    int flags = 0;

    if (!isCaseSensitive) {
        flags |= REG_ICASE;
    }

    if (!useExtendedVerseBoundaries) {
        flags |= SWModule::SEARCHFLAG_STRICTBOUNDARIES;
    }

    return flags;
}

string ModuleSearch::prepareStrongsSearchTerm(string searchTerm, SearchType searchType, SWModule* module)
{
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

    return "Word//Lemma./" + searchTerm;
}

bool ModuleSearch::phraseSequenceCheck(const vector<string>& words, const vector<string>& searchWords)
{
    // For single word searches, sequence doesn't matter
    if (searchWords.size() <= 1) {
        return true;
    }
    
    // Iterate through potential starting positions in the verse
    for (size_t startPos = 0; startPos < words.size(); startPos++) {
        // Check if this is a match for the first word
        if (words[startPos] == searchWords[0]) {
            // See if subsequent words match in sequence
            bool sequenceMatch = true;
            for (size_t i = 1; i < searchWords.size(); i++) {
                size_t nextPos = startPos + i;
                // Make sure we don't go beyond the verse text
                if (nextPos >= words.size() || words[nextPos] != searchWords[i]) {
                    sequenceMatch = false;
                    break;
                }
            }
            
            if (sequenceMatch) {
                return true;
            }
        }
    }
    
    return false;
}

vector<string> ModuleSearch::getSearchResultReferences(SWModule* module, ListKey& listKey, 
                                                       const string& searchTerm, SearchType searchType,
                                                       bool isCaseSensitive, bool filterOnWordBoundaries,
                                                       bool hasStrongs, bool hasInconsistentClosingEndDivs,
                                                       bool moduleMarkupIsBroken)
{
    vector<string> filteredReferences;
    
    // Disable markup before filtering on word boundaries
    this->_textProcessor.disableMarkup();

    // Make the search term lower case if case sensitivity is not required
    string lowerCaseSearchTerm = searchTerm;
    if (!isCaseSensitive) {
        std::transform(lowerCaseSearchTerm.begin(), lowerCaseSearchTerm.end(), lowerCaseSearchTerm.begin(), ::tolower);
    }

    // Split the search term into individual words
    vector<string> searchWords = StringHelper::split(lowerCaseSearchTerm, " ");

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

        // Filter verses based on word boundaries
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

            bool correctOrder = true;

            if (searchType == SearchType::phrase && allPartsMatch) {
                correctOrder = phraseSequenceCheck(words, searchWords);
            }

            if (allPartsMatch && correctOrder) {
                filteredReferences.push_back(module->getKey()->getShortText());
            }
        } else {
            // If not filtering on word boundaries, we simply push the reference
            filteredReferences.push_back(module->getKey()->getShortText());
        }

        listKey++;
    }

    // Enable markup again after word boundary filtering
    this->_textProcessor.enableMarkup();
    
    return filteredReferences;
}

vector<Verse> ModuleSearch::createVersesFromReferences(SWModule* module, const vector<string>& references,
                                                       bool hasStrongs, bool hasInconsistentClosingEndDivs,
                                                       bool moduleMarkupIsBroken)
{
    vector<Verse> verses;
    map<string, int> absoluteVerseNumbers = this->_moduleHelper.getAbsoluteVerseNumberMap(module);

    for (const auto& reference : references) {
        module->setKey(reference.c_str());
        string verseText = this->_textProcessor.getCurrentVerseText(module,
                                                                    hasStrongs,
                                                                    hasInconsistentClosingEndDivs,
                                                                    moduleMarkupIsBroken);

        Verse currentVerse;
        currentVerse.reference = reference;
        currentVerse.absoluteVerseNumber = absoluteVerseNumbers[reference];
        currentVerse.content = verseText;

        verses.push_back(currentVerse);
    }
    
    return verses;
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
    vector<Verse> searchResults;

    if (!validateSearchParameters(module, searchTerm)) {
        return searchResults;
    }

    int flags = getSearchFlags(isCaseSensitive, useExtendedVerseBoundaries);

    ListKey scopeKey;

    if (searchScope != SearchScope::BIBLE) {
        scopeKey = this->getScopeKey(module, searchScope);
        scope = &scopeKey;
    }

    bool hasStrongs = this->_moduleHelper.moduleHasGlobalOption(module, "Strongs");
    bool moduleMarkupIsBroken = this->_moduleHelper.isBrokenMarkupModule(moduleName);
    bool hasInconsistentClosingEndDivs = this->_moduleHelper.isInconsistentClosingEndDivModule(moduleName);

    if (searchType == SearchType::strongsNumber && hasStrongs) {
        searchTerm = prepareStrongsSearchTerm(searchTerm, searchType, module);
        flags |= SWModule::SEARCHFLAG_MATCHWHOLEENTRY;
    }

    // Perform search
    listKey = module->search(searchTerm.c_str(), int(searchType), flags, scope, 0, internalModuleSearchProgressCB);

    // Get search result references while considering the word boundary filter option
    vector<string> filteredReferences = getSearchResultReferences(module, listKey, searchTerm, searchType, 
                                                                  isCaseSensitive, filterOnWordBoundaries, 
                                                                  hasStrongs, hasInconsistentClosingEndDivs, 
                                                                  moduleMarkupIsBroken);

    searchResults = createVersesFromReferences(module, filteredReferences, hasStrongs, 
                                               hasInconsistentClosingEndDivs, moduleMarkupIsBroken);

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