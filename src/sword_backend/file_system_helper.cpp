/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2021 Tobias Klein <contact@tklein.info>

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

#if defined(__linux__) || defined(__APPLE__) || defined(__ANDROID__)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#elif _WIN32

#include <direct.h>
#include <io.h>  
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdlib.h>

#endif

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "file_system_helper.hpp"

using namespace std;

// PUBLIC METHODS

FileSystemHelper::FileSystemHelper()
{
    this->_customHomeDir = "";
}

FileSystemHelper::~FileSystemHelper()
{
}

void FileSystemHelper::setCustomHomeDir(std::string customHomeDir)
{
    if (this->fileExists(customHomeDir)) {
        this->_customHomeDir = customHomeDir;
    } else if (customHomeDir != "") {
        cerr << "ERROR: The directory " << customHomeDir << " is not existing!" << endl;
    }
}

void FileSystemHelper::createBasicDirectories()
{
    int ret = 0;

#ifndef __ANDROID__
    #if defined(__linux__) || defined(__APPLE__)
        // On Unix we fix the situation with two different InstallMgr directories (installMgr and InstallMgr)
        // This does not work on Windows or Android, because there we deal with case-insensitive filesystems

        if (this->hasOldInstallMgrDir()) {
            cout << "Detected old InstallMgr directory installMgr." << endl;
            this->fixInstallMgrDir();
        }
    #endif
#endif

    if (!this->fileExists(this->getUserSwordDir())) {
        ret = this->makeDirectory(this->getUserSwordDir());
        if (ret != 0) {
            cerr << "Failed to create user sword dir at " << this->getUserSwordDir() << endl;
        }
    }

    if (!this->fileExists(this->getModuleDir())) {
        ret = this->makeDirectory(this->getModuleDir());
        if (ret != 0) {
            cerr << "Failed to create module dir at " << this->getModuleDir() << endl;
        }
    }
}

bool FileSystemHelper::isSwordConfExisting()
{
    return this->fileExists(this->getSwordConfPath());
}

string FileSystemHelper::getModuleDir()
{
    stringstream moduleDir;
    moduleDir << this->getUserSwordDir() << this->getPathSeparator() << "mods.d";
    return moduleDir.str();
}

string FileSystemHelper::getSwordConfPath()
{
    stringstream configPath;
    configPath << this->getUserSwordDir() << this->getPathSeparator() << "sword.conf";
    return configPath.str();
}

#ifndef __ANDROID__
#if defined(__linux__) || defined(__APPLE__)

string FileSystemHelper::getOldInstallMgrDir()
{
    stringstream installMgrDir;
    installMgrDir << this->getUserSwordDir() << this->getPathSeparator() << "installMgr";
    return installMgrDir.str();
}

bool FileSystemHelper::hasOldInstallMgrDir()
{
    string oldInstallMgrDir = this->getOldInstallMgrDir();
    return this->fileExists(oldInstallMgrDir);
}

void FileSystemHelper::fixInstallMgrDir()
{
    string oldInstallMgrDir = this->getOldInstallMgrDir();
    string newInstallMgrDir = this->getInstallMgrDir();

    stringstream temporaryInstallMgrDir;
    temporaryInstallMgrDir << newInstallMgrDir << "_";
    
    // We need to rename in two steps, because on Windows and Android filesystems may be case-insensitive
    // and the difference of the InstallMgr directory names is only in case (first character of InstallMgr)
    int result1 = this->renameFile(oldInstallMgrDir, temporaryInstallMgrDir.str());
    int result2 = 0;

    if (!this->fileExists(newInstallMgrDir)) {
        result2 = this->renameFile(temporaryInstallMgrDir.str(), newInstallMgrDir);
    } else {
        cout << "Deleting previously used InstallMgr dir!" << endl;
        this->removeDir(temporaryInstallMgrDir.str());
    }

    if (result1 != 0 || result2 != 0) {
        cerr << "Fixing InstallMgr dir failed!" << endl;
    }
}
#endif
#endif

string FileSystemHelper::getInstallMgrDir()
{
    stringstream installMgrDir;
    installMgrDir << this->getUserSwordDir() << this->getPathSeparator() << "InstallMgr";
    return installMgrDir.str();
}

string FileSystemHelper::getUserSwordDir()
{
    stringstream swordDir;
    swordDir << this->getUserDir() << this->getPathSeparator();

#if defined(__ANDROID__)
    swordDir << "sword";
#elif defined(__linux__) || defined(__APPLE__)
    swordDir << ".sword";
#elif _WIN32
    swordDir << "sword";
#endif

    return swordDir.str();
}

string FileSystemHelper::getSystemSwordDir()
{
    stringstream swordDir;
    swordDir << this->getSystemDir() << this->getPathSeparator();

#if defined(__linux__) || defined(__APPLE__) || defined(__ANDROID__)
    swordDir << this->getPathSeparator() << "sword" << this->getPathSeparator();
#elif _WIN32
    swordDir << "Application Data" << this->getPathSeparator() << "sword" << this->getPathSeparator();
#endif

    return swordDir.str();
}

