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

// STD C++ includes
#include <iostream>

// Own includes
#include "sword_status_reporter.hpp"

using namespace std;

void SwordStatusReporter::setCallBacks(std::function<void(long, long, const char*)>* preStatusCallback,
                                       std::function<void(unsigned long, unsigned long)>* updateCallback) {
    this->_preStatusCallback = preStatusCallback;
    this->_updateCallback = updateCallback;
}

void SwordStatusReporter::update(unsigned long totalBytes, unsigned long completedBytes)
{
    /*int p = (totalBytes > 0) ? (int)(74.0 * ((double)completedBytes / (double)totalBytes)) : 0;
    for (;last < p; ++last) {
        if (!last) {
            SWBuf output;
            output.setFormatted("[ File Bytes: %ld", totalBytes);
            while (output.size() < 75) output += " ";
            output += "]";
            cout << output.c_str() << "\n ";
        }
        cout << "-";
    }
    cout.flush();*/

    if (this->_updateCallback != 0) {
      (*(this->_updateCallback))(totalBytes, completedBytes);
    }
}

void SwordStatusReporter::preStatus(long totalBytes, long completedBytes, const char *message)
{
    /*SWBuf output;
    output.setFormatted("[ Total Bytes: %ld; Completed Bytes: %ld", totalBytes, completedBytes);
    while (output.size() < 75) output += " ";
    output += "]";
    cout << "\n" << output.c_str() << "\n ";
    int p = (int)(74.0 * (double)completedBytes/totalBytes);
    for (int i = 0; i < p; ++i) { cout << "="; }
    cout << "\n\n" << message << "\n";
    last = 0;*/

    //cout << "\n" << message << "\n";

    if (this->_preStatusCallback != 0) {
      (*(this->_preStatusCallback))(totalBytes, completedBytes, message);
    }
}