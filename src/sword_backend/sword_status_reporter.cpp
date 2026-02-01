/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2026 Tobias Klein <contact@tklein.info>

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

#include "sword_status_reporter.hpp"

using namespace std;

void SwordStatusReporter::setCallBacks(std::function<void(long, long, const char*)>* preStatusCallback,
                                       std::function<void(unsigned long, unsigned long)>* updateCallback) {
    this->_preStatusCallback = preStatusCallback;
    this->_updateCallback = updateCallback;
}

void SwordStatusReporter::update(unsigned long totalBytes, unsigned long completedBytes)
{
    if (this->_updateCallback != 0) {
        (*(this->_updateCallback))(totalBytes, completedBytes);
    }
}

void SwordStatusReporter::preStatus(long totalBytes, long completedBytes, const char *message)
{
    if (this->_preStatusCallback != 0) {
        (*(this->_preStatusCallback))(totalBytes, completedBytes, message);
    }
}

void SwordStatusReporter::resetCallbacks()
{
    this->_updateCallback = 0;
    this->_preStatusCallback = 0;
}