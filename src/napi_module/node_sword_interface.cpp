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
#include "repository_interface.hpp"
#include "install_module_worker.hpp"
#include "module_search_worker.hpp"
#include "sword_status_reporter.hpp"
#include "module_store.hpp"
#include "module_installer.hpp"
#include "module_helper.hpp"
#include "dict_helper.hpp"
#include "text_processor.hpp"
#include "module_search.hpp"
#include "mutex.hpp"

using namespace std;
using namespace sword;

#define THROW_JS_EXCEPTION(exceptionString) { \
    Napi::Error::New(info.Env(), exceptionString).ThrowAsJavaScriptException(); \
    unlockApi(); \
    return info.Env().Null(); \
}

#define ASSERT_SW_MODULE_EXISTS(moduleName) { \
    SWModule* validatedSwordModule = this->_moduleStore->getLocalModule(moduleName); \
    if (validatedSwordModule == 0) { \
        string invalidModuleErrorMessage = "getLocalModule returned 0 for '" + string(moduleName) + "'"; \
        THROW_JS_EXCEPTION(invalidModuleErrorMessage); \
    } \
}

#define INIT_SCOPE_AND_VALIDATE(...) {\
    Napi::Env env = info.Env(); \
    Napi::HandleScope scope(env);\
    if (this->validateParams(info, { __VA_ARGS__ }) != 0) { \
        unlockApi(); \
        return env.Null(); \
    } \
}

static Mutex searchMutex;

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
        InstanceMethod("getUpdatedRepoModules", &NodeSwordInterface::getUpdatedRepoModules),
        InstanceMethod("getAllLocalModules", &NodeSwordInterface::getAllLocalModules),
        InstanceMethod("isModuleInUserDir", &NodeSwordInterface::isModuleInUserDir),
        InstanceMethod("isModuleAvailableInRepo", &NodeSwordInterface::isModuleAvailableInRepo),
        InstanceMethod("getRepoLanguages", &NodeSwordInterface::getRepoLanguages),
        InstanceMethod("getRepoModuleCount", &NodeSwordInterface::getRepoModuleCount),
        InstanceMethod("getRepoLanguageModuleCount", &NodeSwordInterface::getRepoLanguageModuleCount),
        InstanceMethod("getRepoModule", &NodeSwordInterface::getRepoModule),
        InstanceMethod("getModuleDescription", &NodeSwordInterface::getModuleDescription),
        InstanceMethod("getLocalModule", &NodeSwordInterface::getLocalModule),
        InstanceMethod("enableMarkup", &NodeSwordInterface::enableMarkup),
        InstanceMethod("disableMarkup", &NodeSwordInterface::disableMarkup),
        InstanceMethod("enableStrongsWithNbsp", &NodeSwordInterface::enableStrongsWithNbsp),
        InstanceMethod("getRawModuleEntry", &NodeSwordInterface::getRawModuleEntry),
        InstanceMethod("getReferenceText", &NodeSwordInterface::getReferenceText),
        InstanceMethod("getChapterText", &NodeSwordInterface::getChapterText),
        InstanceMethod("getBookText", &NodeSwordInterface::getBookText),
        InstanceMethod("getBibleText", &NodeSwordInterface::getBibleText),
        InstanceMethod("getVersesFromReferences", &NodeSwordInterface::getVersesFromReferences),
        InstanceMethod("getReferencesFromReferenceRange", &NodeSwordInterface::getReferencesFromReferenceRange),
        InstanceMethod("getBookList", &NodeSwordInterface::getBookList),
        InstanceMethod("getBookChapterCount", &NodeSwordInterface::getBookChapterCount),
        InstanceMethod("getChapterVerseCount", &NodeSwordInterface::getChapterVerseCount),
        InstanceMethod("getBookIntroduction", &NodeSwordInterface::getBookIntroduction),
        InstanceMethod("moduleHasBook", &NodeSwordInterface::moduleHasBook),
        InstanceMethod("getDictModuleKeys", &NodeSwordInterface::getDictModuleKeys),
        InstanceMethod("getModuleSearchResults", &NodeSwordInterface::getModuleSearchResults),
        InstanceMethod("terminateModuleSearch", &NodeSwordInterface::terminateModuleSearch),
        InstanceMethod("getStrongsEntry", &NodeSwordInterface::getStrongsEntry),
        InstanceMethod("installModule", &NodeSwordInterface::installModule),
        InstanceMethod("cancelInstallation", &NodeSwordInterface::cancelInstallation),
        InstanceMethod("uninstallModule", &NodeSwordInterface::uninstallModule),
        InstanceMethod("refreshLocalModules", &NodeSwordInterface::refreshLocalModules),
        InstanceMethod("saveModuleUnlockKey", &NodeSwordInterface::saveModuleUnlockKey),
        InstanceMethod("isModuleReadable", &NodeSwordInterface::isModuleReadable),
        InstanceMethod("getSwordTranslation", &NodeSwordInterface::getSwordTranslation),
        InstanceMethod("getBookAbbreviation", &NodeSwordInterface::getBookAbbreviation),
        InstanceMethod("getSwordVersion", &NodeSwordInterface::getSwordVersion),
        InstanceMethod("getSwordPath", &NodeSwordInterface::getSwordPath),
        InstanceMethod("unTarGZ", &NodeSwordInterface::unTarGZ),
        InstanceMethod("unZip", &NodeSwordInterface::unZip)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("NodeSwordInterface", func);
    return exports;
}