#if defined(_WIN32)
std::string FileSystemHelper::convertUtf16StringToUtf8(const wstring& wstr)
{
    string convertedString;
    int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, 0, 0, 0, 0);

    if(requiredSize > 0) {
        vector<char> buffer(requiredSize);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &buffer[0], requiredSize, 0, 0);
        convertedString.assign(buffer.begin(), buffer.end() - 1);
    }

    return convertedString;
}

wstring FileSystemHelper::convertUtf8StringToUtf16(const string& str)
{
    wstring convertedString;
    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);

    if(requiredSize > 0) {
        vector<wchar_t> buffer(requiredSize);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], requiredSize);
        convertedString.assign(buffer.begin(), buffer.end() - 1);
    }
 
    return convertedString;
}
#endif

// PRIVATE METHODS

bool FileSystemHelper::fileExists(string fileName)
{
    bool exists = false;

#if defined(__linux__) || defined(__APPLE__) || defined(__ANDROID__)
    if (access(fileName.c_str(), F_OK) != -1 ) {
#elif _WIN32
    wstring wFileName = this->convertUtf8StringToUtf16(fileName);

    if (_waccess(wFileName.c_str(), 0) != -1) {
#endif
        exists = true;
    }

    return exists;
}

int FileSystemHelper::makeDirectory(string dirName)
{
#if defined(__linux__) || defined(__APPLE__) || defined(__ANDROID__)
    return mkdir(dirName.c_str(), 0700);
#elif _WIN32
    wstring wDirName = this->convertUtf8StringToUtf16(dirName);

    return _wmkdir(wDirName.c_str());
#endif
}

int FileSystemHelper::renameFile(string oldFileName, string newFileName)
{
    int result = rename(oldFileName.c_str(), newFileName.c_str());
    if (result != 0) {
        cerr << "Could not rename " << oldFileName << " to " << newFileName << endl;
    }

    return result;
}

string FileSystemHelper::getPathSeparator()
{
#if defined(__linux__) || defined(__APPLE__) || defined(__ANDROID__)
    string pathSeparator = "/";
#elif _WIN32
    string pathSeparator = "\\";
#endif
    return pathSeparator;
}

string FileSystemHelper::getUserDir()
{
    string userDir;

    if (this->_customHomeDir == "") {

        #if defined(__ANDROID__)
            userDir = "/sdcard/Documents";
        #elif defined(__linux__) || defined(__APPLE__)
            userDir = string(getenv("HOME"));
        #elif _WIN32
            wstring wUserDir = wstring(_wgetenv(L"APPDATA"));
            userDir = this->convertUtf16StringToUtf8(wUserDir);
        #endif

    } else {
        userDir = this->_customHomeDir;
    }

    return userDir;
}

string FileSystemHelper::getSystemDir()
{
#if defined(__linux__) || defined(__APPLE__)
    string systemDir = "/usr/share";
#elif _WIN32
    string systemDir = string(getenv("AllUsersProfile"));
#endif
    return systemDir;
}

vector<string> FileSystemHelper::getFilesInDir(string dirName)
{
    vector<string> files;

#if defined(__linux__) || defined(__ANDROID__) || defined(__APPLE__)
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(dirName.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                files.push_back(ent->d_name);
            }
        }

        closedir(dir);
    } else {
      cerr << "getFilesInDir: ERROR calling opendir for " << dirName << " / errno: " << errno << endl;
    }

#elif _WIN32
    // The following code is based on
    // https://docs.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
    wstring wDirName = this->convertUtf8StringToUtf16(dirName);
    
    WIN32_FIND_DATAW ffd;
    WCHAR szDir[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Prepare string for use with FindFile functions.  First, copy the
    // string to a buffer, then append '\*' to the directory name.
    StringCchCopyW(szDir, MAX_PATH, wDirName.c_str());
    StringCchCatW(szDir, MAX_PATH, L"\\*");

    // Find the first file in the directory.
    hFind = FindFirstFileW(szDir, &ffd);

    if (INVALID_HANDLE_VALUE != hFind) {
        do {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                wstring stdFileName = wstring(ffd.cFileName);
                string utf8FileName = this->convertUtf16StringToUtf8(stdFileName);
                files.push_back(utf8FileName);
            }
        } while (FindNextFileW(hFind, &ffd) != 0);

        FindClose(hFind);
    }
#endif

    return files;
}

#ifndef __ANDROID__
#if defined(__linux__) || defined(__APPLE__)
void FileSystemHelper::removeDir(std::string dirName)
{
    const char *path = dirName.c_str();
    struct dirent *entry = NULL;
    DIR *dir = NULL;
    dir = opendir(path);

    while ((entry = readdir(dir))) {   
        DIR *sub_dir = NULL;
        FILE *file = NULL;
        char* abs_path = new char[257];

        if ((*(entry->d_name) != '.') || ((strlen(entry->d_name) > 1) && (entry->d_name[1] != '.'))) {   
            sprintf(abs_path, "%s/%s", path, entry->d_name);

            if((sub_dir = opendir(abs_path))) {
                closedir(sub_dir);
                this->removeDir(string(abs_path));
            } else {
                if ((file = fopen(abs_path, "r"))) {
                    fclose(file);
                    remove(abs_path);
                }
            }
        }

        delete[] abs_path;
    }

    remove(path);
}
#endif
#endif