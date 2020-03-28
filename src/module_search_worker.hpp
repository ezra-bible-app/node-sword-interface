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

#ifndef _MODULE_SEARCH_WORKER
#define _MODULE_SEARCH_WORKER

#include "node_sword_interface_worker.hpp"

class ModuleSearchWorker : public ProgressNodeSwordInterfaceWorker {
public:
    ModuleSearchWorker(SwordFacade& facade,
                       const Napi::Function& jsProgressCallback,
                       const Napi::Function& callback,
                       std::string moduleName,
                       std::string searchTerm,
                       SearchType searchType,
                       bool isCaseSensitive=false)

        : ProgressNodeSwordInterfaceWorker(facade, jsProgressCallback, callback),
        _moduleName(moduleName),
        _searchTerm(searchTerm),
        _searchType(searchType),
        _isCaseSensitive(isCaseSensitive) {

        this->_napiSwordHelper = new NapiSwordHelper(facade);
    }

    void searchProgressCB(char percent, void* userData);
    void Execute(const ExecutionProgress& progress);    
    void OnOK();

private:
    NapiSwordHelper* _napiSwordHelper;
    std::vector<Verse> _stdSearchResults;
    Napi::Array _napiSearchResults;
    std::string _moduleName;
    std::string _searchTerm;
    SearchType _searchType;
    bool _isCaseSensitive;
};

#endif // _MODULE_SEARCH_WORKER