bool NodeSwordInterface::dirExists(const Napi::CallbackInfo& info, std::string dirName)
{
    bool dirExists = true;
    FileSystemHelper fsHelper;

    if (!fsHelper.fileExists(dirName)) {
        std::stringstream errorMessage;
        errorMessage << "The given directory " << dirName << " does not exist!";
        Napi::Error::New(info.Env(), errorMessage.str()).ThrowAsJavaScriptException();
        dirExists = false;
    }

    return dirExists;
}

NodeSwordInterface::NodeSwordInterface(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeSwordInterface>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    std::string localeDir = "";
    this->customHomeDir = "";
    long timeoutMillis = 20000;

    bool homeDirError = false;
    bool localeDirError = false;

    initLock();
    searchMutex.init();
    this->_currentModuleSearchWorker = 0;

    if (info[0].IsString()) {
        this->customHomeDir = string(info[0].As<Napi::String>());

        if (!this->dirExists(info, this->customHomeDir)) {
            homeDirError = true;
        }
    }

    if (info[1].IsString()) {
      localeDir = string(info[1].As<Napi::String>());

        if (!this->dirExists(info, localeDir)) {
            localeDirError = true;
        }
    } else {
        localeDirError = true;
    }

    if (info[2].IsNumber()) {
        timeoutMillis = info[2].As<Napi::Number>().Int64Value();
    } else if (!info[2].IsUndefined() && !info[2].IsNull()) {
        cerr << "Warning: Invalid timeoutMillis value (not a number), using default (20000)" << endl;
    }

    if (!homeDirError && !localeDirError) { // We only proceed if there has not been any issue with the homeDir or localeDir
        this->_moduleStore = new ModuleStore(this->customHomeDir);
        this->_moduleHelper = new ModuleHelper(*(this->_moduleStore));
        this->_dictHelper = new DictHelper(*(this->_moduleStore));
        this->_repoInterface = new RepositoryInterface(this->_swordStatusReporter, *(this->_moduleHelper), *(this->_moduleStore), this->customHomeDir, timeoutMillis);
        this->_moduleInstaller = new ModuleInstaller(*(this->_repoInterface), *(this->_moduleStore), this->customHomeDir);
        this->_napiSwordHelper = new NapiSwordHelper(*(this->_moduleHelper), *(this->_moduleStore));
        this->_textProcessor = new TextProcessor(*(this->_moduleStore), *(this->_moduleHelper));
        this->_moduleSearch = new ModuleSearch(*(this->_moduleStore), *(this->_moduleHelper), *(this->_textProcessor));
        this->_swordTranslationHelper = new SwordTranslationHelper(localeDir);
    }
}

