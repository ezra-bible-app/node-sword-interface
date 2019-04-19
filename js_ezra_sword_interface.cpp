/* This file is part of ezra-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@ezra-project.net>

   ezra-sword-interface is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ezra-sword-interface is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ezra-sword-interface. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>. */

#include <time.h>
#include <sstream>
#include <thread>
#include <iostream>

#include "swmodule.h"
#include "js_ezra_sword_interface.hpp"
#include "js_ezra_sword_interface_worker.hpp"

using namespace std;
using namespace sword;

Napi::FunctionReference JsEzraSwordInterface::constructor;

Napi::Object JsEzraSwordInterface::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "EzraSwordInterface", {
        InstanceMethod("refreshRepositoryConfig", &JsEzraSwordInterface::refreshRepositoryConfig),
        InstanceMethod("refreshRemoteSources", &JsEzraSwordInterface::refreshRemoteSources),
        InstanceMethod("repositoryConfigExisting", &JsEzraSwordInterface::repositoryConfigExisting),
        InstanceMethod("getRepoNames", &JsEzraSwordInterface::getRepoNames),
        InstanceMethod("getAllRepoModules", &JsEzraSwordInterface::getAllRepoModules),
        InstanceMethod("getRepoModulesByLang", &JsEzraSwordInterface::getRepoModulesByLang),
        InstanceMethod("getRepoLanguages", &JsEzraSwordInterface::getRepoLanguages),
        InstanceMethod("getRepoTranslationCount", &JsEzraSwordInterface::getRepoTranslationCount),
        InstanceMethod("getRepoLanguageTranslationCount", &JsEzraSwordInterface::getRepoLanguageTranslationCount),
        InstanceMethod("getModuleDescription", &JsEzraSwordInterface::getModuleDescription),
        InstanceMethod("getLocalModule", &JsEzraSwordInterface::getLocalModule),
        InstanceMethod("getBibleText", &JsEzraSwordInterface::getBibleText),
        InstanceMethod("installModule", &JsEzraSwordInterface::installModule),
        InstanceMethod("uninstallModule", &JsEzraSwordInterface::uninstallModule)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("EzraSwordInterface", func);
    return exports;
}

JsEzraSwordInterface::JsEzraSwordInterface(const Napi::CallbackInfo& info) : Napi::ObjectWrap<JsEzraSwordInterface>(info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  this->_swordFacade = new SwordFacade();
}

Napi::Value JsEzraSwordInterface::refreshRepositoryConfig(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsFunction()) {
      Napi::TypeError::New(env, "Function expected as first argument").ThrowAsJavaScriptException();
  }

  Napi::Function callback = info[0].As<Napi::Function>();
  JsEzraSwordInterfaceWorker* worker = new JsEzraSwordInterfaceWorker(this->_swordFacade, "refreshRepositoryConfig", {}, callback);
  worker->Queue();
  return info.Env().Undefined();
}

