/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>

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


#ifndef _PERCENTAGE_CALC
#define _PERCENTAGE_CALC

// Functions copied and adapted from BibleTime source code
// see https://github.com/bibletime/bibletime/blob/master/src/backend/btinstallmgr.cpp
namespace {

template <typename T>
inline T normalizeCompletionPercentage(const T value) {
    if (value < 0)
        return 0;
    if (value > 100)
        return 100;
    return value;
}

template <typename T>
inline int calculateIntPercentage(T done, T total) {
    if (!(done >= 0 && total >= 0)) {
        return 0;
    }

    // Special care (see warning in InstallModuleWorker::swordUpdateCB()).
    if (done > total)
        done = total;
    if (total == 0)
        return 0;

    return normalizeCompletionPercentage<int>((done / total) * 100);
}

} // anonymous namespace

#endif // _PERCENTAGE_CALC