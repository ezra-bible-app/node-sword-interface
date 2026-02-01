/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2026 Tobias Klein <contact@tklein.info>

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

#ifndef _MODULE_SEARCH_WORKER
#define _MODULE_SEARCH_WORKER

#include "worker.hpp"
#include "module_search.hpp"
#include "module_store.hpp"
#include "mutex.hpp"

class ModuleSearchWorker : public ProgressWorker {
public:
    ModuleSearchWorker(ModuleHelper& moduleHelper,
                       ModuleSearch& moduleSearch,
                       ModuleStore& moduleStore,
                       RepositoryInterface & repoInterface,
                       Mutex& searchMutex,
                       const Napi::Function& jsProgressCallback,
                       const Napi::Function& callback,
                       std::string moduleName,
                       std::string searchTerm,
                       SearchType searchType,
                       SearchScope searchScope,
                       bool isCaseSensitive=false,
                       bool useExtendedVerseBoundaries=false,
                       bool filterOnWordBoundaries=false)

        : ProgressWorker(repoInterface, jsProgressCallback, callback),
        _searchMutex(searchMutex),
        _moduleSearch(moduleSearch),
        _moduleName(moduleName),
        _searchTerm(searchTerm),
        _searchType(searchType),
        _searchScope(searchScope),
        _isCaseSensitive(isCaseSensitive),
        _useExtendedVerseBoundaries(useExtendedVerseBoundaries),
        _filterOnWordBoundaries(filterOnWordBoundaries),
        _searchTerminated(false) {

        this->_napiSwordHelper = new NapiSwordHelper(moduleHelper, moduleStore);
    }

    void searchProgressCB(char percent, void* userData);
    void Execute(const ExecutionProgress& progress);    
    void OnOK();
    void terminateSearch();

private:
    Mutex& _searchMutex;
    ModuleSearch& _moduleSearch;
    NapiSwordHelper* _napiSwordHelper;
    std::vector<Verse> _stdSearchResults;
    Napi::Array _napiSearchResults;
    std::string _moduleName;
    std::string _searchTerm;
    SearchType _searchType;
    SearchScope _searchScope;
    bool _isCaseSensitive;
    bool _useExtendedVerseBoundaries;
    bool _filterOnWordBoundaries; // New member variable
    bool _searchTerminated;
};

#endif // _MODULE_SEARCH_WORKER