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

#ifndef _NODE_SWORD_INTERFACE_WORKER
#define _NODE_SWORD_INTERFACE_WORKER

#include <napi.h>
#include <iostream>

#include "napi_sword_helper.hpp"
#include "sword_facade.hpp"
#include "sword_status_reporter.hpp"

using namespace std;

class SwordProgressFeedback {
public:
    int totalPercent;
    int filePercent;
    std::string message;
};

class BaseNodeSwordInterfaceWorker : public Napi::AsyncProgressWorker<SwordProgressFeedback> {
public:
    BaseNodeSwordInterfaceWorker(SwordFacade& facade, const Napi::Function& callback)
        : Napi::AsyncProgressWorker<SwordProgressFeedback>(callback), _facade(facade) {}

    virtual ~BaseNodeSwordInterfaceWorker() {}

    virtual void OnOK() {
        Napi::HandleScope scope(this->Env());
        Callback().Call({Env().Null()});
    }

    virtual void OnProgress(const SwordProgressFeedback* progressFeedback, size_t /* count */) {
        Napi::HandleScope scope(this->Env());
    }

protected:
    SwordFacade& _facade;
};

class ProgressNodeSwordInterfaceWorker : public BaseNodeSwordInterfaceWorker {
public:
    ProgressNodeSwordInterfaceWorker(SwordFacade& facade,
                                     const Napi::Function& jsProgressCallback,
                                     const Napi::Function& callback)
        : BaseNodeSwordInterfaceWorker(facade, callback),
        _jsProgressCallback(Napi::Persistent(jsProgressCallback)) {}

    void OnProgress(const SwordProgressFeedback* progressFeedback, size_t /* count */) {
        Napi::HandleScope scope(this->Env());

        if (progressFeedback != 0) {
            Napi::Object jsProgressFeedback = Napi::Object::New(this->Env());
            jsProgressFeedback["totalPercent"] = progressFeedback->totalPercent;
            jsProgressFeedback["filePercent"] = progressFeedback->filePercent;
            jsProgressFeedback["message"] = progressFeedback->message;

            progressMutex.lock();
            this->_jsProgressCallback.Call({ jsProgressFeedback });
            progressMutex.unlock();
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

class RefreshRemoteSourcesWorker : public ProgressNodeSwordInterfaceWorker {
public:
    RefreshRemoteSourcesWorker(SwordFacade& facade,
                               const Napi::Function& jsProgressCallback,
                               const Napi::Function& callback,
                               bool forced)

        : ProgressNodeSwordInterfaceWorker(facade, jsProgressCallback, callback),
          _forced(forced) {}
    
    void progressCallback(unsigned int progressPercentage) {
        this->sendExecutionProgress(progressPercentage, 0, "");
    }

    void Execute(const ExecutionProgress& progress) {
        this->_executionProgress = &progress;
        std::function<void(unsigned int)> _progressCallback = std::bind(&RefreshRemoteSourcesWorker::progressCallback,
                                                                        this,
                                                                        std::placeholders::_1);

        int ret = this->_facade.refreshRemoteSources(this->_forced, &_progressCallback);
        this->_isSuccessful = (ret == 0);
    }

    void OnOK() {
        Napi::HandleScope scope(this->Env());
        Napi::Boolean isSuccessful = Napi::Boolean::New(this->Env(), this->_isSuccessful);
        Callback().Call({ isSuccessful });
    }

private:
    bool _isSuccessful;
    bool _forced;
};

class UninstallModuleWorker : public BaseNodeSwordInterfaceWorker {
public:
    UninstallModuleWorker(SwordFacade& facade, const Napi::Function& callback, std::string moduleName)
        : BaseNodeSwordInterfaceWorker(facade, callback), _moduleName(moduleName) {}

    void Execute(const ExecutionProgress& progress) {
        int ret = this->_facade.uninstallModule(this->_moduleName);
        this->_isSuccessful = (ret == 0);
    }

    void OnOK() {
        Napi::HandleScope scope(this->Env());
        Napi::Boolean isSuccessful = Napi::Boolean::New(this->Env(), this->_isSuccessful);
        Callback().Call({ isSuccessful });
    }

private:
    bool _isSuccessful;
    std::string _moduleName;
};

#endif // _NODE_SWORD_INTERFACE_WORKER

