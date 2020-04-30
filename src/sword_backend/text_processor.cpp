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
#include <sstream>
#include <string>
#include <regex>

// Sword includes
#include <versekey.h>
#include <swmodule.h>
#include <swkey.h>

// Own includes
#include "text_processor.hpp"
#include "module_helper.hpp"
#include "string_helper.hpp"
#include "strongs_entry.hpp"

using namespace std;
using namespace sword;

TextProcessor::TextProcessor(ModuleStore& moduleStore, ModuleHelper& moduleHelper)
    : _moduleStore(moduleStore), _moduleHelper(moduleHelper)
{
    this->initStrongs();
}

TextProcessor::~TextProcessor() {
    if (this->_strongsHebrew != 0) delete this->_strongsHebrew;
    if (this->_strongsGreek != 0) delete this->_strongsGreek;
}

void TextProcessor::initStrongs()
{
    if (this->_strongsHebrew == 0 || this->_strongsGreek == 0) {
        this->_strongsHebrew = this->_moduleStore.getLocalModule("StrongsHebrew");
        this->_strongsGreek = this->_moduleStore.getLocalModule("StrongsGreek");
    }
}

string TextProcessor::getFilteredText(const string& text, bool hasStrongs)
{
    static regex schlachterMarkupFilter = regex("<H.*> ");
    static regex chapterFilter = regex("<chapter.*/>");
    static regex lbBeginParagraph = regex("<lb type=\"x-begin-paragraph\"/>");
    static regex lbEndParagraph = regex("<lb type=\"x-end-paragraph\"/>");
    static regex lbElementFilter = regex("<lb ");
    static regex lElementFilter = regex("<l ");
    static regex lgElementFilter = regex("<lg ");
    static regex noteStartElementFilter = regex("<note");
    static regex noteEndElementFilter = regex("</note>");
    static regex headStartElementFilter = regex("<head");
    static regex headEndElementFilter = regex("</head>");
    static regex appStartElementFilter = regex("<app");
    static regex appEndElementFilter = regex("</app>");
    static regex rtxtFilter = regex("<rtxt type=\"omit\"/>");
    static regex quoteJesusElementFilter = regex("<q marker=\"\" who=\"Jesus\">");
    static regex quoteElementFilter = regex("<q ");
    static regex titleStartElementFilter = regex("<title");
    static regex titleEndElementFilter = regex("</title>");
    static regex divMilestoneFilter = regex("<div type=\"x-milestone\"");
    static regex milestoneFilter = regex("<milestone");
    static regex xBrFilter = regex("x-br\"/>");
    static regex divSIDFilter = regex("<div sID=");
    static regex divEIDFilter = regex("<div eID=");
    static regex divineNameStartElement = regex("<divineName>");
    static regex divineNameEndElement = regex("</divineName>");
    static regex strongsWElement = regex("<w lemma=");

    static regex fullStopWithoutSpace = regex("[.]<");
    static regex questionMarkWithoutSpace = regex("[?]<");
    static regex exclamationMarkWithoutSpace = regex("!<");
    static regex commaWithoutSpace = regex(",<");
    static regex semiColonWithoutSpace = regex(";<");
    static regex colonWithoutSpace = regex(":<");

    string filteredText = text;
    filteredText = regex_replace(filteredText, schlachterMarkupFilter, "");
    filteredText = regex_replace(filteredText, chapterFilter, "");
    filteredText = regex_replace(filteredText, lbBeginParagraph, "");
    filteredText = regex_replace(filteredText, lbEndParagraph, "&nbsp;<div class=\"sword-markup sword-paragraph-end\"><br/></div>");
    filteredText = regex_replace(filteredText, lbElementFilter, "<div class=\"sword-markup sword-lb\" ");
    filteredText = regex_replace(filteredText, lElementFilter, "<div class=\"sword-markup sword-l\" ");
    filteredText = regex_replace(filteredText, lgElementFilter, "<div class=\"sword-markup sword-lg\" ");
    filteredText = regex_replace(filteredText, noteStartElementFilter, "<div class=\"sword-markup sword-note\" ");
    filteredText = regex_replace(filteredText, noteEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, headStartElementFilter, "<div class=\"sword-markup sword-head\" ");
    filteredText = regex_replace(filteredText, headEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, appStartElementFilter, "<div class=\"sword-markup sword-app\" ");
    filteredText = regex_replace(filteredText, appEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, rtxtFilter, "");

    filteredText = regex_replace(filteredText, titleStartElementFilter, "<div class=\"sword-markup sword-section-title\"");
    filteredText = regex_replace(filteredText, titleEndElementFilter, "</div>");
    filteredText = regex_replace(filteredText, divMilestoneFilter, "<div class=\"sword-markup sword-x-milestone\"");
    filteredText = regex_replace(filteredText, milestoneFilter, "<div class=\"sword-markup sword-milestone\"");
    filteredText = regex_replace(filteredText, xBrFilter, "x-br\"/> ");
    filteredText = regex_replace(filteredText, divSIDFilter, "<div class=\"sword-markup sword-sid\" sID=");
    filteredText = regex_replace(filteredText, divEIDFilter, "<div class=\"sword-markup sword-eid\" eID=");
    filteredText = regex_replace(filteredText, quoteJesusElementFilter, "<div class=\"sword-markup sword-quote-jesus\"/>");
    filteredText = regex_replace(filteredText, quoteElementFilter, "&quot;<div class=\"sword-markup sword-quote\" ");
    filteredText = regex_replace(filteredText, divineNameStartElement, "");
    filteredText = regex_replace(filteredText, divineNameEndElement, "");
    filteredText = regex_replace(filteredText, strongsWElement, "<w class=");

    filteredText = regex_replace(filteredText, fullStopWithoutSpace, ". <");
    filteredText = regex_replace(filteredText, questionMarkWithoutSpace, "? <");
    filteredText = regex_replace(filteredText, exclamationMarkWithoutSpace, "! <");
    filteredText = regex_replace(filteredText, commaWithoutSpace, ", <");
    filteredText = regex_replace(filteredText, semiColonWithoutSpace, "; <");
    filteredText = regex_replace(filteredText, colonWithoutSpace, ": <");

    if (hasStrongs) {
        filteredText = this->replaceSpacesInStrongs(filteredText);
    }

    return filteredText;
}

