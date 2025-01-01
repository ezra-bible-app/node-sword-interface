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

#include <sstream>
#include <string>
#include <algorithm>
#include <regex>
#include <stdlib.h>

#include <swmodule.h>
#include "strongs_entry.hpp"
#include "string_helper.hpp"

using namespace std;
using namespace sword;

StrongsReference::StrongsReference(string text)
{
    this->text = text;
    this->key = this->parseKey(text);
}

string StrongsReference::parseKey(string text)
{
    string key = "";

    // A reference looks like this:
    //see GREEK for 2767
    //
    // To parse the actual key we first split the string by space, getting the four literals as a result.
    vector<string> splittedReference = StringHelper::split(text, " ");

    if (splittedReference.size() == 4) {
        // The reference language is the first character of the second literal
        char referenceLang = splittedReference[1][0];

        // The key is the last / 4th literal
        // We convert it to a number to get rid of unnecessary leading 0s
        unsigned int numericKey = atoi(splittedReference[3].c_str());
        // Then we convert it back into a string
        string stringKey = to_string(numericKey);

        key = referenceLang + stringKey;
    } else {
        key = "";
    }

    return key;
}

/*
 * Checks whether the key is valid
 */
bool StrongsReference::hasValidKey()
{
    return StrongsEntry::isValidStrongsKey(this->key);
}

StrongsEntry::StrongsEntry(string key, string rawEntry, string moduleVersion)
{
    this->key = key;

    char moduleMinorVersion = moduleVersion[0];

    if (moduleMinorVersion == '1') {
        this->parseFromVersion1RawEntry(rawEntry);
    } else {
        this->parseFromVersion2RawEntry(rawEntry);
    }
}

bool StrongsEntry::isValidStrongsKey(std::string key)
{
    if (key.size() < 2) {
        return false;
    }

    char dictionary = key[0];
    static const int HEBREW_MAX = 8674;
    static const int GREEK_MAX = 5624;
    int maxStrongsNumber;

    if (dictionary == 'H') {
        maxStrongsNumber = HEBREW_MAX;
    } else if (dictionary == 'G') {
        maxStrongsNumber = GREEK_MAX;
    } else {
        // Unknown dictionary type
        return false;
    }

    string strongsNumberString = key.substr(1);
    int strongsNumber = atoi(strongsNumberString.c_str());
    return (strongsNumber > 0 &&
            strongsNumber <= maxStrongsNumber);
}

StrongsEntry* StrongsEntry::getStrongsEntry(SWModule* module, string key)
{
    if (module == 0) {
        return 0;
    }

    if (!StrongsEntry::isValidStrongsKey(key)) {
        return 0;
    }

    string moduleVersion = module->getConfigEntry("Version");

    // Cut off the first character (H or G), since the Sword engine uses the actual number strings as the key for Strong's
    string strongsNumberString = key.substr(1);
    module->setKey(strongsNumberString.c_str());

    StrongsEntry* strongsEntry = new StrongsEntry(key, module->getRawEntry(), moduleVersion);

    return strongsEntry;
}

void StrongsEntry::parseFirstLine(string firstLine)
{
    // The first line looks like this:
    // 2766  keramos  ker'-am-os
    
    // Cut off white space on the left side
    StringHelper::ltrim(firstLine);

    // Make white spaces uniform
    static regex whiteSpace = regex("\\s+");
    firstLine = regex_replace(firstLine, whiteSpace, " ");

    // We will now split it and then parse the transcription (2nd column) and phonetic transcription (3rd column)
    vector<string> firstLineEntries = StringHelper::split(firstLine, " ");
    if (firstLine.size() >= 2) this->transcription = firstLineEntries[1];
    if (firstLine.size() >= 3) this->phoneticTranscription = firstLineEntries[2];
}

void StrongsEntry::eraseEmptyLines(vector<string>& lines)
{
    string currentLine = "";
    int maxIterations = 50;

    for (int i = 0; i < maxIterations && currentLine == "" && lines.size() > 0; i++) {
        currentLine = lines[0];
        StringHelper::trim(currentLine, " ");
        if (currentLine == "") {
            lines.erase(lines.begin(), lines.begin() + 1);
        } else {
            // Break when reaching the first line with actual content
            break;
        }
    }
}

