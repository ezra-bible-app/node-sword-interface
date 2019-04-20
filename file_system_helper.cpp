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

#ifdef __linux__
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
    if (!this->fileExists(this->getSwordDir())) {
        this->makeDirectory(this->getSwordDir());
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
    moduleDir << this->getSwordDir() << this->getPathSeparator() << "mods.d";
    return moduleDir.str();
}

string FileSystemHelper::getSwordConfPath()
{
    stringstream configPath;
    configPath << this->getSwordDir() << this->getPathSeparator() << "sword.conf";
    return configPath.str();
}

string FileSystemHelper::getInstallMgrDir()
{
    stringstream installMgrDir;
    installMgrDir << this->getSwordDir() << this->getPathSeparator() << "installMgr";
    return installMgrDir.str();
}

// PRIVATE METHODS

bool FileSystemHelper::fileExists(string fileName)
{
    bool exists = false;

#ifdef _WIN32
    if( (_access(fileName.c_str(), 0 )) != -1 ) {
#elif __linux__
    if (access(fileName.c_str(), F_OK) != -1 ) {
#endif
        exists = true;
    }

    return exists;
}

int FileSystemHelper::makeDirectory(string dirName)
{
#ifdef __linux__
    return mkdir(dirName.c_str(), 0700);
#elif _WIN32
    return _mkdir(dirName.c_str());
#endif
}

string FileSystemHelper::getPathSeparator()
{
#ifdef __linux__
    string pathSeparator = "/";
#elif _WIN32
    string pathSeparator = "\\";
#endif
    return pathSeparator;
}

string FileSystemHelper::getUserDir()
{
#ifdef __linux__
    string userDir = string(getenv("HOME"));
#elif _WIN32
    string userDir = string(getenv("AllUsersProfile"));
#endif
    return userDir;
}

string FileSystemHelper::getSwordDir()
{
    stringstream swordDir;
    swordDir << this->getUserDir() << this->getPathSeparator();

#ifdef _WIN32
    swordDir << "Application Data" << this->getPathSeparator() << "sword" << this->getPathSeparator();
#elif __linux__
    swordDir << ".sword" << this->getPathSeparator();
#endif

    return swordDir.str();
}


