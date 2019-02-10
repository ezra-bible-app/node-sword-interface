/* This file is part of ezra-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@tklein.info>

   ezra-sword-interface is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ezra-sword-interface is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ezra-sword-interface. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>. */

#include <napi.h>
#include <iostream>

using namespace std;

class EzraSwordInterface;

class JsEzraSwordInterfaceWorker : public Napi::AsyncWorker {
public:
    JsEzraSwordInterfaceWorker(EzraSwordInterface* interface, std::string operation, std::vector<std::string> args, const Napi::Function& callback)
        : Napi::AsyncWorker(callback), _interface(interface), _operation(operation), _args(args) {}

    ~JsEzraSwordInterfaceWorker() {}

    // This code will be executed on the worker thread
    void Execute() {
        if (this->_operation == "refreshRemoteSources") {
            this->_interface->refreshRemoteSources();
        } else if (this->_operation == "installModule") {
            this->_interface->installModule(this->_args[0]);
        }
    }

    void OnOK() {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Null()});
    }

private:
    EzraSwordInterface* _interface = 0;
    std::string _operation;
    std::vector<std::string> _args;
};

