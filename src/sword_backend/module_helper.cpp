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

// Std includes
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

// Sword includes
#include <swmodule.h>
#include <versekey.h>
#include <localemgr.h>

#include "module_helper.hpp"

using namespace std;
using namespace sword;

bool ModuleHelper::moduleHasGlobalOption(SWModule* module, string globalOption)
{
    return this->moduleHasKeyValuePair(module, "GlobalOptionFilter", globalOption);
}

bool ModuleHelper::moduleHasFeature(sword::SWModule* module, std::string feature)
{
    return this->moduleHasKeyValuePair(module, "Feature", feature);
}

bool ModuleHelper::moduleHasStrongsKeys(sword::SWModule* module)
{
    bool hasHebrewStrongsKeys = this->moduleHasFeature(module, "HebrewDef");
    bool hasGreekStrongsKeys = this->moduleHasFeature(module, "GreekDef");

    return hasHebrewStrongsKeys || hasGreekStrongsKeys;
}

bool ModuleHelper::moduleHasKeyValuePair(sword::SWModule* module, std::string key, std::string value)
{
    bool hasKeyValuePair = false;
    ConfigEntMap::const_iterator it = module->getConfig().lower_bound(key.c_str());
    ConfigEntMap::const_iterator end = module->getConfig().upper_bound(key.c_str());

    for(; it !=end; ++it) {
        string currentValue = string(it->second.c_str());
        if (currentValue.find(value) != string::npos) {
            hasKeyValuePair = true;
            break;
        }
    }

    return hasKeyValuePair;
}

bool ModuleHelper::moduleHasBook(sword::SWModule* module, std::string bookCode)
{
    bool hasBook = true;
    stringstream key;
    key << bookCode;
    key << " 1:1";

    module->setKey(key.str().c_str());
    string moduleKeyText = string(module->getKey()->getShortText());

    /* In case of apocryphal books the hasEntry method below is not enough.
       Once we have set the key we need to compare the actual module key with the key we wanted to set.
       For apocryphal books we may get "Rev 1:1" as actual module key and this will mismatch with the set key
       and then indicate that the book is not existing. */

    // We compare the first three letters of the two keys to see whether the key retrieved matches the key we are looking for
    if (moduleKeyText.size() >= 3 && key.str().size() >= 3 && moduleKeyText.substr(0,3) != key.str().substr(0,3)) {
        hasBook = false;
    } else {
        // The first three letters of the key seem ok.
        // Now let's check next whether the module has the corresponding entry.
        hasBook = module->hasEntry(module->getKey());
    }

    return hasBook;
}

vector<string> ModuleHelper::getBookList(string moduleName)
{
    string currentBookName = "";
    vector<string> bookList;
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        VerseKey *vk = (VerseKey *)module->getKey();

        for ((*vk) = TOP; !vk->popError(); vk->setBook(vk->getBook()+1)) {
            if (module->hasEntry(vk)) {
                currentBookName = vk->getOSISBookName();
                bookList.push_back(currentBookName);
            }
        }
    }

    return bookList;
}

int ModuleHelper::getBookChapterCount(std::string moduleName, std::string bookCode)
{
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    int bookChapterCount = -1;

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        stringstream key;
        key << bookCode;
        key << " 1:1";

        module->setKey(key.str().c_str());
        VerseKey currentVerseKey(module->getKey());

        bookChapterCount = currentVerseKey.getChapterMax();
    }

    return bookChapterCount;
}

int ModuleHelper::getChapterVerseCount(std::string moduleName, std::string bookCode, int chapter)
{
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    int chapterVerseCount = -1;

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        stringstream key;
        key << bookCode << " " << chapter << ":1";

        module->setKey(key.str().c_str());
        VerseKey currentVerseKey(module->getKey());

        chapterVerseCount = currentVerseKey.getVerseMax();
    }

    return chapterVerseCount;
}

map<string, int> ModuleHelper::getAbsoluteVerseNumberMap(SWModule* module, vector<string> bookList)
{
    string lastKey = "";
    int currentAbsoluteVerseNumber = 1;
    string lastBookName = "";
    string currentBookName = "";

    std::map<std::string, int> absoluteVerseNumbers;

    if (bookList.size() == 0) {
        bookList = this->getBookList(string(module->getName()));
    }

    for (unsigned int i = 0; i < bookList.size(); i++) {
        string currentBook = bookList[i];
        stringstream initialKey;
        initialKey << currentBook << " " << "1:1";
        currentAbsoluteVerseNumber = 1;

        module->setKey(initialKey.str().c_str());

        VerseKey currentVerseKey(module->getKey());
        currentBookName = currentVerseKey.getBookAbbrev();
        lastBookName = currentBookName;

        for (;;) {
            currentVerseKey = module->getKey();
            currentBookName = currentVerseKey.getBookAbbrev();
            string currentKey(currentVerseKey.getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }

            // Stop once we finished the book
            if ((currentAbsoluteVerseNumber > 0) && (currentBookName != lastBookName)) { break; }

            absoluteVerseNumbers[currentKey] = currentAbsoluteVerseNumber;

            module->increment();
            currentAbsoluteVerseNumber++;
            lastKey = currentKey;
            lastBookName = currentBookName;
        }
    }

    return absoluteVerseNumbers;
}

bool ModuleHelper::isBrokenMarkupModule(std::string moduleName)
{
    return std::find(this->_brokenMarkupModules.begin(),
                     this->_brokenMarkupModules.end(),
                     moduleName) != this->_brokenMarkupModules.end();
}

bool ModuleHelper::isInconsistentClosingEndDivModule(std::string moduleName)
{
    return std::find(this->_inconsistentClosingEndDivModules.begin(),
                     this->_inconsistentClosingEndDivModules.end(),
                     moduleName) != this->_inconsistentClosingEndDivModules.end();
}