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

#include <time.h>
#include <sstream>
#include <thread>
#include <iostream>
#include <mutex>

#include "api_lock.hpp"
#include "swmodule.h"
#include "node_sword_interface.hpp"
#include "napi_sword_helper.hpp"
#include "worker.hpp"
#include "install_module_worker.hpp"
#include "module_search_worker.hpp"
#include "sword_status_reporter.hpp"
#include "module_store.hpp"
#include "module_installer.hpp"
#include "module_helper.hpp"
#include "text_processor.hpp"
#include "module_search.hpp"

using namespace std;
using namespace sword;

#if defined(_WIN32)
// For some reason this symbol is missing in the sword.dll on Windows, hence we include it here.
char * sword::SWBuf::nullStr = (char *)"";
#endif

Napi::FunctionReference NodeSwordInterface::constructor;

Napi::Object NodeSwordInterface::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NodeSwordInterface", {
        InstanceMethod("updateRepositoryConfig", &NodeSwordInterface::updateRepositoryConfig),
        InstanceMethod("repositoryConfigExisting", &NodeSwordInterface::repositoryConfigExisting),
        InstanceMethod("getRepoNames", &NodeSwordInterface::getRepoNames),
        InstanceMethod("getAllRepoModules", &NodeSwordInterface::getAllRepoModules),
        InstanceMethod("getRepoModulesByLang", &NodeSwordInterface::getRepoModulesByLang),
        InstanceMethod("getAllLocalModules", &NodeSwordInterface::getAllLocalModules),
        InstanceMethod("isModuleInUserDir", &NodeSwordInterface::isModuleInUserDir),
        InstanceMethod("isModuleAvailableInRepo", &NodeSwordInterface::isModuleAvailableInRepo),
        InstanceMethod("getRepoLanguages", &NodeSwordInterface::getRepoLanguages),
        InstanceMethod("getRepoTranslationCount", &NodeSwordInterface::getRepoTranslationCount),
        InstanceMethod("getRepoLanguageTranslationCount", &NodeSwordInterface::getRepoLanguageTranslationCount),
        InstanceMethod("getRepoModule", &NodeSwordInterface::getRepoModule),
        InstanceMethod("getModuleDescription", &NodeSwordInterface::getModuleDescription),
        InstanceMethod("getLocalModule", &NodeSwordInterface::getLocalModule),
        InstanceMethod("enableMarkup", &NodeSwordInterface::enableMarkup),
        InstanceMethod("getChapterText", &NodeSwordInterface::getChapterText),
        InstanceMethod("getBookText", &NodeSwordInterface::getBookText),
        InstanceMethod("getBibleText", &NodeSwordInterface::getBibleText),
        InstanceMethod("getBookList", &NodeSwordInterface::getBookList),
        InstanceMethod("getBibleChapterVerseCounts", &NodeSwordInterface::getBibleChapterVerseCounts),
        InstanceMethod("getBookIntroduction", &NodeSwordInterface::getBookIntroduction),
        InstanceMethod("getModuleSearchResults", &NodeSwordInterface::getModuleSearchResults),
        InstanceMethod("getStrongsEntry", &NodeSwordInterface::getStrongsEntry),
        InstanceMethod("installModule", &NodeSwordInterface::installModule),
        InstanceMethod("uninstallModule", &NodeSwordInterface::uninstallModule),
        InstanceMethod("saveModuleUnlockKey", &NodeSwordInterface::saveModuleUnlockKey),
        InstanceMethod("isModuleReadable", &NodeSwordInterface::isModuleReadable),
        InstanceMethod("getSwordTranslation", &NodeSwordInterface::getSwordTranslation),
        InstanceMethod("getBookAbbreviation", &NodeSwordInterface::getBookAbbreviation),
        InstanceMethod("getSwordVersion", &NodeSwordInterface::getSwordVersion)
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

    this->_moduleStore = new ModuleStore();
    this->_moduleHelper = new ModuleHelper(*(this->_moduleStore));
    this->_repoInterface = new RepositoryInterface(this->_swordStatusReporter, *(this->_moduleHelper));
    this->_moduleInstaller = new ModuleInstaller(*(this->_repoInterface), *(this->_moduleStore));
    this->_napiSwordHelper = new NapiSwordHelper(*(this->_moduleHelper), *(this->_moduleStore));
    this->_textProcessor = new TextProcessor(*(this->_moduleStore), *(this->_moduleHelper));
    this->_moduleSearch = new ModuleSearch(*(this->_moduleStore), *(this->_moduleHelper), *(this->_textProcessor));
}

