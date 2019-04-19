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

#ifndef _NODE_SWORD_INTERFACE
#define _NODE_SWORD_INTERFACE

#include <napi.h>
#include "sword_facade.hpp"

class NodeSwordInterface : public Napi::ObjectWrap<NodeSwordInterface> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NodeSwordInterface(const Napi::CallbackInfo& info);

private:
    static Napi::FunctionReference constructor;

    Napi::Value refreshRepositoryConfig(const Napi::CallbackInfo& info);

    Napi::Value refreshRemoteSources(const Napi::CallbackInfo& info);
    Napi::Value repositoryConfigExisting(const Napi::CallbackInfo& info);

    Napi::Value getRepoNames(const Napi::CallbackInfo& info);
    Napi::Value getAllRepoModules(const Napi::CallbackInfo& info);
    Napi::Value getRepoModulesByLang(const Napi::CallbackInfo& info);
    Napi::Value getRepoLanguages(const Napi::CallbackInfo& info);
    Napi::Value getRepoTranslationCount(const Napi::CallbackInfo& info);
    Napi::Value getRepoLanguageTranslationCount(const Napi::CallbackInfo& info);

    Napi::Value getModuleDescription(const Napi::CallbackInfo& info);
    Napi::Value getLocalModule(const Napi::CallbackInfo& info);

    Napi::Value getBibleText(const Napi::CallbackInfo& info);

    Napi::Value installModule(const Napi::CallbackInfo& info);
    Napi::Value uninstallModule(const Napi::CallbackInfo& info);

    SwordFacade* _swordFacade;

    // Functions not exported to js
    void swordModuleToNapiObject(sword::SWModule* swModule, Napi::Object& object);
};

#endif // _NODE_SWORD_INTERFACE

