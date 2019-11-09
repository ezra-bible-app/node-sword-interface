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
#include "napi_sword_helper.hpp"
#include "node_sword_interface_worker.hpp"
#include "sword_facade.hpp"

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
        InstanceMethod("isModuleAvailableInRepo", &NodeSwordInterface::isModuleAvailableInRepo),
        InstanceMethod("getRepoLanguages", &NodeSwordInterface::getRepoLanguages),
        InstanceMethod("getRepoTranslationCount", &NodeSwordInterface::getRepoTranslationCount),
        InstanceMethod("getRepoLanguageTranslationCount", &NodeSwordInterface::getRepoLanguageTranslationCount),
        InstanceMethod("getRepoModule", &NodeSwordInterface::getRepoModule),
        InstanceMethod("getModuleDescription", &NodeSwordInterface::getModuleDescription),
        InstanceMethod("getLocalModule", &NodeSwordInterface::getLocalModule),
        InstanceMethod("enableMarkup", &NodeSwordInterface::enableMarkup),
        InstanceMethod("getBookText", &NodeSwordInterface::getBookText),
        InstanceMethod("getBibleText", &NodeSwordInterface::getBibleText),
        InstanceMethod("getModuleSearchResults", &NodeSwordInterface::getModuleSearchResults),
        InstanceMethod("getStrongsEntry", &NodeSwordInterface::getStrongsEntry),
        InstanceMethod("installModule", &NodeSwordInterface::installModule),
        InstanceMethod("uninstallModule", &NodeSwordInterface::uninstallModule),
        InstanceMethod("getSwordTranslation", &NodeSwordInterface::getSwordTranslation),
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

    this->_swordFacade = new SwordFacade();
    this->_napiSwordHelper = new NapiSwordHelper();
}

#define initScopeAndValidate(...) {\
    Napi::Env env = info.Env(); \
    Napi::HandleScope scope(env);\
    if (this->validateParams(info, { __VA_ARGS__ }) != 0) { return env.Null(); } \
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

Napi::Value NodeSwordInterface::refreshRepositoryConfig(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::function);
    Napi::Function callback = info[0].As<Napi::Function>();
    RefreshRepositoryConfigWorker* worker = new RefreshRepositoryConfigWorker(this->_swordFacade, callback);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::refreshRemoteSources(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::boolean, ParamType::function);
    Napi::Boolean force = info[0].As<Napi::Boolean>();
    Napi::Function callback = info[1].As<Napi::Function>();
    RefreshRemoteSourcesWorker* worker = new RefreshRemoteSourcesWorker(this->_swordFacade, callback, force.Value());
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
    initScopeAndValidate(ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    vector<SWModule*> modules = this->_swordFacade->getAllRepoModules(string(repoName));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject);
    }

    return moduleArray;
}

Napi::Value NodeSwordInterface::isModuleInUserDir(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    bool moduleInUserDir = this->_swordFacade->isModuleInUserDir(moduleName);
    return Napi::Boolean::New(info.Env(), moduleInUserDir);
}

Napi::Value NodeSwordInterface::isModuleAvailableInRepo(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    bool moduleAvailable = this->_swordFacade->isModuleAvailableInRepo(moduleName);
    return Napi::Boolean::New(info.Env(), moduleAvailable);
}

