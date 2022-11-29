/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2022 Tobias Klein <contact@tklein.info>

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
#include <sstream>
#include <string>
#include <regex>

// Sword includes
#include <versekey.h>
#include <listkey.h>
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
    this->_markupEnabled = false;
    this->_rawMarkupEnabled = false;
}

string TextProcessor::getFilteredText(const string& text, int chapter, int verseNr, bool hasStrongs, bool hasInconsistentClosingEndDivs)
{
    static string chapterFilter = "<chapter";
    static regex pbElement = regex("<pb .*?/> ");

    static string lbBeginParagraph = "<lb type=\"x-begin-paragraph\"/>";
    static string lbEndParagraph = "<lb type=\"x-end-paragraph\"/>";
    static string lbElementFilter = "<lb ";
    static string lElementFilter = "<l ";
    static string lgElementFilter = "<lg ";
    static string noteStartElementFilter = "<note";
    static string noteEndElementFilter = "</note>";
    static string headStartElementFilter = "<head";
    static string headEndElementFilter = "</head>";
    static string appStartElementFilter = "<app";
    static string appEndElementFilter = "</app>";
    static string scripRefStartElementFilter = "<scripRef";
    static string scripRefEndElementFilter = "</scripRef>";
    static string rtxtStartElementFilter1 = "<rtxt type=";
    static string rtxtStartElementFilter2 = "<rtxt rend=";
    static string rtxtEndElementFilter = "</rtxt>";
    static string pbElementFilter = "<pb";
    static string quoteJesusElementFilter = "<q marker=\"\" who=\"Jesus\">";
    static string quoteElementFilter = "<q ";
    static string quoteEndElementFilter = "</q>";
    static string titleStartElementFilter = "<title";
    static string titleEndElementFilter = "</title>";
    static string segEndElementFilter = "</seg>";
    static string divTitleElementFilter = "<div class=\"title\"";
    static string secHeadClassFilter = "class=\"sechead\"";
    static string divMilestoneFilter = "<div type=\"x-milestone\"";
    static string xBrFilter = "x-br\"/>";
    static string divSIDFilter = "<div sID=";
    static string divEIDFilter = "<div eID=";
    static string divineNameStartElement = "<divineName>";
    static string divineNameEndElement = "</divineName>";
    static string strongsWElement = "<w lemma=";

    static regex milestoneFilter = regex("<milestone.*?/>");
    static regex segStartElementFilter = regex("<seg.*?>");
    static regex divSectionElementFilter = regex("<div type=\"section\".*?>");
    static regex selfClosingElement = regex("(<)([wdiv]{1,3}) ([\\w:=\"\\- ]*?)(/>)");

    static string fullStopWithoutSpace = ".<";
    static string questionMarkWithoutSpace = "?<";
    static string exclamationMarkWithoutSpace = "!<";
    static string commaWithoutSpace = ",<";
    static string semiColonWithoutSpace = ";<";
    static string colonWithoutSpace = ":<";

    string filteredText = text;

    // Remove the first pbElement, because it prevents correctly replacing the first note in the next step
    filteredText = regex_replace(filteredText, pbElement, "");

    // Remove <note type="variant"> if it occurs in the beginning of the verse (applicable for NA28), because it has
    // been observed that the note is not properly closed.
    static string noteTypeVariant = "<note type=\"variant\">";
    if (StringHelper::hasBeginning(filteredText, noteTypeVariant)) {
        filteredText.replace(0, noteTypeVariant.length(), "");
    }

    this->findAndReplaceAll(filteredText, chapterFilter, "<chapter class=\"sword-markup sword-chapter\"");
    this->findAndReplaceAll(filteredText, lbBeginParagraph, "");
    this->findAndReplaceAll(filteredText, lbEndParagraph, "&nbsp;<div class=\"sword-markup sword-paragraph-end\"><br></div>");
    this->findAndReplaceAll(filteredText, lbElementFilter, "<div class=\"sword-markup sword-lb\" ");
    this->findAndReplaceAll(filteredText, lElementFilter, "<div class=\"sword-markup sword-l\" ");
    this->findAndReplaceAll(filteredText, lgElementFilter, "<div class=\"sword-markup sword-lg\" ");
    this->findAndReplaceAll(filteredText, noteStartElementFilter, "<div class=\"sword-markup sword-note\" ");
    this->findAndReplaceAll(filteredText, noteEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, headStartElementFilter, "<div class=\"sword-markup sword-head\" ");
    this->findAndReplaceAll(filteredText, headEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, appStartElementFilter, "<div class=\"sword-markup sword-app\" ");
    this->findAndReplaceAll(filteredText, appEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, scripRefStartElementFilter, "<div class=\"sword-markup sword-scripref\" ");
    this->findAndReplaceAll(filteredText, scripRefEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, rtxtStartElementFilter1, "<div class=\"sword-markup sword-rtxt\" type=");
    this->findAndReplaceAll(filteredText, rtxtStartElementFilter2, "<div class=\"sword-markup sword-rtxt\" rend=");
    this->findAndReplaceAll(filteredText, rtxtEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, pbElementFilter, "<pb class=\"sword-markup sword-pb\"");
    filteredText = regex_replace(filteredText, milestoneFilter, "");
    filteredText = regex_replace(filteredText, segStartElementFilter, "");
    filteredText = regex_replace(filteredText, divSectionElementFilter, "");

    stringstream sectionTitleElement;
    sectionTitleElement << "<div class=\"sword-markup sword-section-title\" ";
    sectionTitleElement << "chapter=\"" << chapter << "\" ";
    sectionTitleElement << "verse=\"" << verseNr << "\"";
    this->findAndReplaceAll(filteredText, titleStartElementFilter, sectionTitleElement.str());
    this->findAndReplaceAll(filteredText, divTitleElementFilter, sectionTitleElement.str());

    stringstream secHead;
    secHead << "class=\"sword-markup sword-section-title\" ";
    secHead << "chapter=\"" << chapter << "\"";
    this->findAndReplaceAll(filteredText, secHeadClassFilter, secHead.str());

    if (filteredText.find("subType=\"x-Chapter") != string::npos ||
        filteredText.find("type=\"chapter") != string::npos) {

      static string swordSectionTitle = "sword-section-title";
      static string swordSectionTitleChapter = "sword-section-title sword-chapter-title";
      this->findAndReplaceAll(filteredText, swordSectionTitle, swordSectionTitleChapter);
    }

    this->findAndReplaceAll(filteredText, titleEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, segEndElementFilter, "");
    this->findAndReplaceAll(filteredText, divMilestoneFilter, "<div class=\"sword-markup sword-x-milestone\"");
    this->findAndReplaceAll(filteredText, xBrFilter, "x-br\"/> ");
    this->findAndReplaceAll(filteredText, divSIDFilter, "<div class=\"sword-markup sword-sid\" sID=");
    this->findAndReplaceAll(filteredText, divEIDFilter, "<div class=\"sword-markup sword-eid\" eID=");
    this->findAndReplaceAll(filteredText, quoteJesusElementFilter, "<div class=\"sword-markup sword-quote-jesus\">");
    this->findAndReplaceAll(filteredText, quoteElementFilter, "<div class=\"sword-markup sword-quote\" ");
    this->findAndReplaceAll(filteredText, quoteEndElementFilter, "</div>");
    this->findAndReplaceAll(filteredText, divineNameStartElement, "");
    this->findAndReplaceAll(filteredText, divineNameEndElement, "");
    this->findAndReplaceAll(filteredText, strongsWElement, "<w class=");

    filteredText = regex_replace(filteredText, selfClosingElement, "<$2 $3></$2>");

    this->findAndReplaceAll(filteredText, fullStopWithoutSpace, ". <");
    this->findAndReplaceAll(filteredText, questionMarkWithoutSpace, "? <");
    this->findAndReplaceAll(filteredText, exclamationMarkWithoutSpace, "! <");
    this->findAndReplaceAll(filteredText, commaWithoutSpace, ", <");
    this->findAndReplaceAll(filteredText, semiColonWithoutSpace, "; <");
    this->findAndReplaceAll(filteredText, colonWithoutSpace, ": <");

    if (hasInconsistentClosingEndDivs) {
        int numberOfOpeningDivs = StringHelper::numberOfSubstrings(filteredText, "<div");
        int numberOfClosingDivs = StringHelper::numberOfSubstrings(filteredText, "</div>");

        // Remove the last closing div(s) if the number of closing divs is higher than the number of opening divs
        if (numberOfClosingDivs > numberOfOpeningDivs) {
            const string closingDiv = "</div>";
            unsigned int diff = numberOfClosingDivs - numberOfOpeningDivs;

            for (unsigned int i = 0; i < diff; i++) {
                size_t lastClosingDivOffset = filteredText.rfind(closingDiv);
                filteredText.erase(lastClosingDivOffset, closingDiv.length());
            }
        }

        // Add closing div(s) if the number of closing divs is smaller than the number of opening divs
        if (numberOfClosingDivs < numberOfOpeningDivs) {
            const string closingDiv = "</div>";
            unsigned int diff = numberOfOpeningDivs - numberOfClosingDivs;

            for (unsigned int i = 0; i < diff; i++) {
                size_t lastClosingDivOffset = filteredText.rfind(closingDiv);
                filteredText.insert(lastClosingDivOffset + closingDiv.length(), closingDiv);
            }
        }
    }

    if (hasStrongs) {
        filteredText = this->replaceSpacesInStrongs(filteredText);
    }

    return filteredText;
}