void StrongsEntry::parseDefinitionAndReferences(vector<string>& lines)
{
    vector<StrongsReference> references;
    vector<string> rawReferences;
    stringstream definition;
    static regex doubleLineBreak = regex("\n\n");

    for (unsigned int i = 0; i < lines.size(); i++) {
        string currentLine = lines[i];
        if (currentLine.substr(0,5) == " see " || currentLine.substr(0,4) == "see ") {
            StringHelper::trim(currentLine);
            StrongsReference reference(currentLine);
            // Only put the current line into the list of references if it's not already in there
            // We do this, because there are Strong's like H3069 (Yhovih) that contain duplicates
            if (find(rawReferences.begin(), rawReferences.end(), currentLine) == rawReferences.end()) {
                rawReferences.push_back(currentLine);

                // We only take references with a valid (non-empty) key
                if (reference.hasValidKey()) {
                    references.push_back(reference);
                }
            }
        } else {
            definition << currentLine << "\n";
        }
    }

    string definitionString = definition.str();
    // Clean up double line breaks
    definitionString = regex_replace(definitionString, doubleLineBreak, "\n");
    // Cut off white space in beginning and end (if any)
    StringHelper::trim(definitionString);

    this->definition = definitionString;
    this->references = references;
}

void StrongsEntry::parseFromVersion1RawEntry(string rawEntry)
{
    this->rawEntry = rawEntry;

    std::size_t firstCommaPosition = rawEntry.find_first_of(",");
    std::size_t firstLineBreakPosition = rawEntry.find_first_of("\n");

    // If the first line contains a comma we replace it with a line break
    // An example for this is G2147
    // This is necessary so that the parsing works in this specific case
    if (firstCommaPosition < firstLineBreakPosition) {
        this->rawEntry.replace(firstCommaPosition, 1, "\n");
    }

    vector<string> allLines = StringHelper::split(this->rawEntry, "\n");
    if (allLines.size() == 0) {
        return;
    }

    this->parseFirstLine(allLines[0]);
    // Erase the first line
    allLines.erase(allLines.begin(), allLines.begin() + 1);
    // Erase all empty lines, this leaves us with the definition and references
    this->eraseEmptyLines(allLines);
    this->parseDefinitionAndReferences(allLines);
}

void StrongsEntry::parseFromVersion2RawEntry(string rawEntry)
{
    this->rawEntry = rawEntry;

    vector<string> allLines = StringHelper::split(this->rawEntry, "\n");
    if (allLines.size() == 0) {
        return;
    }

    string details = allLines[0];
    string phoneticTranscription = details;

    // Parse the transcription
    string transcriptionTag = "<orth rend=\"bold\" type=\"trans\">";
    string transcriptionEndTag = "</orth>";
    this->transcription = this->parseFromVersion2Element(details, transcriptionTag, transcriptionEndTag);

    // Parse the phonetic transcription
    string phoneticTranscriptionTag = "<pron rend=\"italic\">{";
    string phoneticTranscriptionEndTag = "}";
    this->phoneticTranscription = this->parseFromVersion2Element(phoneticTranscription, phoneticTranscriptionTag, phoneticTranscriptionEndTag);

    // Parse the definition
    string definition = rawEntry;
    definition = this->parseFromVersion2Element(rawEntry, "<def>", "</def>");
    string references = definition;

    std::size_t lineBreakPosition = definition.find("<lb");
    string defEndTag = "</def>";
    std::size_t defEndTagPosition = definition.find(defEndTag);

    StringHelper::trim(definition);

    string lineBreak = "<lb/>";
    lineBreakPosition = definition.find(lineBreak);

    if (lineBreakPosition != string::npos) {
        definition.erase(lineBreakPosition, string::npos);
    }

    // Parse the references
    lineBreakPosition = references.find(lineBreak);

    if (lineBreakPosition != string::npos) {
        references.erase(0, lineBreakPosition);
        
        defEndTagPosition = references.find(defEndTag);

        if (defEndTagPosition != string::npos) {
            references.erase(defEndTagPosition, string::npos);
        }

        references.erase(0, lineBreak.size());
    }

    StringHelper::trim(references);
    vector<string> referenceLines = StringHelper::split(references, "<lb/> ");
    this->parseDefinitionAndReferences(referenceLines);

    // Store definition from variable above.
    this->definition = definition;
}

string StrongsEntry::parseFromVersion2Element(string rawEntry, string startTag, string endTag)
{
    std::size_t startTagPosition = rawEntry.find(startTag);
    if (startTagPosition != string::npos) {
        rawEntry.erase(0, startTagPosition);
    }

    std::size_t endTagPosition = rawEntry.find(endTag);
    
    if (endTagPosition != string::npos) {
        rawEntry.erase(endTagPosition, string::npos);
    }

    if (startTag.size() <= rawEntry.size()) {
        rawEntry.erase(0, startTag.size());
    }

    return rawEntry;
}
