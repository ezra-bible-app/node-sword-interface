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
#include <mutex>

#include "napi_sword_helper.hpp"
#include "sword_facade.hpp"
#include "sword_status_reporter.hpp"

using namespace std;

class BaseNodeSwordInterfaceWorker : public Napi::AsyncProgressWorker<uint32_t> {
public:
    BaseNodeSwordInterfaceWorker(SwordFacade* facade, SwordStatusReporter* statusReporter, const Napi::Function& callback)
        : Napi::AsyncProgressWorker<uint32_t>(callback), _facade(facade), _statusReporter(statusReporter) {}

    virtual ~BaseNodeSwordInterfaceWorker() {}

    virtual void OnOK() {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Null()});
    }

    virtual void OnProgress(const uint32_t* data, size_t /* count */) {
        Napi::HandleScope scope(Env());

        //Callback().Call({Env().Null(), Env().Null(), Napi::Number::New(Env(), *data)});
    }

protected:
    SwordFacade* _facade;
    SwordStatusReporter* _statusReporter;
};

class RefreshRemoteSourcesWorker : public BaseNodeSwordInterfaceWorker {
public:
    RefreshRemoteSourcesWorker(SwordFacade* facade, const Napi::Function& callback, bool forced)
        : BaseNodeSwordInterfaceWorker(facade, 0, callback), _forced(forced) {}

    void Execute(const ExecutionProgress& progress) {
        int ret = this->_facade->refreshRemoteSources(this->_forced);
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

static std::mutex searchMutex;

class GetModuleSearchResultWorker : public BaseNodeSwordInterfaceWorker {
public:
    GetModuleSearchResultWorker(SwordFacade* facade,
                                const Napi::Function& callback,
                                std::string moduleName,
                                std::string searchTerm,
                                SearchType searchType,
                                bool isCaseSensitive=false)

        : BaseNodeSwordInterfaceWorker(facade, 0, callback),
        _moduleName(moduleName),
        _searchTerm(searchTerm),
        _searchType(searchType),
        _isCaseSensitive(isCaseSensitive) {}

    void Execute(const ExecutionProgress& progress) {
        searchMutex.lock();
        this->_stdSearchResults = this->_facade->getModuleSearchResults(this->_moduleName,
                                                                        this->_searchTerm,
                                                                        this->_searchType,
                                                                        this->_isCaseSensitive);
        searchMutex.unlock();
    }
    
    void OnOK() {
        Napi::HandleScope scope(this->Env());
        this->_napiSearchResults = this->_napiSwordHelper.getNapiVerseObjectsFromRawList(this->Env(), this->_moduleName, this->_stdSearchResults);
        Callback().Call({ this->_napiSearchResults });
    }

private:
    NapiSwordHelper _napiSwordHelper;
    std::vector<Verse> _stdSearchResults;
    Napi::Array _napiSearchResults;
    std::string _moduleName;
    std::string _searchTerm;
    SearchType _searchType;
    bool _isCaseSensitive;
};

class InstallModuleWorker : public BaseNodeSwordInterfaceWorker {
public:
    InstallModuleWorker(SwordFacade* facade, SwordStatusReporter* statusReporter, const Napi::Function& jsProgressCallback, const Napi::Function& callback, std::string moduleName)
        : BaseNodeSwordInterfaceWorker(facade, statusReporter, callback), _moduleName(moduleName), _jsProgressCallback(Napi::Persistent(jsProgressCallback)) {}

    void swordPreStatusCB(long totalBytes, long completedBytes, const char *message) {
        //cout << "swordPreStatusCB" << endl;

        if (this->_executionProgress != 0) {
            uint32_t progress = 1;
            this->_executionProgress->Send(&progress, 1);
        }
    }

    void swordUpdateCB(unsigned long totalBytes, unsigned long completedBytes) {
        //cout << "swordUpdateCB" << endl;
    }

    void Execute(const ExecutionProgress& progress) {
        this->_executionProgress = &progress;

        std::function<void(long, long, const char*)> _swordPreStatusCB = std::bind(&InstallModuleWorker::swordPreStatusCB,
                                                                                   this,
                                                                                   std::placeholders::_1,
                                                                                   std::placeholders::_2,
                                                                                   std::placeholders::_3);
        
        std::function<void(unsigned long, unsigned long)> _swordUpdateCB = std::bind(&InstallModuleWorker::swordUpdateCB,
                                                                                     this,
                                                                                     std::placeholders::_1,
                                                                                     std::placeholders::_2);

        this->_statusReporter->setCallBacks(&_swordPreStatusCB, &_swordUpdateCB);

        cout << "Starting module installation" << endl;
        int ret = this->_facade->installModule(this->_moduleName);
        cout << "Done!" << endl;
        this->_isSuccessful = (ret == 0);
    }

    virtual void OnProgress(const uint32_t* data, size_t /* count */) {
        Napi::HandleScope scope(this->Env());
        this->_jsProgressCallback.Call({ Napi::Number::New(this->Env(), *data) });
    }

    void OnOK() {
        Napi::HandleScope scope(this->Env());
        Napi::Boolean isSuccessful = Napi::Boolean::New(this->Env(), this->_isSuccessful);
        Callback().Call({ isSuccessful });
    }

private:
    const ExecutionProgress* _executionProgress = 0;
    bool _isSuccessful;
    std::string _moduleName;
    Napi::FunctionReference _jsProgressCallback;
};

class UninstallModuleWorker : public BaseNodeSwordInterfaceWorker {
public:
    UninstallModuleWorker(SwordFacade* facade, SwordStatusReporter* statusReporter, const Napi::Function& callback, std::string moduleName)
        : BaseNodeSwordInterfaceWorker(facade, statusReporter, callback), _moduleName(moduleName) {}

    void Execute(const ExecutionProgress& progress) {
        int ret = this->_facade->uninstallModule(this->_moduleName);
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

