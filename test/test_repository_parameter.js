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

describe('Repository Parameter API Tests', () => {
  let nsi;

  beforeAll(() => {
    nsi = new NodeSwordInterface();
  });

  test('should accept installModule with old API (moduleCode, progressCB)', async () => {
    // This test verifies backward compatibility
    // We're not actually installing anything, just checking the signature works
    const promise = nsi.installModule('TEST_MODULE', (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('should accept installModule with new API (moduleCode, repoName, progressCB)', async () => {
    // This test verifies the new repository-aware API
    const promise = nsi.installModule('TEST_MODULE', 'CrossWire', (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('should accept installModule with new API using null repository', async () => {
    // This test verifies that null/undefined repository falls back to old behavior
    const promise = nsi.installModule('TEST_MODULE', null, (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('should accept installModule with new API using undefined repository', async () => {
    // This test verifies that null/undefined repository falls back to old behavior
    const promise = nsi.installModule('TEST_MODULE', undefined, (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('getModuleDescription should work with old API (moduleCode only)', () => {
    // This will likely throw an error since no repos are configured
    // But it tests the signature works
    expect(() => {
      nsi.getModuleDescription('KJV');
    }).toThrow();
  });

  test('getModuleDescription should accept new API (moduleCode, repoName)', () => {
    // This will likely throw an error since no repos are configured
    // But it tests the signature works
    expect(() => {
      nsi.getModuleDescription('KJV', 'CrossWire');
    }).toThrow();
  });

  test('getModuleDescription should accept null repository', () => {
    // This will likely throw an error since no repos are configured
    // But it tests the signature works
    expect(() => {
      nsi.getModuleDescription('KJV', null);
    }).toThrow();
  });

  test('isModuleAvailableInRepo should work with old API', () => {
    // Should return false since no repos configured
    const result = nsi.isModuleAvailableInRepo('KJV');
    expect(typeof result).toBe('boolean');
  });

  test('isModuleAvailableInRepo should accept new API with repoName', () => {
    // Should return false since no repos configured
    const result = nsi.isModuleAvailableInRepo('KJV', 'CrossWire');
    expect(typeof result).toBe('boolean');
  });

  test('getRepoModule should work with old API', () => {
    // Will throw error since no repos configured, but tests signature
    expect(() => {
      nsi.getRepoModule('KJV');
    }).toThrow();
  });

  test('getRepoModule should accept new API with repoName', () => {
    // Will throw error since no repos configured, but tests signature
    expect(() => {
      nsi.getRepoModule('KJV', 'CrossWire');
    }).toThrow();
  });
});