int NodeSwordInterface::validateParams(const Napi::CallbackInfo& info, vector<ParamType> paramSpec) {
    Napi::Env env = info.Env();

    if (info.Length() != paramSpec.size()) {
        string paramCountError = "Expected " + to_string(paramSpec.size()) + " parameters, but got " + to_string(info.Length()) + "!";
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
                case ParamType::array:
                    if (!info[i].IsArray()) {
                        string arrayError = "Array expected for argument " + to_string(i + 1);
                        Napi::TypeError::New(env, arrayError).ThrowAsJavaScriptException();
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
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String moduleTypeString = info[1].As<Napi::String>();
    ModuleType moduleType = this->getModuleTypeFromString(moduleTypeString);

    vector<SWModule*> modules = this->_repoInterface->getAllRepoModules(repoName, moduleType);
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        napiObject["repository"] = repoName;
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
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    
    // Extract repository name (can be null/undefined)
    std::string repoName = "all";
    if (!info[0].IsNull() && !info[0].IsUndefined()) {
        repoName = info[0].As<Napi::String>().Utf8Value();
    }
    
    // Validate module code parameter
    Napi::String moduleName = info[1].As<Napi::String>();
    
    bool moduleAvailable = this->_repoInterface->isModuleAvailableInRepo(moduleName, repoName);
    unlockApi();
    return Napi::Boolean::New(info.Env(), moduleAvailable);
}

Napi::Value NodeSwordInterface::getAllLocalModules(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleTypeString = info[0].As<Napi::String>();
    ModuleType moduleType = this->getModuleTypeFromString(moduleTypeString);

    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    vector<SWModule*> modules = this->_moduleStore->getAllLocalModules(moduleType);
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    unlockApi();
    return moduleArray;
}

ModuleType NodeSwordInterface::getModuleTypeFromString(std::string moduleTypeString)
{
    if (moduleTypeString == "BIBLE") {
        return ModuleType::bible;
    } else if (moduleTypeString == "DICT") {
        return ModuleType::dict;
    } else if (moduleTypeString == "COMMENTARY") {
        return ModuleType::commentary;
    } else {
        return ModuleType::any;
    }
}

Napi::Value NodeSwordInterface::getRepoModulesByLang(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::string, ParamType::boolean, ParamType::boolean, ParamType::boolean, ParamType::boolean);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();
    Napi::String moduleTypeString = info[2].As<Napi::String>();
    Napi::Boolean headersFilter = info[3].As<Napi::Boolean>();
    Napi::Boolean strongsFilter = info[4].As<Napi::Boolean>();
    Napi::Boolean hebrewStrongsKeys = info[5].As<Napi::Boolean>();
    Napi::Boolean greekStrongsKeys = info[6].As<Napi::Boolean>();

    ModuleType moduleType = this->getModuleTypeFromString(moduleTypeString);

    vector<SWModule*> modules = this->_repoInterface->getRepoModulesByLang(repoName,
                                                                           languageCode,
                                                                           moduleType,
                                                                           headersFilter,
                                                                           strongsFilter,
                                                                           hebrewStrongsKeys,
                                                                           greekStrongsKeys);
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->_napiSwordHelper->swordModuleToNapiObject(env, modules[i], napiObject);
        napiObject["repository"] = repoName;
        moduleArray.Set(i, napiObject); 
    }

    unlockApi();
    return moduleArray;
}

Napi::Value NodeSwordInterface::getUpdatedRepoModules(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::boolean);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::Boolean includeBeta = info[1].As<Napi::Boolean>();

    vector<SWModule*> modules = this->_repoInterface->getUpdatedRepoModules(repoName, includeBeta);
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
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String moduleTypeString = info[1].As<Napi::String>();
    ModuleType moduleType = this->getModuleTypeFromString(moduleTypeString);

    vector<string> repoLanguages = this->_repoInterface->getRepoLanguages(repoName, moduleType);
    Napi::Array languageArray = this->_napiSwordHelper->getNapiArrayFromStringVector(info.Env(), repoLanguages);
    unlockApi();
    return languageArray;
}

Napi::Value NodeSwordInterface::getRepoModuleCount(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String moduleTypeString = info[1].As<Napi::String>();
    ModuleType moduleType = this->getModuleTypeFromString(moduleTypeString);

    unsigned int moduleCount = this->_repoInterface->getRepoModuleCount(repoName, moduleType);
    Napi::Number jsModuleCount = Napi::Number::New(info.Env(), moduleCount);
    unlockApi();
    return jsModuleCount;
}

