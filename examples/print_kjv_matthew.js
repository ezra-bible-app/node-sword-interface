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