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

#ifndef _STRONGS_ENTRY
#define _STRONGS_ENTRY

#include <vector>
#include <string>

namespace sword {
    class SWModule;
};

class StrongsReference
{
public:
    StrongsReference(std::string text);
    virtual ~StrongsReference(){}

    bool isKeyValid();

    std::string text;
    std::string key;

private:
    std::string parseKey(std::string text);
};

class StrongsEntry
{
public:
    StrongsEntry(std::string key, std::string rawEntry);
    virtual ~StrongsEntry(){}

    static StrongsEntry* getStrongsEntry(sword::SWModule* module, std::string key);

    std::string rawEntry;
    std::string key;
    std::string transcription;
    std::string phoneticTranscription;
    std::string definition;
    std::vector<StrongsReference> references;

    void parseFromRawEntry(std::string rawEntry);
    void parseFirstLine(std::string firstLine);
    void eraseEmptyLines(std::vector<std::string>& lines);
    void parseDefinitionAndReferences(std::vector<std::string>& lines);
};

#endif // _STRONGS_ENTRY