Napi::Value NodeSwordInterface::getRepoLanguageModuleCount(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::string);
    Napi::String repoName = info[0].As<Napi::String>();
    Napi::String languageCode = info[1].As<Napi::String>();
    Napi::String moduleTypeString = info[2].As<Napi::String>();
    ModuleType moduleType = this->getModuleTypeFromString(moduleTypeString);

    unsigned int moduleCount = this->_repoInterface->getRepoLanguageModuleCount(repoName, languageCode, moduleType);
    Napi::Number jsModuleCount = Napi::Number::New(info.Env(), moduleCount);
    unlockApi();
    return jsModuleCount;
}

Napi::Value NodeSwordInterface::getRepoModule(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    
    // Extract parameters
    Napi::Object napiObject = Napi::Object::New(env);
    Napi::String moduleName = info[1].As<Napi::String>();
    std::string repoName;

    // Extract repository name (can be null/undefined)
    if (!info[0].IsNull() && !info[0].IsUndefined()) {
        repoName = info[0].As<Napi::String>().Utf8Value();
    } else {
        repoName = this->_repoInterface->getModuleRepo(moduleName);
    }
    
    SWModule* swordModule = this->_repoInterface->getRepoModule(moduleName, repoName);

    if (swordModule == 0) {
        string errorMessage = "getRepoModule returned 0 for '" + string(moduleName) + "'";
        THROW_JS_EXCEPTION(errorMessage);
    } else {
        this->_napiSwordHelper->swordModuleToNapiObject(env, swordModule, napiObject);
        napiObject["repository"] = repoName;
    }

    unlockApi();
    return napiObject;
}

Napi::Value NodeSwordInterface::getModuleDescription(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    
    // Extract repository name (can be null/undefined)
    std::string repoName = "all";
    if (!info[0].IsNull() && !info[0].IsUndefined()) {
        repoName = info[0].As<Napi::String>().Utf8Value();
    }
    
    // Validate module code parameter
    Napi::String moduleName = info[1].As<Napi::String>();

    SWModule* swordModule = this->_repoInterface->getRepoModule(moduleName, repoName);
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

    if (swordModule != 0) {
      this->_napiSwordHelper->swordModuleToNapiObject(env, swordModule, napiObject);
      unlockApi();
      return napiObject;
    } else {
      unlockApi();
      return info.Env().Undefined();
    }
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

Napi::Value NodeSwordInterface::disableMarkup(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->_textProcessor->disableMarkup();
    unlockApi();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::enableStrongsWithNbsp(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->_textProcessor->enableStrongsWithNbsp();
    unlockApi();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::getRawModuleEntry(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::Env env = info.Env();
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String key = info[1].As<Napi::String>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);
    swordModule->setKey(string(key).c_str());
    bool entryExisting = swordModule->hasEntry(swordModule->getKey());

    if (entryExisting) {
      Napi::String entryText = Napi::String::New(env, swordModule->getRawEntry());
      unlockApi();
      return entryText;
    } else {
      unlockApi();
      return env.Undefined();
    }
}

Napi::Value NodeSwordInterface::getReferenceText(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::Env env = info.Env();
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String key = info[1].As<Napi::String>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    Verse rawVerse = this->_textProcessor->getReferenceText(moduleName, key);
    Napi::Object verseObject = Napi::Object::New(env);
    this->_napiSwordHelper->verseTextToNapiObject(moduleName, rawVerse, verseObject);

    unlockApi();
    return verseObject;
}

Napi::Value NodeSwordInterface::getChapterText(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::number);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    Napi::Number chapterNumber = info[2].As<Napi::Number>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

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
    ASSERT_SW_MODULE_EXISTS(moduleName);

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
    ASSERT_SW_MODULE_EXISTS(moduleName);

    vector<Verse> bibleText = this->_textProcessor->getBibleText(moduleName);
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), string(moduleName), bibleText);
    unlockApi();
    return versesArray;
}

Napi::Value NodeSwordInterface::getVersesFromReferences(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::array);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::Array inputReferences = info[1].As<Napi::Array>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    std::vector<std::string> references;
    for (unsigned int i = 0; i < inputReferences.Length(); i++) {
        Napi::Value currentInputReference = inputReferences[i];
        references.push_back(string(currentInputReference.As<Napi::String>()));
    }

    vector<Verse> rawVerses = this->_textProcessor->getVersesFromReferences(moduleName, references);
    Napi::Array versesArray = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(info.Env(), string(moduleName), rawVerses);

    unlockApi();
    return versesArray;
}

