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

#ifndef _INSTALL_MODULE_WORKER
#define _INSTALL_MODULE_WORKER

#include "worker.hpp"
#include "percentage_calc.hpp"
#include "module_installer.hpp"

class InstallModuleWorker : public ProgressWorker {
public:
    InstallModuleWorker(RepositoryInterface& repoInterface,
                        ModuleInstaller& moduleInstaller,
                        const Napi::Function& jsProgressCallback,
                        const Napi::Function& callback,
                        std::string moduleName)

        : ProgressWorker(repoInterface,
                         jsProgressCallback,
                         callback),
                         _moduleInstaller(moduleInstaller),
                         _moduleName(moduleName) {}

    void swordPreStatusCB(long totalBytes, long completedBytes, const char *message);
    void swordUpdateCB(double dltotal, double dlnow);
    void Execute(const ExecutionProgress& progress);
    void OnOK();

private:
    ModuleInstaller& _moduleInstaller;
    int _result;
    std::string _moduleName;
    long _completedBytes = 0;
    long _totalBytes = 0;
    int _totalPercent = 0;
    int _filePercent = 0;
};

#endif // _INSTALL_MODULE_WORKER