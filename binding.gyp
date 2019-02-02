/* This file is part of ezra-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@tklein.info>

   ezra-sword-interface is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ezra-sword-interface is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ezra-sword-interface. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>. */

{
    "targets": [{
        "target_name": "ezra_sword_interface",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions -std=c++11 -pthread" ],
        "sources": [
            "ezra_sword_interface.cpp",
            "js_ezra_sword_interface.cpp",
            "binding.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "/usr/include/sword"
        ],
        'libraries': [ 
            '-lsword',
            '/usr/lib/x86_64-linux-gnu/libcurl.so.4'
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }]
}