#define THROW_JS_EXCEPTION(exceptionString) { \
    Napi::Error::New(info.Env(), exceptionString).ThrowAsJavaScriptException(); \
    unlockApi(); \
    return info.Env().Null(); \
}

#define INIT_SCOPE_AND_VALIDATE(...) {\
    Napi::Env env = info.Env(); \
    Napi::HandleScope scope(env);\
    if (this->validateParams(info, { __VA_ARGS__ }) != 0) { \
        unlockApi(); \
        return env.Null(); \
    } \
}

int NodeSwordInterface::validateParams(const Napi::CallbackInfo& info, vector<ParamType> paramSpec) {
    Napi::Env env = info.Env();

    if (info.Length() != paramSpec.size()) {
        string paramCountError = "Expected " + to_string(paramSpec.size()) + " parameters, but got " += to_string(info.Length()) + "!";
        Napi::TypeError::New(env, paramCountError).ThrowAsJavaScriptException();
        return -1;
    } else {
        for (unsigned int i = 0; i < paramSpec.size(); i++) {
            switch (paramSpec[i]) {
                case ParamType::string:
                    if (!info[i].IsString()) {
                        string stringError = "String expected for argument " + to_string(i + 1);
                        Napi::TypeError::New(env, stringError).ThrowAsJavaScriptException();
                        return -1;
                    }
                    break;
                case ParamType::number:
                    if (!info[i].IsNumber()) {
                        string numberError = "Number expected for argument " + to_string(i + 1);
                        Napi::TypeError::New(env, numberError).ThrowAsJavaScriptException();
                        return -1;
                    }
                    break;
                case ParamType::boolean:
                    if (!info[i].IsBoolean()) {
                        string booleanError = "Boolean expected for argument " + to_string(i + 1);
                        Napi::TypeError::New(env, booleanError).ThrowAsJavaScriptException();
                        return -1;
                    }
                    break;
                case ParamType::function:
                    if (!info[i].IsFunction()) {
                        string functionError = "Function expected for argument " + to_string(i + 1);
                        Napi::TypeError::New(env, functionError).ThrowAsJavaScriptException();
                        return -1;
                    }
                    break;
                default:
                    cerr << "Unknown ParamType / invalid paramSpec!" << endl;
                    return -1;
            }
        }
    }

    return 0;
}

Napi::Value NodeSwordInterface::updateRepositoryConfig(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::boolean, ParamType::function, ParamType::function);
    Napi::Boolean force = info[0].As<Napi::Boolean>();
    Napi::Function progressCallback = info[1].As<Napi::Function>();
    Napi::Function callback = info[2].As<Napi::Function>();
    RefreshRemoteSourcesWorker* worker = new RefreshRemoteSourcesWorker(*(this->_repoInterface),
                                                                        progressCallback,
                                                                        callback,
                                                                        force.Value());
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::repositoryConfigExisting(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    bool configExisting = (this->_repoInterface->getRepoNames().size() > 0);
    unlockApi();
    return Napi::Boolean::New(env, configExisting);
}

Napi::Value NodeSwordInterface::getRepoNames(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    vector<string> repoNames = this->_repoInterface->getRepoNames();
    Napi::Array repoNameArray = this->_napiSwordHelper->getNapiArrayFromStringVector(env, repoNames);
    unlockApi();
    return repoNameArray;
}

