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

#ifndef _TEXT_PROCESSOR
#define _TEXT_PROCESSOR

#include "common_defs.hpp"

namespace sword {
    class SWModule;
};

class ModuleStore;
class ModuleHelper;
class StrongsEntry;

class TextProcessor
{
public:
    TextProcessor(ModuleStore& moduleStore, ModuleHelper& moduleHelper);
    virtual ~TextProcessor() {}

    void enableMarkup() { this->_markupEnabled = true; }
    void disableMarkup() { this->_markupEnabled = false; }

    std::vector<Verse> getBibleText(std::string moduleName);
    std::vector<Verse> getBookText(std::string moduleName, std::string bookCode, int startVerseNumber=-1, int verseCount=-1);
    std::vector<Verse> getChapterText(std::string moduleName, std::string bookCode, int chapter);
    std::vector<Verse> getVersesFromReferences(std::string moduleName, std::vector<std::string>& references);
    std::vector<std::string> getReferencesFromReferenceRange(std::string referenceRange);
    std::string getCurrentVerseText(sword::SWModule* module, bool hasStrongs, bool forceNoMarkup=false);
    std::string getBookIntroduction(std::string moduleName, std::string bookCode);

    StrongsEntry* getStrongsEntry(std::string key);

    bool moduleHasStrongsZeroPrefixes(sword::SWModule* module);
    bool isModuleReadable(sword::SWModule* module, std::string key="John 1:1");

private:
    std::vector<Verse> getText(std::string moduleName,
                               std::string key,
                               QueryLimit queryLimit=QueryLimit::none,
                               int startVerseNr=-1,
                               int verseCount=-1);

    std::string getCurrentChapterHeading(sword::SWModule* module);
    std::string getFilteredText(const std::string& text, int chapter, bool hasStrongs=false);
    std::string replaceSpacesInStrongs(const std::string& text);
    void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);

    std::string getBookFromReference(std::string reference);
    std::vector<std::string> getBookListFromReferences(std::vector<std::string>& references);

    ModuleStore& _moduleStore;
    ModuleHelper& _moduleHelper;
    bool _markupEnabled;
    bool _rawMarkupEnabled;
};

#endif // _TEXT_PROCESSOR