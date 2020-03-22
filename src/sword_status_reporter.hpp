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

#ifndef _SWORD_STATUS_REPORTER
#define _SWORD_STATUS_REPORTER

#include <functional>
#include <remotetrans.h>

class SwordStatusReporter : public sword::StatusReporter
{
public:
    void setCallBacks(std::function<void(long, long, const char*)>* preStatusCallback,
                      std::function<void(unsigned long, unsigned long)>* updateCallback);

    virtual void update(unsigned long totalBytes, unsigned long completedBytes);
    virtual void preStatus(long totalBytes, long completedBytes, const char *message);

private:
    std::function<void(long, long, const char*)>* _preStatusCallback = 0;
    std::function<void(unsigned long, unsigned long)>* _updateCallback = 0;
};

#endif // _SWORD_STATUS_REPORTER