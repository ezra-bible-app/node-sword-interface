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

#include "api_lock.hpp"
#include "worker.hpp"
#include "install_module_worker.hpp"

void InstallModuleWorker::swordPreStatusCB(long totalBytes, long completedBytes, const char *message)
{

    this->sendExecutionProgress(0, 0, string(message));
    this->_completedBytes = completedBytes;
    this->_totalBytes = totalBytes;
}

// Function copied and adapted from BibleTime source code
// see https://github.com/bibletime/bibletime/blob/master/src/backend/btinstallmgr.cpp
void InstallModuleWorker::swordUpdateCB(double dltotal, double dlnow)
{
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


    this->sendExecutionProgress(totalPercent, filePercent, "");
}

void InstallModuleWorker::Execute(const ExecutionProgress& progress)
{
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

    SwordStatusReporter& statusReporter = this->_facade.getStatusReporter();
    statusReporter.setCallBacks(&_swordPreStatusCB, &_swordUpdateCB);
    int ret = this->_facade.installModule(this->_moduleName);
    statusReporter.resetCallbacks();
    this->_isSuccessful = (ret == 0);
    unlockApi();
}

void InstallModuleWorker::OnOK()
{
    Napi::HandleScope scope(this->Env());
    Napi::Boolean isSuccessful = Napi::Boolean::New(this->Env(), this->_isSuccessful);
    Callback().Call({ isSuccessful });
}