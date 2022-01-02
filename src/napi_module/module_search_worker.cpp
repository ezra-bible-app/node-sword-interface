/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2022 Tobias Klein <contact@tklein.info>

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

#include "api_lock.hpp"
#include "worker.hpp"
#include "module_search_worker.hpp"
#include "common_defs.hpp"

void ModuleSearchWorker::Execute(const ExecutionProgress& progress)
{
    this->_searchMutex.lock();
    this->_executionProgress = &progress;
    this->_searchTerminated = false;
    
    std::function<void(char, void*)> searchProgressCB = std::bind(&ModuleSearchWorker::searchProgressCB,
                                                                  this,
                                                                  std::placeholders::_1,
                                                                  std::placeholders::_2);
    setModuleSearchProgressCB(&searchProgressCB);
    this->_stdSearchResults = this->_moduleSearch.getModuleSearchResults(this->_moduleName,
                                                                         this->_searchTerm,
                                                                         this->_searchType,
                                                                         this->_searchScope,
                                                                         this->_isCaseSensitive,
                                                                         this->_useExtendedVerseBoundaries);
    
    if (this->_searchTerminated) {
      this->_stdSearchResults.clear();
    }

    setModuleSearchProgressCB(0);
    this->_searchMutex.unlock();
    unlockApi();
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

void ModuleSearchWorker::terminateSearch()
{
    this->_searchTerminated = true;
    this->_moduleSearch.terminate();
}