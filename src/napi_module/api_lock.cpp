/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>

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

#include "api_lock.hpp"

static MUTEX apiMutex;

bool initLock()
{
    #if defined(__linux__) || defined(__APPLE__)
        return pthread_mutex_init(&apiMutex, NULL);
    #elif _WIN32
        apiMutex = CreateMutex(0, FALSE, 0);
        return (apiMutex == 0);
    #endif

    return false;
}

bool lockApi()
{
    #if defined(__linux__) || defined(__APPLE__)
        return pthread_mutex_lock(&apiMutex) == 0;
    #elif _WIN32
        return (WaitForSingleObject(&apiMutex, INFINITE) == WAIT_FAILED ? false : true);
    #endif

    return false;
}

bool unlockApi()
{
    #if defined(__linux__) || defined(__APPLE__)
        return pthread_mutex_unlock(&apiMutex) == 0;
    #elif _WIN32
        return (ReleaseMutex(&apiMutex) == 0);
    #endif

    return false;
}