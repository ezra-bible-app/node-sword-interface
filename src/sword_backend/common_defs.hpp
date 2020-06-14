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

#ifndef _COMMON_DEFS
#define _COMMON_DEFS

#include <string>

enum class QueryLimit {
    none,
    chapter,
    book
};

enum class ModuleType {
    bible,
    dict,
    any
};

class Verse
{
public:
    Verse() {};
    virtual ~Verse() {};

    std::string reference;
    int absoluteVerseNumber;
    std::string content;
};

#endif // _COMMON_DEFS