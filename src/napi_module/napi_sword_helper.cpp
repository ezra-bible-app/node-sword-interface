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


#include <swmodule.h>
#include "napi_sword_helper.hpp"
#include "string_helper.hpp"
#include "module_helper.hpp"
#include "module_store.hpp"
#include "common_defs.hpp"

using namespace std;
using namespace sword;

Napi::Array NapiSwordHelper::getNapiArrayFromStringVector(const Napi::Env& env, vector<string>& stringVector)
{
    Napi::Array napiArray = Napi::Array::New(env, stringVector.size());

    for (unsigned int i = 0; i < stringVector.size(); i++) {
        napiArray.Set(i, stringVector[i]);
    }

    return napiArray;
}

Napi::Array NapiSwordHelper::getNapiVerseObjectsFromRawList(const Napi::Env& env, string moduleCode, vector<Verse>& verses)
{
    Napi::Array versesArray = Napi::Array::New(env, verses.size());

    for (unsigned int i = 0; i < verses.size(); i++) {
        Verse currentRawVerse = verses[i];
        Napi::Object verseObject = Napi::Object::New(env);
        this->verseTextToNapiObject(moduleCode, currentRawVerse, verseObject);
        versesArray.Set(i, verseObject);
    }

    return versesArray;
}

void NapiSwordHelper::swordModuleToNapiObject(const Napi::Env& env, SWModule* swModule, Napi::Object& object)
{
    if (swModule == 0) {
        cerr << "swModule is 0! Cannot run conversion to napi object!" << endl;
        return;
    }

    object["name"] = swModule->getName();
    object["type"] = swModule->getType();
    object["description"] = swModule->getDescription();
    object["language"] = swModule->getLanguage();
    object["location"] = this->_moduleStore.getModuleDataPath(swModule);
  
    object["abbreviation"] = this->getConfigEntry(swModule, "Abbreviation", env);
    object["about"] = this->getConfigEntry(swModule, "About", env);
    object["distributionLicense"] = this->getConfigEntry(swModule, "DistributionLicense", env);
    object["shortCopyright"] = this->getConfigEntry(swModule, "ShortCopyright", env);
    object["version"] = this->getConfigEntry(swModule, "Version", env);

    if (swModule->getConfigEntry("Direction")) {
        string direction = string(swModule->getConfigEntry("Direction"));
        object["isRightToLeft"] = Napi::Boolean::New(env, (direction == "RtoL"));
    } else {
        object["isRightToLeft"] = Napi::Boolean::New(env, false);
    }

    if (swModule->getConfigEntry("InstallSize")) {
        int moduleSize = std::stoi(string(swModule->getConfigEntry("InstallSize")));
        object["size"] = Napi::Number::New(env, moduleSize);
    } else {
        object["size"] = Napi::Number::New(env, -1);
    }

    bool moduleIsLocked = swModule->getConfigEntry("CipherKey");
    
    object["locked"] = Napi::Boolean::New(env, moduleIsLocked);
    if (moduleIsLocked && swModule->getConfigEntry("UnlockInfo")) {
        object["unlockInfo"] = swModule->getConfigEntry("UnlockInfo");
    } else {
        object["unlockInfo"] = "";
    }

    object["inUserDir"] = Napi::Boolean::New(env, this->_moduleStore.isModuleInUserDir(swModule));
    object["hasStrongs"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasGlobalOption(swModule, "Strongs"));
    object["hasFootnotes"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasGlobalOption(swModule, "Footnotes"));
    object["hasHeadings"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasGlobalOption(swModule, "Headings"));
    object["hasRedLetterWords"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasGlobalOption(swModule, "RedLetter"));
    object["hasCrossReferences"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasGlobalOption(swModule, "Scripref"));

    object["hasGreekStrongsKeys"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasFeature(swModule, "GreekDef"));
    object["hasHebrewStrongsKeys"] = Napi::Boolean::New(env, this->_moduleHelper.moduleHasFeature(swModule, "HebrewDef"));
}

Napi::String NapiSwordHelper::getConfigEntry(sword::SWModule* swModule, std::string key, const Napi::Env& env)
{
    std::string configEntry = "";

    if (swModule == 0) {
        cerr << "swModule is 0! Cannot get config entry!" << endl;
    } else {
        if (swModule->getConfigEntry(key.c_str())) {
            configEntry = swModule->getConfigEntry(key.c_str());
        }
    }

    Napi::String napiConfigEntry = Napi::String::New(env, configEntry);
    return napiConfigEntry;
}

void NapiSwordHelper::verseTextToNapiObject(string moduleCode, Verse rawVerse, Napi::Object& object)
{
    string reference = rawVerse.reference;
    string verseText = rawVerse.content;

    vector<string> splittedReference = StringHelper::split(reference, " ");
    string book = splittedReference[0];
    string chapterVerseReference = splittedReference[1];

    vector<string> splittedChapterVerseReference = StringHelper::split(chapterVerseReference, ":");
    int chapter = std::stoi(splittedChapterVerseReference[0]);
    int verseNr = std::stoi(splittedChapterVerseReference[1]);

    object["moduleCode"] = moduleCode;
    object["bibleBookShortTitle"] = book;
    object["chapter"] = chapter;
    object["verseNr"] = verseNr;
    object["absoluteVerseNr"] = rawVerse.absoluteVerseNumber;
    object["content"] = verseText;
}

void NapiSwordHelper::strongsEntryToNapiObject(const Napi::Env& env, StrongsEntry* strongsEntry, Napi::Object& object)
{
    if (strongsEntry == 0) {
        cerr << "strongsEntry is 0! Cannot run conversion to napi object!" << endl;
        return;
    }

    object["rawEntry"] = strongsEntry->rawEntry;
    object["key"] = strongsEntry->key;
    object["transcription"] = strongsEntry->transcription;
    object["phoneticTranscription"] = strongsEntry->phoneticTranscription;
    object["definition"] = strongsEntry->definition;

    Napi::Array referencesArray = Napi::Array::New(env, strongsEntry->references.size());
    for (unsigned int i = 0; i < strongsEntry->references.size(); i++) {
        StrongsReference currentReference = strongsEntry->references[i];
        Napi::Object referenceObject = Napi::Object::New(env);
        
        referenceObject["text"] = currentReference.text;
        referenceObject["key"] = currentReference.key;

        referencesArray.Set(i, referenceObject);
    }

    object["references"] = referencesArray;
}
