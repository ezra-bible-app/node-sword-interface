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

namespace {

template <typename T>
inline T normalizeCompletionPercentage(const T value) {
    if (value < 0)
        return 0;
    if (value > 100)
        return 100;
    return value;
}

template <typename T>
inline int calculateIntPercentage(T done, T total) {
    // BT_ASSERT(done >= 0);
    // BT_ASSERT(total >= 0);

    // Special care (see warning in BtInstallMgr::statusUpdate()).
    if (done > total)
        done = total;
    if (total == 0)
        return 100;

    return normalizeCompletionPercentage<int>((done / total) * 100);
}

} // anonymous namespace

class SwordProgressFeedback {
public:
    int totalPercent;
    int filePercent;
    std::string message;
};

class BaseNodeSwordInterfaceWorker : public Napi::AsyncProgressWorker<SwordProgressFeedback> {
public:
    BaseNodeSwordInterfaceWorker(SwordFacade* facade, SwordStatusReporter* statusReporter, const Napi::Function& callback)
        : Napi::AsyncProgressWorker<SwordProgressFeedback>(callback), _facade(facade), _statusReporter(statusReporter) {}

    virtual ~BaseNodeSwordInterfaceWorker() {}

    virtual void OnOK() {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Null()});
    }

    virtual void OnProgress(const SwordProgressFeedback* progressFeedback, size_t /* count */) {
        Napi::HandleScope scope(this->Env());
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
            SwordProgressFeedback feedback;

            feedback.totalPercent = 0;
            feedback.filePercent = 0;
            feedback.message = string(message);
            this->_executionProgress->Send(&feedback, 1);
        }

        this->_completedBytes = completedBytes;
        this->_totalBytes = totalBytes;
    }

    void swordUpdateCB(double dltotal, double dlnow) {
        /**
                WARNING

                Note that these *might be* rough measures due to the double data
                type being used by Sword to store the number of bytes. Special
                care must be taken to work around this, since the arguments may
                contain weird values which would otherwise break this logic.
        */

        if (dltotal < 0.0) // Special care (see warning above)
            dltotal = 0.0;
        if (dlnow < 0.0) // Special care (see warning above)
            dlnow = 0.0;

        const int totalPercent = calculateIntPercentage<double>(dlnow + this->_completedBytes,
                                                                this->_totalBytes);
        const int filePercent  = calculateIntPercentage(dlnow, dltotal);


        if (this->_executionProgress != 0) {
            SwordProgressFeedback feedback;

            feedback.totalPercent = totalPercent;
            feedback.filePercent = filePercent;
            feedback.message = "";
            this->_executionProgress->Send(&feedback, 1);
        }
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

    virtual void OnProgress(const SwordProgressFeedback* progressFeedback, size_t /* count */) {
        Napi::HandleScope scope(this->Env());

        if (progressFeedback != 0) {
            Napi::Object jsProgressFeedback = Napi::Object::New(this->Env());
            jsProgressFeedback["totalPercent"] = progressFeedback->totalPercent;
            jsProgressFeedback["filePercent"] = progressFeedback->filePercent;
            jsProgressFeedback["message"] = progressFeedback->message;

            this->_jsProgressCallback.Call({ jsProgressFeedback });
        }
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
    long _completedBytes = 0;
    long _totalBytes = 0;
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

