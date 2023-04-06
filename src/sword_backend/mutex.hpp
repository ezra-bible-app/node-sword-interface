/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2023 Tobias Klein <contact@tklein.info>

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

#ifndef _MUTEX
#define _MUTEX

#if defined(__linux__) || defined(__APPLE__)
    #include <mutex>
#elif _WIN32
    #include <windows.h>
    #include <process.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
    #define MUTEX std::mutex
#elif _WIN32
    #define MUTEX HANDLE
#endif

class Mutex {
public:
    Mutex(){};
    ~Mutex();

    bool init();
    bool lock();
    bool unlock();

private:
    MUTEX _mutex;
};

#endif // _MUTEX