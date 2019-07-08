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

#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#elif _WIN32
#include <direct.h>
#include  <io.h>  
#include  <stdio.h>  
#endif

#include <iostream>
#include <string>
#include <sstream>

#include "file_system_helper.hpp"

using namespace std;

// PUBLIC METHODS

FileSystemHelper::FileSystemHelper()
{
}

FileSystemHelper::~FileSystemHelper()
{
}

void FileSystemHelper::createBasicDirectories()
{
    if (!this->fileExists(this->getUserSwordDir())) {
        this->makeDirectory(this->getUserSwordDir());
    }

    if (!this->fileExists(this->getModuleDir())) {
        this->makeDirectory(this->getModuleDir());
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

string FileSystemHelper::getInstallMgrDir()
{
    stringstream installMgrDir;
    installMgrDir << this->getUserSwordDir() << this->getPathSeparator() << "installMgr";
    return installMgrDir.str();
}

string FileSystemHelper::getUserSwordDir()
{
    stringstream swordDir;
    swordDir << this->getUserDir() << this->getPathSeparator();

#if defined(__linux__) || defined(__APPLE__)
    swordDir << ".sword";
#elif _WIN32
    swordDir << this->getPathSeparator() << "sword";
#endif

    return swordDir.str();
}

string FileSystemHelper::getSystemSwordDir()
{
    stringstream swordDir;
    swordDir << this->getSystemDir() << this->getPathSeparator();

#if defined(__linux__) || defined(__APPLE__)
    swordDir << this->getPathSeparator() << "sword" << this->getPathSeparator();
#elif _WIN32
    swordDir << "Application Data" << this->getPathSeparator() << "sword" << this->getPathSeparator();
#endif

    return swordDir.str();
}

// PRIVATE METHODS

bool FileSystemHelper::fileExists(string fileName)
{
    bool exists = false;

#if defined(__linux__) || defined(__APPLE__)
    if (access(fileName.c_str(), F_OK) != -1 ) {
#elif _WIN32
    if( (_access(fileName.c_str(), 0 )) != -1 ) {
#endif
        exists = true;
    }

    return exists;
}

int FileSystemHelper::makeDirectory(string dirName)
{
#if defined(__linux__) || defined(__APPLE__)
    return mkdir(dirName.c_str(), 0700);
#elif _WIN32
    return _mkdir(dirName.c_str());
#endif
}

string FileSystemHelper::getPathSeparator()
{
#if defined(__linux__) || defined(__APPLE__)
    string pathSeparator = "/";
#elif _WIN32
    string pathSeparator = "\\";
#endif
    return pathSeparator;
}

string FileSystemHelper::getUserDir()
{
#if defined(__linux__) || defined(__APPLE__)
    string userDir = string(getenv("HOME"));
#elif _WIN32
    string userDir = string(getenv("APPDATA"));
#endif
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