string TextProcessor::getCurrentChapterHeading(sword::SWModule* module)
{
    string chapterHeading = "";
    VerseKey currentVerseKey(module->getKey());

    if (currentVerseKey.getVerse() == 1) { // X:1, set key to X:0
        // Include chapter/book/testament/module intros
        currentVerseKey.setIntros(true);
        currentVerseKey.setVerse(0);

        module->setKey(currentVerseKey);
        chapterHeading = string(module->getRawEntry());
        StringHelper::trim(chapterHeading);

        // Set verse back to X:1
        currentVerseKey.setVerse(1);
        module->setKey(currentVerseKey);
    }

    chapterHeading = this->getFilteredText(chapterHeading);
    return chapterHeading;
}

string TextProcessor::getCurrentVerseText(sword::SWModule* module, bool hasStrongs, bool forceNoMarkup)
{
    string verseText;
    string filteredText;

    if (this->_markupEnabled && !forceNoMarkup) {
        verseText = string(module->getRawEntry());
        StringHelper::trim(verseText);
        filteredText = this->getFilteredText(verseText, hasStrongs);
    } else {
        verseText = string(module->stripText());
        StringHelper::trim(verseText);
        filteredText = verseText;
    }
    
    return filteredText;
}

vector<Verse> TextProcessor::getBibleText(string moduleName)
{
    return this->getText(moduleName, "Gen 1:1");
}

vector<Verse> TextProcessor::getBookText(string moduleName, string bookCode, int startVerseNumber, int verseCount)
{
    stringstream key;
    key << bookCode;
    key << " 1:1";

    return this->getText(moduleName, key.str(), QueryLimit::book, startVerseNumber, verseCount);
}

vector<Verse> TextProcessor::getChapterText(string moduleName, string bookCode, int chapter)
{
    stringstream key;
    key << bookCode << " " << chapter << ":1";

    return this->getText(moduleName, key.str(), QueryLimit::chapter);
}

