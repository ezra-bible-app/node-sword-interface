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

using namespace std;

class SwordFacade;

class NodeSwordInterfaceWorker : public Napi::AsyncWorker {
public:
    NodeSwordInterfaceWorker(SwordFacade* facade, std::string operation, std::vector<std::string> args, const Napi::Function& callback)
        : Napi::AsyncWorker(callback), _facade(facade), _operation(operation), _args(args) {}

    ~NodeSwordInterfaceWorker() {}

    // This code will be executed on the worker thread
    void Execute() {
      if (this->_operation == "refreshRepositoryConfig") {
        this->_facade->refreshRepositoryConfig();
      } else if (this->_operation == "refreshRemoteSources") {
        bool force = (this->_args[0] == "true");
        this->_facade->refreshRemoteSources(force);
      } else if (this->_operation == "installModule") {
        this->_facade->installModule(this->_args[0]);
      } else if (this->_operation == "uninstallModule") {
        this->_facade->uninstallModule(this->_args[0]);
      }
    }

    void OnOK() {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Null()});
    }

private:
    SwordFacade* _facade = 0;
    std::string _operation;
    std::vector<std::string> _args;
};

#endif // _NODE_SWORD_INTERFACE_WORKER

