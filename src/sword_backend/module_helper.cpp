/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>

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

map<string, vector<int>> ModuleHelper::getBibleChapterVerseCounts(std::string moduleName)
{
    string currentBookName = "";
    string lastBookName = "";
    string lastKey = "";
    int lastChapter = -1;
    int currentChapterCount = 0;
    map<string, vector<int>> bibleChapterVerseCounts;
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        module->setKey("Gen 1:1");

        for (;;) {
            VerseKey currentVerseKey(module->getKey());
            currentBookName = currentVerseKey.getBookAbbrev();
            int currentChapter = currentVerseKey.getChapter();
            string currentKey(module->getKey()->getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }

            if (currentBookName != lastBookName) {
                // Init a new chapter verse count vector for the new book
                vector<int> currentChapterVerseCounts;
                bibleChapterVerseCounts[currentBookName] = currentChapterVerseCounts;
            }

            if (lastChapter != -1 && (currentChapter != lastChapter || currentBookName != lastBookName)) {
                bibleChapterVerseCounts[lastBookName].push_back(currentChapterCount);
                currentChapterCount = 0;
            }

            currentChapterCount++;

            module->increment();
            lastChapter = currentChapter;
            lastBookName = currentBookName;
            lastKey = currentKey;
        }

        bibleChapterVerseCounts[currentBookName].push_back(currentChapterCount);
    }

    return bibleChapterVerseCounts;
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
