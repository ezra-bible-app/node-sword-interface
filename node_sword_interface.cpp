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
        InstanceMethod("getRepoLanguages", &NodeSwordInterface::getRepoLanguages),
        InstanceMethod("getRepoTranslationCount", &NodeSwordInterface::getRepoTranslationCount),
        InstanceMethod("getRepoLanguageTranslationCount", &NodeSwordInterface::getRepoLanguageTranslationCount),
        InstanceMethod("getModuleDescription", &NodeSwordInterface::getModuleDescription),
        InstanceMethod("getLocalModule", &NodeSwordInterface::getLocalModule),
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
    vector<SWModule*> modules = this->_swordFacade->getAllRepoModules(std::string(repoName));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(modules[i], napiObject);
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

    vector<SWModule*> modules = this->_swordFacade->getRepoModulesByLang(std::string(repoName), std::string(languageCode));
    Napi::Array moduleArray = Napi::Array::New(env, modules.size());

    for (unsigned int i = 0; i < modules.size(); i++) {
        Napi::Object napiObject = Napi::Object::New(env);
        this->swordModuleToNapiObject(modules[i], napiObject);
        moduleArray.Set(i, napiObject); 
    }

    return moduleArray;
}

void NodeSwordInterface::swordModuleToNapiObject(SWModule* swModule, Napi::Object& object)
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

Napi::Value NodeSwordInterface::getRepoLanguages(const Napi::CallbackInfo& info)
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

Napi::Value NodeSwordInterface::getRepoTranslationCount(const Napi::CallbackInfo& info)
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

    unsigned int translationCount = this->_swordFacade->getRepoLanguageTranslationCount(std::string(repoName), std::string(languageCode));
    Napi::Number jsTranslationCount = Napi::Number::New(env, translationCount);

    return jsTranslationCount;
}

Napi::Value NodeSwordInterface::getModuleDescription(const Napi::CallbackInfo& info)
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

Napi::Value NodeSwordInterface::getLocalModule(const Napi::CallbackInfo& info)
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

Napi::Value NodeSwordInterface::getBibleText(const Napi::CallbackInfo& info)
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

    NodeSwordInterfaceWorker* worker = new NodeSwordInterfaceWorker(this->_swordFacade, "installModule", { moduleName }, callback);
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