Napi::Value JsEzraSwordInterface::refreshRemoteSources(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Function expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::Function callback = info[0].As<Napi::Function>();
    JsEzraSwordInterfaceWorker* worker = new JsEzraSwordInterfaceWorker(this->_swordFacade, "refreshRemoteSources", {}, callback);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value JsEzraSwordInterface::repositoryConfigExisting(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    bool configExisting = (this->_swordFacade->getRepoNames().size() > 0);
    return Napi::Boolean::New(env, configExisting);
}

Napi::Value JsEzraSwordInterface::getRepoNames(const Napi::CallbackInfo& info)
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

Napi::Value JsEzraSwordInterface::getAllRepoModules(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();
    vector<SWModule*> modules = this->_swordFacade->getAllRepoModules(std::string(repoName));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

Napi::Value JsEzraSwordInterface::getRepoModulesByLang(const Napi::CallbackInfo& info)
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

    vector<SWModule*> modules = this->_swordFacade->getRepoModulesByLang(std::string(repoName), std::string(languageCode));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

void JsEzraSwordInterface::swordModuleToNapiObject(SWModule* swModule, Napi::Object& object)
{
    if (swModule == 0) {
        cerr << "swModule is 0! Cannot run conversion to napi object!" << endl;
        return;
    }

    object["name"] = swModule->getName();
    object["description"] = swModule->getDescription();
    object["language"] = swModule->getLanguage();
    object["version"] = swModule->getConfigEntry("Version");
    object["locked"] = swModule->getConfigEntry("CipherKey") ? "true" : "false";
    object["about"] = swModule->getConfigEntry("About");

    if (swModule->getConfigEntry("InstallSize")) {
      object["size"] = swModule->getConfigEntry("InstallSize");
    } else {
      object["size"] = "-1";
    }

    if (swModule->getConfigEntry("Abbreviation")) {
      object["abbreviation"] = swModule->getConfigEntry("Abbreviation");
    } else {
      object["abbreviation"] = "";
    }
}

Napi::Value JsEzraSwordInterface::getRepoLanguages(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();
    vector<string> repoLanguages = this->_swordFacade->getRepoLanguages(std::string(repoName));
    Napi::Array languageArray = Napi::Array::New(env, repoLanguages.size());

    for (unsigned int i = 0; i < repoLanguages.size(); i++) {
        languageArray.Set(i, repoLanguages[i]); 
    }

    return languageArray;
}

Napi::Value JsEzraSwordInterface::getRepoTranslationCount(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String repoName = info[0].As<Napi::String>();

    unsigned int translationCount = this->_swordFacade->getRepoTranslationCount(std::string(repoName));
    Napi::Number jsTranslationCount = Napi::Number::New(env, translationCount);

    return jsTranslationCount;
}

Napi::Value JsEzraSwordInterface::getRepoLanguageTranslationCount(const Napi::CallbackInfo& info)
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

    unsigned int translationCount = this->_swordFacade->getRepoLanguageTranslationCount(std::string(repoName), std::string(languageCode));
    Napi::Number jsTranslationCount = Napi::Number::New(env, translationCount);

    return jsTranslationCount;
}

Napi::Value JsEzraSwordInterface::getModuleDescription(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_swordFacade->getRepoModule(moduleName);

    if (swordModule == 0) {
        Napi::TypeError::New(env, "getRepoModule returned 0!").ThrowAsJavaScriptException();
    }

    string moduleDescription = string(swordModule->getDescription());
    Napi::String napiModuleDescription = Napi::String::New(env, moduleDescription);
    return napiModuleDescription;
}

Napi::Value JsEzraSwordInterface::getLocalModule(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    SWModule* swordModule = this->_swordFacade->getLocalModule(moduleName);

    if (swordModule == 0) {
        Napi::TypeError::New(env, "getLocalModule returned 0!").ThrowAsJavaScriptException();
    }

    Napi::Object napiObject = Napi::Object::New(env);
    this->swordModuleToNapiObject(swordModule, napiObject);

    return napiObject;
}

Napi::Value JsEzraSwordInterface::getBibleText(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (info.Length() != 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected as first argument").ThrowAsJavaScriptException();
    }

    Napi::String moduleName = info[0].As<Napi::String>();
    vector<string> bibleText = this->_swordFacade->getBibleText(std::string(moduleName));
    Napi::Array versesArray = Napi::Array::New(env, bibleText.size());

    for (unsigned int i = 0; i < bibleText.size(); i++) {
        versesArray.Set(i, bibleText[i]); 
    }

    return versesArray;
}

Napi::Value JsEzraSwordInterface::installModule(const Napi::CallbackInfo& info)
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

    JsEzraSwordInterfaceWorker* worker = new JsEzraSwordInterfaceWorker(this->_swordFacade, "installModule", { moduleName }, callback);
    worker->Queue();
    return info.Env().Undefined();
}

Napi::Value JsEzraSwordInterface::uninstallModule(const Napi::CallbackInfo& info)
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

    JsEzraSwordInterfaceWorker* worker = new JsEzraSwordInterfaceWorker(this->_swordFacade, "uninstallModule", { moduleName }, callback);
    worker->Queue();
    return info.Env().Undefined();
}