Napi::Value NodeSwordInterface::getAllRepoModules(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    vector<SWModule*> modules = this->_repoInterface->getAllRepoModules(repoName);
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject);
    }

    unlockApi();
    return moduleArray;
}

Napi::Value NodeSwordInterface::isModuleInUserDir(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    bool moduleInUserDir = this->_moduleStore->isModuleInUserDir(moduleName);
    unlockApi();
    return Napi::Boolean::New(info.Env(), moduleInUserDir);
}

Napi::Value NodeSwordInterface::isModuleAvailableInRepo(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    bool moduleAvailable = this->_repoInterface->isModuleAvailableInRepo(moduleName);
    unlockApi();
    return Napi::Boolean::New(info.Env(), moduleAvailable);
}

Napi::Value NodeSwordInterface::getAllLocalModules(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    vector<SWModule*> modules = this->_moduleStore->getAllLocalModules();
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    unlockApi();
    return moduleArray;
}

Napi::Value NodeSwordInterface::getRepoModulesByLang(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::boolean, ParamType::boolean);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();
    Napi::Boolean headersFilter = info[2].As<Napi::Boolean>();
    Napi::Boolean strongsFilter = info[3].As<Napi::Boolean>();
    vector<SWModule*> modules = this->_repoInterface->getRepoModulesByLang(repoName, languageCode, headersFilter, strongsFilter);
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    unlockApi();
    return moduleArray;
}

Napi::Value NodeSwordInterface::getRepoLanguages(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    vector<string> repoLanguages = this->_repoInterface->getRepoLanguages(repoName);
    Napi::Array languageArray = this->_napiSwordHelper->getNapiArrayFromStringVector(info.Env(), repoLanguages);
    unlockApi();
    return languageArray;
}

Napi::Value NodeSwordInterface::getRepoTranslationCount(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    unsigned int translationCount = this->_repoInterface->getRepoTranslationCount(repoName);
    Napi::Number jsTranslationCount = Napi::Number::New(info.Env(), translationCount);
    unlockApi();
    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getRepoLanguageTranslationCount(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();
    unsigned int translationCount = this->_repoInterface->getRepoLanguageTranslationCount(repoName, languageCode);
    Napi::Number jsTranslationCount = Napi::Number::New(info.Env(), translationCount);
    unlockApi();
    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getRepoModule(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::Object napiObject = Napi::Object::New(env);
    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_repoInterface->getRepoModule(moduleName);

    if (swordModule == 0) {
        string errorMessage = "getRepoModule returned 0 for '" + string(moduleName) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    } else {
        this->_napiSwordHelper->swordModuleToNapiObject(env, swordModule, napiObject);
    }

    unlockApi();
    return napiObject;
}

Napi::Value NodeSwordInterface::getModuleDescription(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();

    SWModule* swordModule = this->_repoInterface->getRepoModule(moduleName);
    if (swordModule == 0) {
        string errorMessage = "getRepoModule returned 0 for '" + string(moduleName) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    }

    string moduleDescription = string(swordModule->getDescription());
    Napi::String napiModuleDescription = Napi::String::New(env, moduleDescription);
    unlockApi();
    return napiModuleDescription;
}

Napi::Value NodeSwordInterface::getLocalModule(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Object napiObject = Napi::Object::New(env);    
    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);

    if (swordModule == 0) {
        string errorMessage = "getLocalModule returned 0 for '" + string(moduleName) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    } else {
        this->_napiSwordHelper->swordModuleToNapiObject(env, swordModule, napiObject);
    }

    unlockApi();
    return napiObject;
}

Napi::Value NodeSwordInterface::enableMarkup(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->_textProcessor->enableMarkup();
    unlockApi();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::getChapterText(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::number);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    Napi::Number chapterNumber = info[2].As<Napi::Number>();

    vector<Verse> chapterText = this->_textProcessor->getChapterText(moduleName, bookCode, chapterNumber.Int32Value());
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), string(moduleName), chapterText);
    unlockApi();
    return versesArray;
}

