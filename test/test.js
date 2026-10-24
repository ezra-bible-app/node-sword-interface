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

const NodeSwordInterface = require('../index.js');

describe('NodeSwordInterface', () => {
  let nsi;

  beforeAll(async () => {
    nsi = new NodeSwordInterface();

    kjv = nsi.getLocalModule('KJV');

    if (!kjv) {
      console.log('KJV module not found, installing ...');
      await nsi.installModule('CrossWire', 'KJV');
      kjv = nsi.getLocalModule('KJV');
    }

    expect(kjv).toBeDefined();
  }, 60000);

  test('should initialize NodeSwordInterface instance', () => {
    expect(nsi).toBeDefined();
  });

  test('should return an error on parallely executed modules searches', async () => {
    const moduleCode = 'KJV';
    const searchTerm = 'faith';

    const firstResult = nsi.getModuleSearchResults(moduleCode, searchTerm);
    expect(firstResult).toBeInstanceOf(Promise);

    for (let i = 0; i < 10; i++) {
      let additional_result = nsi.getModuleSearchResults(moduleCode, searchTerm);
      expect(additional_result).rejects.toThrow('Module search in progress. Wait until it is finished.');
    }

    const search_results = await firstResult; // Wait for the first result to resolve
    expect(search_results.length).toBeGreaterThan(0);
  });

  test('should preserve results when switching search scopes', async () => {
    const search = (scope) => nsi.getModuleSearchResults('KJV', 'God', undefined, 'phrase', scope);
    const hasReference = (results, book) => results.some((verse) =>
      verse.bibleBookShortTitle === book && verse.chapter === 1 && verse.verseNr === 1);

    const bibleResults = await search('BIBLE');
    const oldTestamentResults = await search('OT');
    const newTestamentResults = await search('NT');
    const genesisResults = await search('Gen');

    expect(hasReference(bibleResults, 'Gen')).toBe(true);
    expect(hasReference(bibleResults, 'John')).toBe(true);
    expect(hasReference(oldTestamentResults, 'Gen')).toBe(true);
    expect(hasReference(oldTestamentResults, 'John')).toBe(false);
    expect(hasReference(newTestamentResults, 'John')).toBe(true);
    expect(hasReference(newTestamentResults, 'Gen')).toBe(false);
    expect(hasReference(genesisResults, 'Gen')).toBe(true);
    expect(genesisResults.every((verse) => verse.bibleBookShortTitle === 'Gen')).toBe(true);
  }, 30000);

  test('should reject unknown book scopes and allow the next search', async () => {
    await expect(nsi.getModuleSearchResults('KJV', 'God', undefined, 'phrase', 'NotABook'))
      .rejects.toThrow('Unknown or unsupported book search scope!');

    const results = await nsi.getModuleSearchResults('KJV', 'God');
    expect(results.length).toBeGreaterThan(0);
  }, 30000);
});
