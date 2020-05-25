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

// Std includes
#include <string>
#include <vector>

// Sword includes
#include <swmodule.h>
#include <versekey.h>
#include <localemgr.h>

#include "module_helper.hpp"

using namespace std;
using namespace sword;

bool ModuleHelper::moduleHasGlobalOption(SWModule* module, string globalOption)
{
    bool hasGlobalOption = false;
    ConfigEntMap::const_iterator it = module->getConfig().lower_bound("GlobalOptionFilter");
    ConfigEntMap::const_iterator end = module->getConfig().upper_bound("GlobalOptionFilter");

    for(; it !=end; ++it) {
        string currentOption = string(it->second.c_str());
        if (currentOption.find(globalOption) != string::npos) {
            hasGlobalOption = true;
            break;
        }
    }

    return hasGlobalOption;
}

vector<string> ModuleHelper::getBookList(string moduleName)
{
    string currentBookName = "";
    string lastBookName = "";
    string lastKey = "";
    vector<string> bookList;
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        module->setKey("Gen 1:1");

        for (;;) {
            VerseKey currentVerseKey(module->getKey());
            currentBookName = currentVerseKey.getBookAbbrev();
            string currentKey(module->getKey()->getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }

            if (currentBookName != lastBookName) {
                string firstVerseText = string(module->getRawEntry());

                if (firstVerseText.length() != 0) {
                    // We assume the book is existing if the first verse has content
                    bookList.push_back(currentBookName);
                }
            }

            module->increment();
            lastBookName = currentBookName;
            lastKey = currentKey;
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