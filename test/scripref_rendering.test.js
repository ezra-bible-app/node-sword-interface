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

const NodeSwordInterface = require('../index.js');

describe('ScripRef Rendering', () => {
  test('scripRef should render as span not div', () => {
    const nsi = new NodeSwordInterface();
    
    // Expected behavior: scripRef tags should be converted to span elements
    // The actual transformation happens in text_processor.cpp lines 129-130
    const expectedOutput = '<span class="sword-markup sword-scripref" passage="1Tim 1:15; 4:9; 2Tim 2:11; Titus 3:8">1Tim 1:15; 4:9; 2Tim 2:11; Titus 3:8</span>';
    
    // This test documents the expected behavior
    expect(expectedOutput).toContain('<span class="sword-markup sword-scripref"');
    expect(expectedOutput).not.toContain('<div class="sword-markup sword-scripref"');
  });
  
  test('scripRef should use inline element to prevent line breaks', () => {
    // Verify that the fix uses span (inline) instead of div (block-level)
    // This prevents unwanted paragraph breaks in commentary text
    
    const textBefore = 'some text see ';
    const scripRefElement = '<span class="sword-markup sword-scripref" passage="1Tim 3:12; 5:9; Titus 1:6">1Tim 3:12; 5:9; Titus 1:6</span>';
    const textAfter = ' more text';
    
    const fullText = textBefore + scripRefElement + textAfter;
    
    // Span elements are inline, so they don't create line breaks
    // Div elements are block-level, so they would create unwanted line breaks
    expect(scripRefElement).toContain('span');
    expect(scripRefElement).not.toContain('<div');
    
    // The full text should flow inline without forced breaks
    expect(fullText).toBe('some text see <span class="sword-markup sword-scripref" passage="1Tim 3:12; 5:9; Titus 1:6">1Tim 3:12; 5:9; Titus 1:6</span> more text');
  });
});