Napi::Value NodeSwordInterface::getBookText(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::number, ParamType::number);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    Napi::Number startVerseNr = info[2].As<Napi::Number>();
    Napi::Number verseCount = info[3].As<Napi::Number>();

    vector<Verse> bookText = this->_textProcessor->getBookText(moduleName, bookCode, startVerseNr.Int32Value(), verseCount.Int32Value());
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), string(moduleName), bookText);
    unlockApi();
    return versesArray;
}

Napi::Value NodeSwordInterface::getBibleText(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();

    vector<Verse> bibleText = this->_textProcessor->getBibleText(moduleName);
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), string(moduleName), bibleText);
    unlockApi();
    return versesArray;
}

Napi::Value NodeSwordInterface::getBookList(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();

    vector<string> bookList = this->_moduleHelper->getBookList(moduleName);
    Napi::Array bookArray = this->_napiSwordHelper->getNapiArrayFromStringVector(info.Env(), bookList);
    unlockApi();
    return bookArray;
}

Napi::Value NodeSwordInterface::getBibleChapterVerseCounts(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();

    map<string, vector<int>> chapterVerseCounts = this->_moduleHelper->getBibleChapterVerseCounts(moduleName);
    Napi::Object jsChapterVerseCounts = Napi::Object::New(env);
    
    for (const auto &book : chapterVerseCounts) {
        vector<int> verseCounts = book.second;
        Napi::Array jsVerseCounts = Napi::Array::New(env, verseCounts.size());

        for (unsigned int i = 0; i < verseCounts.size(); i++) {
            jsVerseCounts.Set(i, verseCounts[i]);
        }

        jsChapterVerseCounts[book.first] = jsVerseCounts;
    }

    unlockApi();
    return jsChapterVerseCounts;
}

Napi::Value NodeSwordInterface::getBookIntroduction(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    Napi::String introText = Napi::String::New(env, this->_textProcessor->getBookIntroduction(moduleName, bookCode));
    unlockApi();
    return introText;
}

Napi::Value NodeSwordInterface::getModuleSearchResults(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, // moduleName
                            ParamType::string, // searchTerm
                            ParamType::string, // searchType
                            ParamType::boolean, // isCaseSensitive
                            ParamType::boolean, // useExtendedVerseBoundaries
                            ParamType::function, // progressCallback
                            ParamType::function); // final Callback

    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String searchTerm = info[1].As<Napi::String>();
    string searchTypeString = string(info[2].As<Napi::String>());
    Napi::Boolean isCaseSensitive = info[3].As<Napi::Boolean>();
    Napi::Boolean useExtendedVerseBoundaries = info[4].As<Napi::Boolean>();
    Napi::Function jsProgressCallback = info[5].As<Napi::Function>();
    Napi::Function callback = info[6].As<Napi::Function>();
    SearchType searchType = SearchType::multiWord;
    
    if (searchTypeString == "phrase") {
        searchType = SearchType::phrase;
    } else if (searchTypeString == "multiWord") {
        searchType = SearchType::multiWord;
    } else if (searchTypeString == "strongsNumber") {
        searchType = SearchType::strongsNumber;
    } else {
        THROW_JS_EXCEPTION("Unknown search type!");
    }

    if (searchType == SearchType::strongsNumber) {
        if (!StrongsEntry::isValidStrongsKey(searchTerm)) {
            THROW_JS_EXCEPTION("The given search term is not a valid Strong's number!");
        }
    }

    ModuleSearchWorker* worker = new ModuleSearchWorker(*(this->_moduleHelper),
                                                        *(this->_moduleSearch),
                                                        *(this->_moduleStore),
                                                        *(this->_repoInterface),
                                                        jsProgressCallback,
                                                        callback,
                                                        moduleName,
                                                        searchTerm,
                                                        searchType,
                                                        isCaseSensitive,
                                                        useExtendedVerseBoundaries);
    worker->Queue();
    return env.Undefined();
}

