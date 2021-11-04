/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2021 Tobias Klein <contact@tklein.info>

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

#ifndef _WORKER
#define _WORKER

#include <napi.h>
#include <iostream>
#include <map>

#include "api_lock.hpp"
#include "napi_sword_helper.hpp"
#include "repository_interface.hpp"
#include "sword_status_reporter.hpp"
#include "common_defs.hpp"
#include "module_installer.hpp"

using namespace std;

class SwordProgressFeedback {
public:
    int totalPercent;
    int filePercent;
    std::string message;
};

class BaseWorker : public Napi::AsyncProgressWorker<SwordProgressFeedback> {
public:
    BaseWorker(RepositoryInterface& repoInterface, const Napi::Function& callback)
        : Napi::AsyncProgressWorker<SwordProgressFeedback>(callback), _repoInterface(repoInterface) {}

    virtual ~BaseWorker() {}

    virtual void OnOK() {
        Napi::HandleScope scope(this->Env());
        Callback().Call({Env().Null()});
    }

    virtual void OnProgress(const SwordProgressFeedback* progressFeedback, size_t /* count */) {
        Napi::HandleScope scope(this->Env());
    }

protected:
    RepositoryInterface& _repoInterface;
};

class ProgressWorker : public BaseWorker {
public:
    ProgressWorker(RepositoryInterface& repoInterface,
                   const Napi::Function& jsProgressCallback,
                   const Napi::Function& callback)
        : BaseWorker(repoInterface, callback),
        _jsProgressCallback(Napi::Persistent(jsProgressCallback)) {}

    void OnProgress(const SwordProgressFeedback* progressFeedback, size_t /* count */) {
        Napi::HandleScope scope(this->Env());

        if (progressFeedback != 0) {
            Napi::Object jsProgressFeedback = Napi::Object::New(this->Env());
            jsProgressFeedback["totalPercent"] = progressFeedback->totalPercent;
            jsProgressFeedback["filePercent"] = progressFeedback->filePercent;
            jsProgressFeedback["message"] = progressFeedback->message;

            this->_jsProgressCallback.Call({ jsProgressFeedback });
        }
    }

protected:
    virtual void sendExecutionProgress(int totalPercent, int filePercent, std::string message) {
        if (this->_executionProgress != 0) {
            SwordProgressFeedback feedback;

            feedback.totalPercent = totalPercent;
            feedback.filePercent = filePercent;
            feedback.message = message;
            this->_executionProgress->Send(&feedback, 1);
        }        
    }

    Napi::FunctionReference _jsProgressCallback;
    const ExecutionProgress* _executionProgress = 0;
};

class RefreshRemoteSourcesWorker : public ProgressWorker {
public:
    RefreshRemoteSourcesWorker(RepositoryInterface& repoInterface,
                               const Napi::Function& jsProgressCallback,
                               const Napi::Function& callback,
                               bool forced)

        : ProgressWorker(repoInterface, jsProgressCallback, callback),
          _forced(forced) {}
    
    void progressCallback(unsigned int progressPercentage) {
        this->sendExecutionProgress(progressPercentage, 0, "");
    }

    void Execute(const ExecutionProgress& progress) {
        this->_executionProgress = &progress;
        std::function<void(unsigned int)> _progressCallback = std::bind(&RefreshRemoteSourcesWorker::progressCallback,
                                                                        this,
                                                                        std::placeholders::_1);

        int ret = this->_repoInterface.refreshRemoteSources(this->_forced, &this->_repoUpdateStatus, &_progressCallback);
        this->_isSuccessful = (ret == 0);
        unlockApi();
    }

    void OnOK() {
        Napi::HandleScope scope(this->Env());

        Napi::Object repoUpdateStatusObject = Napi::Object::New(this->Env());
        repoUpdateStatusObject["result"] = this->_isSuccessful;

        map<string, bool>::iterator it;
        for (it = this->_repoUpdateStatus.begin(); it != this->_repoUpdateStatus.end(); it++) {
          repoUpdateStatusObject[it->first] = it->second;
        }
        
        Callback().Call({ repoUpdateStatusObject });
    }

private:
    bool _isSuccessful;
    bool _forced;
    map<string, bool> _repoUpdateStatus;
};

class UninstallModuleWorker : public BaseWorker {
public:
    UninstallModuleWorker(RepositoryInterface& repoInterface, ModuleInstaller& moduleInstaller, const Napi::Function& callback, std::string moduleName)
        : BaseWorker(repoInterface, callback), _moduleInstaller(moduleInstaller), _moduleName(moduleName) {}

    void Execute(const ExecutionProgress& progress) {
        int ret = this->_moduleInstaller.uninstallModule(this->_moduleName);
        this->_isSuccessful = (ret == 0);
        unlockApi();
    }

    void OnOK() {
        Napi::HandleScope scope(this->Env());
        Napi::Boolean isSuccessful = Napi::Boolean::New(this->Env(), this->_isSuccessful);
        Callback().Call({ isSuccessful });
    }

private:
    ModuleInstaller& _moduleInstaller;
    bool _isSuccessful;
    std::string _moduleName;
};

#endif // _WORKER

