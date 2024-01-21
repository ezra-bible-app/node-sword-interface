/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2024 Tobias Klein <contact@tklein.info>

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

#include "string_helper.hpp"

using namespace std;

void StringHelper::rtrim(string& s, const string& delimiters )
{
   s.erase( s.find_last_not_of( delimiters ) + 1 );
}
 
void StringHelper::ltrim(string& s,  const string& delimiters )
{
   s.erase( 0, s.find_first_not_of( delimiters ) );
}
 
void StringHelper::trim(string& s, const string& delimiters )
{
    s.erase( s.find_last_not_of( delimiters ) + 1 ).erase( 0, s.erase( s.find_last_not_of( delimiters ) + 1 ).find_first_not_of( delimiters ) );
}

// taken from https://stackoverflow.com/a/874160
bool StringHelper::hasEnding(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool StringHelper::hasBeginning(std::string const &fullString, std::string const &beginning) {
    if (fullString.length() >= beginning.length()) {
        return (0 == fullString.compare (0, beginning.length(), beginning));
    } else {
        return false;
    }
}

// from https://stackoverflow.com/a/46943631
vector<string> StringHelper::split(string str, string token) {
    vector<string>result;
    
    while (str.size()) {
        std::size_t index = str.find(token);

        if (index != string::npos) {
            result.push_back(str.substr(0,index));
            str = str.substr(index + token.size());
            if (str.size() == 0) result.push_back(str);
        } else {
            result.push_back(str);
            str = "";
        }
    }

    return result;
}

int StringHelper::numberOfSubstrings(const std::string& str, const std::string& token) {
    if (token.length() == 0) {
        return 0;
    }

    int count = 0;

    for (size_t offset = str.find(token); offset != std::string::npos; offset = str.find(token, offset + token.length())) {
        count++;
    }

    return count;
}