Napi::Value NodeSwordInterface::getReferencesFromReferenceRange(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String referenceRange = info[0].As<Napi::String>();

    vector<string> references = this->_textProcessor->getReferencesFromReferenceRange(referenceRange);
    Napi::Array napiReferences = this->_napiSwordHelper->getNapiArrayFromStringVector(info.Env(), references);

    unlockApi();
    return napiReferences;
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

Napi::Value NodeSwordInterface::getBookChapterCount(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();

    Napi::Number bookChapterCount = Napi::Number::New(env, this->_moduleHelper->getBookChapterCount(moduleName, bookCode));

    unlockApi();
    return bookChapterCount;
}

Napi::Value NodeSwordInterface::getChapterVerseCount(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::number);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    Napi::Number chapter = info[2].As<Napi::Number>();

    Napi::Number chapterVerseCount = Napi::Number::New(env, this->_moduleHelper->getChapterVerseCount(moduleName, bookCode, chapter));

    unlockApi();
    return chapterVerseCount;
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

Napi::Value NodeSwordInterface::moduleHasBook(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);
    Napi::Boolean hasBook = Napi::Boolean::New(env, this->_moduleHelper->moduleHasBook(swordModule, bookCode));

    unlockApi();
    return hasBook;
}

Napi::Value NodeSwordInterface::getDictModuleKeys(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    vector<string> dictModuleKeys = this->_dictHelper->getKeyList(moduleName);
    Napi::Array dictModuleKeyArray = this->_napiSwordHelper->getNapiArrayFromStringVector(env, dictModuleKeys);

    unlockApi();
    return dictModuleKeyArray;
}

Napi::Value NodeSwordInterface::getModuleSearchResults(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, // moduleName
                            ParamType::string, // searchTerm
                            ParamType::string, // searchType
                            ParamType::string, // searchScope
                            ParamType::boolean, // isCaseSensitive
                            ParamType::boolean, // useExtendedVerseBoundaries
                            ParamType::boolean, // filterOnWordBoundaries
                            ParamType::function, // progressCallback
                            ParamType::function); // final Callback

    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String searchTerm = info[1].As<Napi::String>();
    string searchTypeString = string(info[2].As<Napi::String>());
    string searchScopeString = string(info[3].As<Napi::String>());
    Napi::Boolean isCaseSensitive = info[4].As<Napi::Boolean>();
    Napi::Boolean useExtendedVerseBoundaries = info[5].As<Napi::Boolean>();
    Napi::Boolean filterOnWordBoundaries = info[6].As<Napi::Boolean>();
    Napi::Function jsProgressCallback = info[7].As<Napi::Function>();
    Napi::Function callback = info[8].As<Napi::Function>();
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

    SearchScope searchScope = SearchScope::BIBLE;

    if (searchScopeString == "BIBLE") {
        searchScope = SearchScope::BIBLE;
    } else if (searchScopeString == "OT") {
        searchScope = SearchScope::OT;
    } else if (searchScopeString == "NT") {
        searchScope = SearchScope::NT;
    } else {
        THROW_JS_EXCEPTION("Unknown search scope!");
    }

    this->_currentModuleSearchWorker = new ModuleSearchWorker(*(this->_moduleHelper),
                                                              *(this->_moduleSearch),
                                                              *(this->_moduleStore),
                                                              *(this->_repoInterface),
                                                              searchMutex,
                                                              jsProgressCallback,
                                                              callback,
                                                              moduleName,
                                                              searchTerm,
                                                              searchType,
                                                              searchScope,
                                                              isCaseSensitive,
                                                              useExtendedVerseBoundaries,
                                                              filterOnWordBoundaries); // Pass the new parameter
    this->_currentModuleSearchWorker->Queue();
    return env.Undefined();
}