Napi::Value NodeSwordInterface::getAllLocalModules(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    vector<SWModule*> modules = this->_swordFacade->getAllLocalModules();
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

Napi::Value NodeSwordInterface::getRepoModulesByLang(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    initScopeAndValidate(ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();
    vector<SWModule*> modules = this->_swordFacade->getRepoModulesByLang(string(repoName), string(languageCode));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

Napi::Value NodeSwordInterface::getRepoLanguages(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    vector<string> repoLanguages = this->_swordFacade->getRepoLanguages(string(repoName));
    Napi::Array languageArray = Napi::Array::New(info.Env(), repoLanguages.size());

    for (unsigned int i = 0; i < repoLanguages.size(); i++) {
        languageArray.Set(i, repoLanguages[i]); 
    }

    return languageArray;
}

Napi::Value NodeSwordInterface::getRepoTranslationCount(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    unsigned int translationCount = this->_swordFacade->getRepoTranslationCount(string(repoName));
    Napi::Number jsTranslationCount = Napi::Number::New(info.Env(), translationCount);
    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getRepoLanguageTranslationCount(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();
    unsigned int translationCount = this->_swordFacade->getRepoLanguageTranslationCount(string(repoName), string(languageCode));
    Napi::Number jsTranslationCount = Napi::Number::New(info.Env(), translationCount);
    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getRepoModule(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    initScopeAndValidate(ParamType::string);
    Napi::Object napiObject = Napi::Object::New(env);
    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_swordFacade->getRepoModule(std::string(moduleName));

    if (swordModule == 0) {
        stringstream errorMessage;
        errorMessage << "getRepoModule returned 0 for '" << std::string(moduleName) << "'" << endl;
        Napi::Error::New(env, errorMessage.str().c_str()).ThrowAsJavaScriptException();
    } else {
        this->_napiSwordHelper->swordModuleToNapiObject(env, swordModule, napiObject);
    }

    return napiObject;
}

Napi::Value NodeSwordInterface::getModuleDescription(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    initScopeAndValidate(ParamType::string);
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
    initScopeAndValidate(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Object napiObject = Napi::Object::New(env);    
    SWModule* swordModule = this->_swordFacade->getLocalModule(string(moduleName));

    if (swordModule == 0) {
        stringstream errorMessage;
        errorMessage << "getLocalModule returned 0 for '" << string(moduleName) << "'" << endl;
        Napi::Error::New(env, errorMessage.str().c_str()).ThrowAsJavaScriptException();
    } else {
        this->_napiSwordHelper->swordModuleToNapiObject(env, swordModule, napiObject);
    }

    return napiObject;
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
    initScopeAndValidate(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    std::string stdModuleName = std::string(moduleName);
    vector<string> bookText = this->_swordFacade->getBookText(string(moduleName), string(bookCode));
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), stdModuleName, bookText);
    return versesArray;
}

Napi::Value NodeSwordInterface::getBibleText(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    std::string stdModuleName = std::string(moduleName);
    vector<string> bibleText = this->_swordFacade->getBibleText(string(moduleName));
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), stdModuleName, bibleText);
    return versesArray;
}

Napi::Value NodeSwordInterface::getModuleSearchResults(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    initScopeAndValidate(ParamType::string, ParamType::string, ParamType::string, ParamType::boolean, ParamType::function);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String searchTerm = info[1].As<Napi::String>();
    string searchTypeString = string(info[2].As<Napi::String>());
    Napi::Boolean isCaseSensitive = info[3].As<Napi::Boolean>();
    Napi::Function callback = info[4].As<Napi::Function>();

    SearchType searchType = SearchType::multiWord;
    
    if (searchTypeString == "phrase") {
        searchType = SearchType::phrase;
    } else if (searchTypeString == "multiWord") {
        searchType = SearchType::multiWord;
    } else if (searchTypeString == "strongsNumber") {
        searchType = SearchType::strongsNumber;
    } else {
        Napi::TypeError::New(env, "Unknown search type!").ThrowAsJavaScriptException();
    }

    GetModuleSearchResultWorker* worker = new GetModuleSearchResultWorker(this->_swordFacade,
                                                                          callback,
                                                                          moduleName,
                                                                          searchTerm,
                                                                          searchType,
                                                                          isCaseSensitive);
    worker->Queue();
    return env.Undefined();
}

Napi::Value NodeSwordInterface::getStrongsEntry(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    initScopeAndValidate(ParamType::string);
    Napi::String strongsKey = info[0].As<Napi::String>();
    Napi::Object napiObject = Napi::Object::New(env);
    StrongsEntry* strongsEntry = this->_swordFacade->getStrongsEntry(strongsKey);

    if (strongsEntry == 0) {
        stringstream errorMessage;
        errorMessage << "getStrongsEntry returned 0 for '" << string(strongsKey) << "'" << endl;
        Napi::Error::New(env, errorMessage.str().c_str()).ThrowAsJavaScriptException();
    } else {
        this->_napiSwordHelper->strongsEntryToNapiObject(env, strongsEntry, napiObject);
    }

    delete strongsEntry;
    return napiObject;
}

Napi::Value NodeSwordInterface::installModule(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string, ParamType::function);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();
    InstallModuleWorker* worker = new InstallModuleWorker(this->_swordFacade, callback, moduleName);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::uninstallModule(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string, ParamType::function);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();
    UninstallModuleWorker* worker = new UninstallModuleWorker(this->_swordFacade, callback, moduleName);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::getSwordTranslation(const Napi::CallbackInfo& info)
{
    initScopeAndValidate(ParamType::string, ParamType::string, ParamType::string);
    Napi::String configDir = info[0].As<Napi::String>();
    Napi::String originalString = info[1].As<Napi::String>();
    Napi::String localeCode = info[2].As<Napi::String>();

    Napi::String translation = Napi::String::New(info.Env(), this->_swordFacade->getSwordTranslation(
        string(configDir),
        string(originalString),
        string(localeCode)
    ));

    return translation;
}

Napi::Value NodeSwordInterface::getSwordVersion(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::String swVersion = Napi::String::New(env, this->_swordFacade->getSwordVersion());
    return swVersion;
}