string TextProcessor::getCurrentChapterHeading(sword::SWModule* module)
{
    string currentModuleName = string(module->getName());
    string chapterHeading = "";
    VerseKey currentVerseKey = module->getKey();
    int currentChapter = currentVerseKey.getChapter();
    int currentVerseNr = currentVerseKey.getVerse();

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

    if (this->_markupEnabled && !this->_rawMarkupEnabled) {
        // The chapter headings in the ISV are screwed up somehow for 1:1
        // Therefore we do not render chapter headings for the first verse of the book in this case.
        if (currentChapter == 1 && currentVerseNr == 1 && currentModuleName == "ISV") {
            chapterHeading = "";
        } else {
            chapterHeading = this->getFilteredText(chapterHeading, currentChapter, currentVerseNr);
        }
    }

    return chapterHeading;
}

string TextProcessor::getCurrentVerseText(sword::SWModule* module, bool hasStrongs, bool hasInconsistentClosingEndDivs, bool forceNoMarkup)
{
    string verseText;
    string filteredText;

    if (this->_markupEnabled && !forceNoMarkup) {
        VerseKey currentVerseKey = module->getKey();
        int currentChapter = currentVerseKey.getChapter();
        int currentVerseNr = currentVerseKey.getVerse();
        verseText = string(module->getRawEntry());
        StringHelper::trim(verseText);
        filteredText = verseText;

        if (!this->_rawMarkupEnabled) {
            filteredText = this->getFilteredText(verseText, currentChapter, currentVerseNr, hasStrongs, hasInconsistentClosingEndDivs);
        }
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

string TextProcessor::getBookFromReference(string reference)
{
    VerseKey key(reference.c_str());
    return string(key.getOSISBookName());
}

vector<string> TextProcessor::getBookListFromReferences(vector<string>& references)
{
    vector<string> bookList;

    for (unsigned int i = 0; i < references.size(); i++) {
        string currentReference = references[i];
        string book = this->getBookFromReference(currentReference);

        if (find(bookList.begin(), bookList.end(), book) == bookList.end()) {
            bookList.push_back(book);
        }
    }

    return bookList;
}

vector<Verse> TextProcessor::getVersesFromReferences(string moduleName, vector<string>& references)
{
    vector<Verse> verses;
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    vector<string> bookList = this->getBookListFromReferences(references);
    map<string, int> absoluteVerseNumbers = this->_moduleHelper.getAbsoluteVerseNumberMap(module, bookList);
    bool moduleMarkupIsBroken = this->_moduleHelper.isBrokenMarkupModule(moduleName);
    bool hasInconsistentClosingEndDivs = this->_moduleHelper.isInconsistentClosingEndDivModule(moduleName);

    for (unsigned int i = 0; i < references.size(); i++) {
        string currentReference = references[i];
        string currentVerseText = "";

        module->setKey(currentReference.c_str());
        bool entryExisting = module->hasEntry(module->getKey());

        if (entryExisting) {
          currentVerseText = this->getCurrentVerseText(module, false, hasInconsistentClosingEndDivs, moduleMarkupIsBroken);
        }

        Verse currentVerse;
        currentVerse.reference = module->getKey()->getShortText();
        currentVerse.absoluteVerseNumber = absoluteVerseNumbers[currentVerse.reference];
        currentVerse.content = currentVerseText;
        verses.push_back(currentVerse);
    }

    return verses;
}

vector<string> TextProcessor::getReferencesFromReferenceRange(std::string referenceRange)
{
    vector<string> references;
    VerseKey parser;
    ListKey result;

    result = parser.parseVerseList(referenceRange.c_str(), parser, true);

    for (result = TOP; !result.popError(); result++) {
        VerseKey currentKey(result.getShortText());
        references.push_back(string(currentKey.getOSISRef()));
    }

    return references;
}

vector<Verse> TextProcessor::getText(string moduleName, string key, QueryLimit queryLimit, int startVerseNumber, int verseCount)
{
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);
    string lastKey;
    int index = 0;
    string lastBookName = "";
    int lastChapter = -1;
    bool currentBookExisting = true;
    bool moduleMarkupIsBroken = this->_moduleHelper.isBrokenMarkupModule(moduleName);
    bool hasInconsistentClosingEndDivs = this->_moduleHelper.isInconsistentClosingEndDivModule(moduleName);

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
            // We only add it when we're looking at the first verse of a chapter
            // and if the module markup is not broken
            // and if the requested verse count is more than one or the default (-1 / all verses).
            if (firstVerseInChapter && !moduleMarkupIsBroken && (verseCount > 1 || verseCount == -1)) {
                string chapterHeading = this->getCurrentChapterHeading(module);
                verseText += chapterHeading;
            }

            // Current verse text
            verseText += this->getCurrentVerseText(module,
                                                   hasStrongs,
                                                   hasInconsistentClosingEndDivs,
                                                   // Note that if markup is broken this will enforce
                                                   // the usage of the "stripped" / non-markup variant of the text
                                                   moduleMarkupIsBroken);

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
        module->setKeyText(bookCode.c_str());
        VerseKey *verseKey = (VerseKey *)module->getKey();

        // Include chapter/book/testament/module intros
        verseKey->setIntros(true);
        verseKey->setChapter(0);
        verseKey->setVerse(0);
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

vector<Verse> TextProcessor::getBookHeaderList(string moduleName, string bookCode, bool withAbsoluteVerseNumbers)
{
    vector<Verse> headerList;
    SWModule* module = this->_moduleStore.getLocalModule(moduleName);

    if (module == 0) {
        cerr << "getBookHeaderList: getLocalModule returned zero pointer for " << moduleName << endl;
    } else {
        map<string, int> absoluteVerseNumbers;
        
        if (withAbsoluteVerseNumbers) {
            absoluteVerseNumbers = this->_moduleHelper.getAbsoluteVerseNumberMap(module, { bookCode });
        }

        ListKey scopeList = VerseKey().parseVerseList(bookCode.c_str(), "", true);
        SWKey* scope = &scopeList;

        ListKey resultKey = module->search("/Heading", SWModule::SEARCHTYPE_ENTRYATTR, 0, scope);

        static string titleStartElementFilter = "<title";
        static string titleEndElementFilter = "</title>";
        static string divTitleElementFilter = "<div class=\"title\"";
        static string secHeadClassFilter = "class=\"sechead\"";

        for (resultKey = TOP; !resultKey.popError(); resultKey++) {
            module->setKey(resultKey);
            module->renderText();

            VerseKey currentKey(resultKey.getShortText());

            // get both Preverse and Interverse Headings and just merge them into the same map
            AttributeValue headings = module->getEntryAttributes()["Heading"]["Preverse"];
            AttributeValue interverseHeadings = module->getEntryAttributes()["Heading"]["Interverse"];
            headings.insert(interverseHeadings.begin(), interverseHeadings.end());

            stringstream currentHeaderTextStream;

            Verse currentHeader;
            currentHeader.reference = module->getKey()->getShortText();
            currentHeader.absoluteVerseNumber = -1;
            if (withAbsoluteVerseNumbers) {
                currentHeader.absoluteVerseNumber = absoluteVerseNumbers[currentHeader.reference];
            }

            for (AttributeValue::const_iterator it = headings.begin(); it != headings.end(); ++it) {
                currentHeaderTextStream << it->second << endl;
            }

            string currentHeaderText = currentHeaderTextStream.str();
            stringstream sectionTitleElement;
            sectionTitleElement << "<div class=\"sword-markup sword-section-title\" ";
            sectionTitleElement << "chapter=\"" << currentKey.getChapter() << "\"";
            this->findAndReplaceAll(currentHeaderText, titleStartElementFilter, sectionTitleElement.str());
            this->findAndReplaceAll(currentHeaderText, divTitleElementFilter, sectionTitleElement.str());

            stringstream secHead;
            secHead << "class=\"sword-markup sword-section-title\" ";
            secHead << "chapter=\"" << currentKey.getChapter() << "\"";
            this->findAndReplaceAll(currentHeaderText, secHeadClassFilter, secHead.str());
            this->findAndReplaceAll(currentHeaderText, titleEndElementFilter, "</div>");

            currentHeader.content = currentHeaderText;
            headerList.push_back(currentHeader);
        }
    }

    return headerList;
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

    bool previousMarkupSetting = this->_markupEnabled;    
    this->enableMarkup();
    string verseText = this->getCurrentVerseText(module, true);
    this->_markupEnabled = previousMarkupSetting;
    
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
        module = this->_moduleStore.getLocalModule("StrongsHebrew");
    } else if (strongsType == 'G') {
        module = this->_moduleStore.getLocalModule("StrongsGreek");
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

unsigned int TextProcessor::findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr)
{
    unsigned int count = 0;

    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while(pos != std::string::npos)
    {
        count++;
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + replaceStr.size());
    }

    return count;
}