Napi::Value NodeSwordInterface::getStrongsEntry(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String strongsKey = info[0].As<Napi::String>();
    Napi::Object napiObject = Napi::Object::New(env);
    StrongsEntry* strongsEntry = this->_textProcessor->getStrongsEntry(strongsKey);

    if (strongsEntry == 0) {
        string errorMessage = "getStrongsEntry returned 0 for '" + string(strongsKey) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    } else {
        this->_napiSwordHelper->strongsEntryToNapiObject(env, strongsEntry, napiObject);
    }

    delete strongsEntry;

    unlockApi();
    return napiObject;
}

Napi::Value NodeSwordInterface::installModule(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::function, ParamType::function);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Function progressCallback = info[1].As<Napi::Function>();
    Napi::Function callback = info[2].As<Napi::Function>();

    InstallModuleWorker* worker = new InstallModuleWorker(*(this->_repoInterface),
                                                          *(this->_moduleInstaller),
                                                          progressCallback,
                                                          callback,
                                                          moduleName);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::uninstallModule(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::function);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();
    UninstallModuleWorker* worker = new UninstallModuleWorker(*(this->_repoInterface), *(this->_moduleInstaller), callback, moduleName);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::saveModuleUnlockKey(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);

    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String key = info[1].As<Napi::String>();

    int returnCode = this->_moduleInstaller->saveModuleUnlockKey(moduleName, key);

    if (returnCode == 0) {
        unlockApi();
        return info.Env().Undefined();
    } else {
        string errorMessage;

        switch(returnCode) {
            case -1:
                errorMessage = "Invalid parameter";
                break;

            case -2:
                errorMessage = "Section cipherKey not found in config file!";
                break;

            case -3:
                errorMessage = "Module file not found!";
                break;
            
            default:
                errorMessage = "Unknown error!";
                break;
        }

        THROW_JS_EXCEPTION(errorMessage);
    }

    unlockApi();
}

Napi::Value NodeSwordInterface::isModuleReadable(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);

    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);

    if (swordModule == 0) {
        string errorMessage = "getLocalModule returned 0 for '" + string(moduleName) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    } else {
        bool moduleReadable = this->_textProcessor->isModuleReadable(swordModule);
        unlockApi();
        return Napi::Boolean::New(info.Env(), moduleReadable);
    }
}

Napi::Value NodeSwordInterface::getSwordTranslation(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::string);
    Napi::String configDir = info[0].As<Napi::String>();
    Napi::String originalString = info[1].As<Napi::String>();
    Napi::String localeCode = info[2].As<Napi::String>();

    Napi::String translation = Napi::String::New(info.Env(), this->_swordTranslationHelper.getSwordTranslation(
        configDir,
        originalString,
        localeCode
    ));

    unlockApi();
    return translation;
}

Napi::Value NodeSwordInterface::getBookAbbreviation(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::string, ParamType::string);
    Napi::String configDir = info[0].As<Napi::String>();
    Napi::String moduleName = info[1].As<Napi::String>();
    Napi::String bookCode = info[2].As<Napi::String>();
    Napi::String localeCode = info[3].As<Napi::String>();

    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);

    if (swordModule == 0) {
        string errorMessage = "getLocalModule returned 0 for '" + string(moduleName) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    } else {
        Napi::String abbreviation = Napi::String::New(info.Env(), this->_swordTranslationHelper.getBookAbbreviation(
            configDir,
            swordModule,
            bookCode,
            localeCode
        ));

        unlockApi();
        return abbreviation;
    }
}

Napi::Value NodeSwordInterface::getSwordVersion(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    string version = "1.8.900-8edcfc";
    Napi::String swVersion = Napi::String::New(env, version);
    unlockApi();
    return swVersion;
}
