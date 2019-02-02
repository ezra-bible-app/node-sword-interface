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

const ezraSwordInterfaceModule = require('./build/Release/ezra-sword-interface.node');
const ezraSwordInterface = new ezraSwordInterfaceModule.EzraSwordInterface();

module.exports = ezraSwordInterface;

