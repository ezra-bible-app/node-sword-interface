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

#ifndef _FILE_SYSTEM_HELPER
#define _FILE_SYSTEM_HELPER

class FileSystemHelper
{
public:
    FileSystemHelper();
    virtual ~FileSystemHelper();

    void createBasicDirectories();
    bool isSwordConfExisting();
    std::string getModuleDir();
    std::string getSwordConfPath();
    std::string getInstallMgrDir();

private:
    bool fileExists(std::string fileName);
    int makeDirectory(std::string dirName);
    std::string getPathSeparator();
    std::string getUserDir();
    std::string getSwordDir();
};

#endif // _FILE_SYSTEM_HELPER

