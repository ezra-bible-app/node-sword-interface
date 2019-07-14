/* This file is part of node-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@ezra-project.net>

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

#include <time.h>
#include <sstream>
#include <thread>
#include <iostream>

#include "swmodule.h"
#include "node_sword_interface.hpp"
#include "node_sword_interface_worker.hpp"

using namespace std;
using namespace sword;

Napi::FunctionReference NodeSwordInterface::constructor;

Napi::Object NodeSwordInterface::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NodeSwordInterface", {
        InstanceMethod("refreshRepositoryConfig", &NodeSwordInterface::refreshRepositoryConfig),
        InstanceMethod("refreshRemoteSources", &NodeSwordInterface::refreshRemoteSources),
        InstanceMethod("repositoryConfigExisting", &NodeSwordInterface::repositoryConfigExisting),
        InstanceMethod("getRepoNames", &NodeSwordInterface::getRepoNames),
        InstanceMethod("getAllRepoModules", &NodeSwordInterface::getAllRepoModules),
        InstanceMethod("getRepoModulesByLang", &NodeSwordInterface::getRepoModulesByLang),
        InstanceMethod("getAllLocalModules", &NodeSwordInterface::getAllLocalModules),
        InstanceMethod("isModuleInUserDir", &NodeSwordInterface::isModuleInUserDir),
        InstanceMethod("getRepoLanguages", &NodeSwordInterface::getRepoLanguages),
        InstanceMethod("getRepoTranslationCount", &NodeSwordInterface::getRepoTranslationCount),
        InstanceMethod("getRepoLanguageTranslationCount", &NodeSwordInterface::getRepoLanguageTranslationCount),
        InstanceMethod("getRepoModule", &NodeSwordInterface::getRepoModule),
        InstanceMethod("getModuleDescription", &NodeSwordInterface::getModuleDescription),
        InstanceMethod("getLocalModule", &NodeSwordInterface::getLocalModule),
        InstanceMethod("enableMarkup", &NodeSwordInterface::enableMarkup),
        InstanceMethod("getBookText", &NodeSwordInterface::getBookText),
        InstanceMethod("getBibleText", &NodeSwordInterface::getBibleText),
        InstanceMethod("installModule", &NodeSwordInterface::installModule),
        InstanceMethod("uninstallModule", &NodeSwordInterface::uninstallModule)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("NodeSwordInterface", func);
    return exports;
}

NodeSwordInterface::NodeSwordInterface(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeSwordInterface>(info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  this->_swordFacade = new SwordFacade();
}

Napi::Value NodeSwordInterface::refreshRepositoryConfig(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsFunction()) {
      Napi::TypeError::New(env, "Function expected as first argument").ThrowAsJavaScriptException();
  }

  Napi::Function callback = info[0].As<Napi::Function>();
  NodeSwordInterfaceWorker* worker = new NodeSwordInterfaceWorker(this->_swordFacade, "refreshRepositoryConfig", {}, callback);
  worker->Queue();
  return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::refreshRemoteSources(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 2) {
      Napi::TypeError::New(env, "Expected 2 parameters!").ThrowAsJavaScriptException();
    } else if (!info[0].IsBoolean()) {
        Napi::TypeError::New(env, "Boolean expected as first argument").ThrowAsJavaScriptException();
    } else if (!info[1].IsFunction()) {
        Napi::TypeError::New(env, "Function expected as second argument").ThrowAsJavaScriptException();
    }

    Napi::Boolean force = info[0].As<Napi::Boolean>();
    Napi::Function callback = info[1].As<Napi::Function>();

    bool isForced = force.Value();
    string isForcedArg = "false";
    if (isForced) {
        isForcedArg = "true";
    }

    NodeSwordInterfaceWorker* worker = new NodeSwordInterfaceWorker(this->_swordFacade, "refreshRemoteSources", { isForcedArg }, callback);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::repositoryConfigExisting(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    bool configExisting = (this->_swordFacade->getRepoNames().size() > 0);
    return Napi::Boolean::New(env, configExisting);
}

Napi::Value NodeSwordInterface::getRepoNames(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    vector<string> repoNames = this->_swordFacade->getRepoNames();
    Napi::Array repoNameArray = Napi::Array::New(env, repoNames.size());

    for (unsigned int i = 0; i < repoNames.size(); i++) {
        repoNameArray.Set(i, repoNames[i]); 
    }

    return repoNameArray;
}

Napi::Value NodeSwordInterface::getAllRepoModules(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();
    vector<SWModule*> modules = this->_swordFacade->getAllRepoModules(string(repoName));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

Napi::Value NodeSwordInterface::isModuleInUserDir(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();

    bool moduleInUserDir = this->_swordFacade->isModuleInUserDir(string(moduleName));
    return Napi::Boolean::New(env, moduleInUserDir);
}

Napi::Value NodeSwordInterface::getAllLocalModules(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    vector<SWModule*> modules = this->_swordFacade->getAllLocalModules();
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

Napi::Value NodeSwordInterface::getRepoModulesByLang(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 2) {
      Napi::TypeError::New(env, "Expected 2 parameters!").ThrowAsJavaScriptException();
    } else if (!info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    } else if (!info[1].IsString()) {
        Napi::TypeError::New(env, "String expected as second argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();

    vector<SWModule*> modules = this->_swordFacade->getRepoModulesByLang(string(repoName), string(languageCode));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

void NodeSwordInterface::swordModuleToNapiObject(const Napi::Env& env, SWModule* swModule, Napi::Object& object)
{
    if (swModule == 0) {
        cerr << "swModule is 0! Cannot run conversion to napi object!" << endl;
        return;
    }

    object["name"] = swModule->getName();
    object["description"] = swModule->getDescription();
    object["language"] = swModule->getLanguage();
    object["version"] = swModule->getConfigEntry("Version");
    object["about"] = swModule->getConfigEntry("About");
    object["location"] = swModule->getConfigEntry("AbsoluteDataPath");
    
    bool moduleInUserDir = this->_swordFacade->isModuleInUserDir(swModule);
    object["inUserDir"] = Napi::Boolean::New(env, false);

    if (swModule->getConfigEntry("Direction")) {
        string direction = string(swModule->getConfigEntry("Direction"));
        object["isRightToLeft"] = Napi::Boolean::New(env, (direction == "RtoL"));
    } else {
        object["isRightToLeft"] = Napi::Boolean::New(env, false);
    }

    bool moduleIsLocked = swModule->getConfigEntry("CipherKey");
    object["locked"] = Napi::Boolean::New(env, moduleIsLocked);

    if (swModule->getConfigEntry("InstallSize")) {
      int moduleSize = std::stoi(string(swModule->getConfigEntry("InstallSize")));
      object["size"] = Napi::Number::New(env, moduleSize);
    } else {
      object["size"] = Napi::Number::New(env, -1);
    }

    if (swModule->getConfigEntry("Abbreviation")) {
      object["abbreviation"] = swModule->getConfigEntry("Abbreviation");
    } else {
      object["abbreviation"] = "";
    }

    object["hasStrongs"] = Napi::Boolean::New(env, this->moduleHasGlobalOption(swModule, "Strongs"));
    object["hasFootnotes"] = Napi::Boolean::New(env, this->moduleHasGlobalOption(swModule, "Footnotes"));
    object["hasHeadings"] = Napi::Boolean::New(env, this->moduleHasGlobalOption(swModule, "Headings"));
    object["hasRedLetterWords"] = Napi::Boolean::New(env, this->moduleHasGlobalOption(swModule, "RedLetter"));
    object["hasCrossReferences"] = Napi::Boolean::New(env, this->moduleHasGlobalOption(swModule, "Scripref"));
}

bool NodeSwordInterface::moduleHasGlobalOption(SWModule* module, string globalOption)
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

// FIXME: This lacks the bibleTranslationId
void NodeSwordInterface::verseTextToNapiObject(string& rawVerse, unsigned int absoluteVerseNr, Napi::Object& object)
{
    vector<string> splittedVerse = this->split(rawVerse, '|');
    string reference = splittedVerse[0];
    string verseText = splittedVerse[1];

    vector<string> splittedReference = this->split(reference, ' ');
    string book = splittedReference[0];
    string chapterVerseReference = splittedReference[1];

    vector<string> splittedChapterVerseReference = this->split(chapterVerseReference, ':');
    string chapter = splittedChapterVerseReference[0];
    string verseNr = splittedChapterVerseReference[1];

    object["bibleBookShortTitle"] = book;
    object["chapter"] = chapter;
    object["verseNr"] = verseNr;
    object["absoluteVerseNr"] = absoluteVerseNr;
    object["content"] = verseText;
}

vector<string> NodeSwordInterface::split(const string& s, char separator)
{
    vector<string> output;
    string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(separator, pos)) != string::npos)
    {
        string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}

Napi::Value NodeSwordInterface::getRepoLanguages(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();
    vector<string> repoLanguages = this->_swordFacade->getRepoLanguages(string(repoName));
    Napi::Array languageArray = Napi::Array::New(env, repoLanguages.size());

    for (unsigned int i = 0; i < repoLanguages.size(); i++) {
        languageArray.Set(i, repoLanguages[i]); 
    }

    return languageArray;
}

Napi::Value NodeSwordInterface::getRepoTranslationCount(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();

    unsigned int translationCount = this->_swordFacade->getRepoTranslationCount(string(repoName));
    Napi::Number jsTranslationCount = Napi::Number::New(env, translationCount);

    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getRepoLanguageTranslationCount(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 2) {
      Napi::TypeError::New(env, "Expected 2 parameters!").ThrowAsJavaScriptException();
    } else if (!info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    } else if (!info[1].IsString()) {
        Napi::TypeError::New(env, "String expected as second argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();

    unsigned int translationCount = this->_swordFacade->getRepoLanguageTranslationCount(string(repoName), string(languageCode));
    Napi::Number jsTranslationCount = Napi::Number::New(env, translationCount);

    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getRepoModule(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::Object napiObject = Napi::Object::New(env);
    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_swordFacade->getRepoModule(std::string(moduleName));

    if (swordModule == 0) {
        stringstream errorMessage;
        errorMessage << "getRepoModule returned 0 for '" << std::string(moduleName) << "'" << endl;
        Napi::Error::New(env, errorMessage.str().c_str()).ThrowAsJavaScriptException();
    } else {
        this->swordModuleToNapiObject(env, swordModule, napiObject);
    }

    return napiObject;
}

Napi::Value NodeSwordInterface::getModuleDescription(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_swordFacade->getRepoModule(string(moduleName));

    if (swordModule == 0) {
        Napi::Error::New(env, "getRepoModule returned 0!").ThrowAsJavaScriptException();
    }

    string moduleDescription = string(swordModule->getDescription());
    Napi::String napiModuleDescription = Napi::String::New(env, moduleDescription);
    return napiModuleDescription;
}

Napi::Value NodeSwordInterface::getLocalModule(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::Object napiObject = Napi::Object::New(env);
    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_swordFacade->getLocalModule(string(moduleName));

    if (swordModule == 0) {
        stringstream errorMessage;
        errorMessage << "getLocalModule returned 0 for '" << string(moduleName) << "'" << endl;
        Napi::Error::New(env, errorMessage.str().c_str()).ThrowAsJavaScriptException();
    } else {
        this->swordModuleToNapiObject(env, swordModule, napiObject);
    }

    return napiObject;
}

Napi::Array NodeSwordInterface::getNapiVerseObjectsFromRawList(const Napi::Env& env, vector<string> verses)
{
    Napi::Array versesArray = Napi::Array::New(env, verses.size());

    for (unsigned int i = 0; i < verses.size(); i++) {
        string currentRawVerse = verses[i];
        // FIXME: This only works within one bible book, not for the whole bible
        unsigned int currentAbsoluteVerseNr = i + 1;

        Napi::Object verseObject = Napi::Object::New(env);
        this->verseTextToNapiObject(currentRawVerse, currentAbsoluteVerseNr, verseObject);
        versesArray.Set(i, verseObject); 
    }

    return versesArray;
}

Napi::Value NodeSwordInterface::enableMarkup(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    this->_swordFacade->enableMarkup();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::getBookText(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 2) {
      Napi::TypeError::New(env, "Expected 2 parameters!").ThrowAsJavaScriptException();
    } else if (!info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument (moduleName)").ThrowAsJavaScriptException();
    } else if (!info[1].IsString()) {
        Napi::TypeError::New(env, "String expected as second argument (bookCode)").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();

    vector<string> bookText = this->_swordFacade->getBookText(string(moduleName), string(bookCode));
    Napi::Array versesArray = this->getNapiVerseObjectsFromRawList(env, bookText);

    return versesArray;
}

Napi::Value NodeSwordInterface::getBibleText(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    vector<string> bibleText = this->_swordFacade->getBibleText(string(moduleName));
    Napi::Array versesArray = this->getNapiVerseObjectsFromRawList(env, bibleText);

    return versesArray;
}

Napi::Value NodeSwordInterface::installModule(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 2) {
      Napi::TypeError::New(env, "Expected 2 parameters!").ThrowAsJavaScriptException();
    } else if (!info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    } else if (!info[1].IsFunction()) {
        Napi::TypeError::New(env, "Function expected as second argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();

    NodeSwordInterfaceWorker* worker = new NodeSwordInterfaceWorker(this->_swordFacade, "installModule", { string(moduleName) }, callback);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::uninstallModule(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 2) {
      Napi::TypeError::New(env, "Expected 2 parameters!").ThrowAsJavaScriptException();
    } else if (!info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    } else if (!info[1].IsFunction()) {
        Napi::TypeError::New(env, "Function expected as second argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();

    NodeSwordInterfaceWorker* worker = new NodeSwordInterfaceWorker(this->_swordFacade, "uninstallModule", { moduleName }, callback);
    worker->Queue();
    return info.Env().Undefined();
}

