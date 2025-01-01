/* This file is part of node-sword-interface.

   Copyright (C) 2019 - 2025 Tobias Klein <contact@tklein.info>

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

// Note that this uses the sources within this directory structure.
// After installing via NPM you would use this require call: require('node-sword-interface');
const NodeSwordInterface = require('../index.js');
var interface = new NodeSwordInterface();

function printKjvInfo() {
  // Print some module information
  var kjv = interface.getLocalModule('KJV');
  console.log(kjv.description);
  console.log(kjv.about);
}

function printMatthew() {
  // Get the verses of the Gospel of Matthew
  var verses = interface.getBookText('KJV', 'Mat');

  // Do something with the verses
  for (var i = 0; i < verses.length; i++) {
    var currentVerse = verses[i];
    var verseReference = currentVerse.chapter + ':' + currentVerse.verseNr;
    console.log(verseReference + ' '  + currentVerse.content);
  }
}

printKjvInfo();
printMatthew();