Napi::Value NodeSwordInterface::terminateModuleSearch(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (this->_currentModuleSearchWorker != 0) {
        this->_currentModuleSearchWorker->terminateSearch();
    }

    return info.Env().Undefined();
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
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    
    // Extract repository name (can be null/undefined)
    std::string repoName = "";
    if (!info[0].IsNull() && !info[0].IsUndefined()) {
        repoName = info[0].As<Napi::String>().Utf8Value();
    }
    
    // Validate remaining parameters
    Napi::String moduleName = info[1].As<Napi::String>();
    Napi::Function progressCallback = info[2].As<Napi::Function>();
    Napi::Function callback = info[3].As<Napi::Function>();

    InstallModuleWorker* worker = new InstallModuleWorker(*(this->_repoInterface),
                                                          *(this->_moduleInstaller),
                                                          progressCallback,
                                                          callback,
                                                          moduleName,
                                                          repoName);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::cancelInstallation(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->_moduleInstaller->cancelInstallation();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::uninstallModule(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    
    // Extract repository name (can be null/undefined)
    std::string repoName = "";
    if (!info[0].IsNull() && !info[0].IsUndefined()) {
        repoName = info[0].As<Napi::String>().Utf8Value();
    }
    
    // Validate remaining parameters
    Napi::String moduleName = info[1].As<Napi::String>();
    Napi::Function callback = info[2].As<Napi::Function>();
    
    UninstallModuleWorker* worker = new UninstallModuleWorker(*(this->_repoInterface), *(this->_moduleInstaller), callback, moduleName, repoName);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::refreshLocalModules(const Napi::CallbackInfo& info)
{
    lockApi();
    this->_moduleInstaller->resetAllMgrs();
    unlockApi();
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
    return info.Env().Undefined();
}

Napi::Value NodeSwordInterface::isModuleReadable(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);
    bool moduleReadable = this->_textProcessor->isModuleReadable(swordModule);
    unlockApi();
    return Napi::Boolean::New(info.Env(), moduleReadable);
}

Napi::Value NodeSwordInterface::getSwordTranslation(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    Napi::String originalString = info[0].As<Napi::String>();
    Napi::String localeCode = info[1].As<Napi::String>();

    Napi::String translation = Napi::String::New(info.Env(), this->_swordTranslationHelper->getSwordTranslation(
        originalString,
        localeCode
    ));

    unlockApi();
    return translation;
}

Napi::Value NodeSwordInterface::getBookAbbreviation(const Napi::CallbackInfo& info)
{
    lockApi();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string, ParamType::string);
    Napi::String moduleName = info[0].As<Napi::String>();
    Napi::String bookCode = info[1].As<Napi::String>();
    Napi::String localeCode = info[2].As<Napi::String>();
    ASSERT_SW_MODULE_EXISTS(moduleName);

    SWModule* swordModule = this->_moduleStore->getLocalModule(moduleName);

    Napi::String abbreviation = Napi::String::New(info.Env(), this->_swordTranslationHelper->getBookAbbreviation(
        swordModule,
        bookCode,
        localeCode
    ));

    unlockApi();
    return abbreviation;
}

Napi::Value NodeSwordInterface::getSwordVersion(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

#if defined(__ANDROID__)
    string version = "1.9.0-2025-12-30";
#elif defined(__linux__) || defined(__APPLE__)
    string version = "1.9.0-2025-12-30";
#elif _WIN32
    string version = "1.9.0-2025-03-03";
#endif

    Napi::String swVersion = Napi::String::New(env, version);
    unlockApi();
    return swVersion;
}

Napi::Value NodeSwordInterface::getSwordPath(const Napi::CallbackInfo& info)
{
    lockApi();
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    FileSystemHelper fsHelper;
    if (this->customHomeDir != "") {
      fsHelper.setCustomHomeDir(customHomeDir);
    }

    Napi::String swordPath = Napi::String::New(env, fsHelper.getUserSwordDir());
    unlockApi();
    return swordPath;
}

Napi::Value NodeSwordInterface::unTarGZ(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    lockApi();

    string filePath = info[0].As<Napi::String>().Utf8Value();
    string destPath = info[1].As<Napi::String>().Utf8Value();

    FileSystemHelper fsHelper;
    bool ret = fsHelper.unTarGZ(filePath, destPath);

    unlockApi();
    return Napi::Boolean::New(env, ret);
}

Napi::Value NodeSwordInterface::unZip(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    INIT_SCOPE_AND_VALIDATE(ParamType::string, ParamType::string);
    lockApi();

    string filePath = info[0].As<Napi::String>().Utf8Value();
    string destPath = info[1].As<Napi::String>().Utf8Value();

    FileSystemHelper fsHelper;
    bool ret = fsHelper.unZip(filePath, destPath);

    unlockApi();
    return Napi::Boolean::New(env, ret);
}
