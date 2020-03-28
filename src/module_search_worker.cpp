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

#include "worker.hpp"
#include "module_search_worker.hpp"
#include "sword_facade.hpp"

static std::mutex searchMutex;

void ModuleSearchWorker::Execute(const ExecutionProgress& progress)
{
    searchMutex.lock();

    this->_executionProgress = &progress;
    
    std::function<void(char, void*)> searchProgressCB = std::bind(&ModuleSearchWorker::searchProgressCB,
                                                                  this,
                                                                  std::placeholders::_1,
                                                                  std::placeholders::_2);
    setModuleSearchProgressCB(&searchProgressCB);
    this->_stdSearchResults = this->_facade.getModuleSearchResults(this->_moduleName,
                                                                   this->_searchTerm,
                                                                   this->_searchType,
                                                                   this->_isCaseSensitive);
    searchMutex.unlock();
}

void ModuleSearchWorker::searchProgressCB(char percent, void* userData)
{
    this->sendExecutionProgress((int)percent, 0, "");
}

void ModuleSearchWorker::OnOK()
{
    Napi::HandleScope scope(this->Env());
    this->_napiSearchResults = this->_napiSwordHelper->getNapiVerseObjectsFromRawList(this->Env(), this->_moduleName, this->_stdSearchResults);
    Callback().Call({ this->_napiSearchResults });
}