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

#ifndef _FILE_SYSTEM_HELPER
#define _FILE_SYSTEM_HELPER

class FileSystemHelper
{
public:
    FileSystemHelper();
    virtual ~FileSystemHelper();

    void setCustomHomeDir(std::string customHomeDir);
    void createBasicDirectories();
    bool isSwordConfExisting();
    std::string getModuleDir();
    std::string getSwordConfPath();
    std::string getInstallMgrDir();

#ifndef __ANDROID__
    #if defined(__linux__) || defined(__APPLE__)

    // We started out with installMgr instead of InstallMgr (lowercase instead of uppercase first letter).
    // These methods are fixing this situation by renaming installMgr to InstallMgr, which is the default
    // also used by other SWORD frontends.
    std::string getOldInstallMgrDir();
    bool hasOldInstallMgrDir();
    void fixInstallMgrDir();

    #endif
#endif

    std::string getUserSwordDir();
    std::string getSystemSwordDir();
    std::vector<std::string> getFilesInDir(std::string dirName);
    std::string getPathSeparator();
    
    bool fileExists(std::string fileName);

#if defined(_WIN32)
    std::wstring convertUtf8StringToUtf16(const std::string& str);
    std::string convertUtf16StringToUtf8(const std::wstring& wstr);
#endif

private:
    int makeDirectory(std::string dirName);
    int renameFile(std::string oldFileName, std::string newFileName);

#ifndef __ANDROID__
    #if defined(__linux__) || defined(__APPLE__)

    void removeDir(std::string dirName);

    #endif
#endif

    std::string getUserDir();
    std::string getSystemDir();
    std::string _customHomeDir;
};

#endif // _FILE_SYSTEM_HELPER