vector<Verse> TextProcessor::getText(string moduleName, string key, QueryLimit queryLimit, int startVerseNumber, int verseCount)
{
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    string lastKey;
    int index = 0;
    string lastBookName = "";
    int lastChapter = -1;
    bool currentBookExisting = true;

    // This holds the text that we will return
    vector<Verse> text;

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        bool hasStrongs = this->_moduleHelper.moduleHasGlobalOption(module, "Strongs");

        module->setKey(key.c_str());

        if (startVerseNumber >= 1) {
          module->increment(startVerseNumber - 1);
        } else {
          startVerseNumber = 1;
        }

        for (;;) {
            VerseKey currentVerseKey(module->getKey());
            string currentBookName(currentVerseKey.getBookAbbrev());
            int currentChapter = currentVerseKey.getChapter();
            bool firstVerseInBook = false;
            bool firstVerseInChapter = (currentVerseKey.getVerse() == 1);
            string verseText = "";
            string currentKey(module->getKey()->getShortText());

            // Stop, once the newly read key is the same as the previously read key
            if (currentKey == lastKey) { break; }
            // Stop, once the newly ready key is a different book than the previously read key
            if (queryLimit == QueryLimit::book && (index > 0) && (currentBookName != lastBookName)) { break; }
            // Stop, once the newly ready key is a different chapter than the previously read key
            if (queryLimit == QueryLimit::chapter && (index > 0) && (currentChapter != lastChapter)) { break; }
            // Stop once the maximum number of verses is reached
            if (startVerseNumber >= 1 && verseCount >= 1 && (index == verseCount)) { break; }

            if (currentBookName != lastBookName) {
                currentBookExisting = true;
                firstVerseInBook = true;
            }

            // Chapter heading
            if (firstVerseInChapter) {
                verseText += this->getCurrentChapterHeading(module);
            }

            // Current verse text
            verseText += this->getCurrentVerseText(module, hasStrongs);

            // If the current verse does not have any content and if it is the first verse in this book
            // we assume that the book is not existing.
            if (verseText.length() == 0 && firstVerseInBook) { currentBookExisting = false; }

            if (currentBookExisting) {
                Verse currentVerse;
                currentVerse.reference = module->getKey()->getShortText();
                currentVerse.absoluteVerseNumber = startVerseNumber + index;
                currentVerse.content = verseText;
                text.push_back(currentVerse);
            }

            lastKey = currentKey;
            lastBookName = currentBookName;
            lastChapter = currentChapter;
            module->increment();
            index++;
        }
    }

    return text;
}

string TextProcessor::getBookIntroduction(string moduleName, string bookCode)
{
    string bookIntroText = "";
    string filteredText = "";
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        VerseKey verseKey(bookCode.c_str());

        // Include chapter/book/testament/module intros
        verseKey.setIntros(true);
        verseKey.setChapter(0);
        verseKey.setVerse(0);
        module->setKey(verseKey);

        bookIntroText = string(module->getRawEntry());
        StringHelper::trim(bookIntroText);

        static regex titleStartElementFilter = regex("<title");
        static regex titleEndElementFilter = regex("</title>");
        static regex noteStartElementFilter = regex("<note");
        static regex noteEndElementFilter = regex("</note>");
        static regex headStartElementFilter = regex("<head");
        static regex headEndElementFilter = regex("</head>");
        static regex chapterDivFilter = regex("<div type=\"chapter\" n=\"[0-9]{1}\" id=\"[-A-Z0-9]{1,8}\">");

        filteredText = bookIntroText;
        filteredText = regex_replace(filteredText, titleStartElementFilter, "<div class=\"sword-markup sword-book-title\"");
        filteredText = regex_replace(filteredText, titleEndElementFilter, "</div>");
        filteredText = regex_replace(filteredText, noteStartElementFilter, "<div class=\"sword-markup sword-note\"");
        filteredText = regex_replace(filteredText, noteEndElementFilter, "</div>");
        filteredText = regex_replace(filteredText, headStartElementFilter, "<div class=\"sword-markup sword-head\"");
        filteredText = regex_replace(filteredText, headEndElementFilter, "</div>");
        filteredText = regex_replace(filteredText, chapterDivFilter, "");
    }

    return filteredText;
}

string TextProcessor::replaceSpacesInStrongs(const string& text)
{
    string input = text;
    static regex strongsWText = regex(">[^<]*</w>");
    static std::regex space(" ");
    smatch m;
    string filteredText;

    // Search for Strongs pattern and then replace all spaces within each occurance
    while (std::regex_search(input, m, strongsWText)) {
        filteredText += m.prefix();
        filteredText += std::regex_replace(m[0].str(), space, "&nbsp;");
        input = m.suffix();
    }
    filteredText += input;

    return filteredText;
}


bool TextProcessor::moduleHasStrongsZeroPrefixes(sword::SWModule* module)
{
    string key = "Gen 1:1";
    module->setKey(key.c_str());
    this->enableMarkup();
    string verseText = this->getCurrentVerseText(module, true);
    
    return verseText.find("strong:H0") != string::npos;
}

bool TextProcessor::isModuleReadable(sword::SWModule* module, std::string key)
{
    module->setKey(key.c_str());
    string verseText = this->getCurrentVerseText(module, false);
    return verseText.size() > 0;
}

StrongsEntry* TextProcessor::getStrongsEntry(string key)
{
    SWModule* module = 0;
    char strongsType = key[0];

    if (strongsType == 'H') {
        module = this->_strongsHebrew;
    } else if (strongsType == 'G') {
        module = this->_strongsGreek;
    } else {
        return 0;
    }

    if (module == 0) {
        cerr << "No valid Strong's module available!" << endl;
        return 0;
    }

    StrongsEntry* entry = StrongsEntry::getStrongsEntry(module, key);
    return entry;
}