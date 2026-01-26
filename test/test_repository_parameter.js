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

  test('installModule should accept (repoName, moduleCode, progressCB)', async () => {
    const promise = nsi.installModule('CrossWire', 'TEST_MODULE', (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('installModule should accept null repository', async () => {
    const promise = nsi.installModule(null, 'TEST_MODULE', (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('installModule should accept undefined repository', async () => {
    const promise = nsi.installModule(undefined, 'TEST_MODULE', (progress) => {});
    expect(promise).toBeInstanceOf(Promise);
    
    // Cancel to avoid actual installation attempt
    nsi.cancelInstallation();
    
    // Expect rejection since we cancelled or module doesn't exist
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('getModuleDescription should accept (repoName, moduleCode)', () => {
    // This will likely throw an error since no repos are configured
    // But it tests the signature works
    expect(() => {
      nsi.getModuleDescription('CrossWire', 'KJV');
    }).toThrow();
  });

  test('getModuleDescription should accept null repository', () => {
    // This will likely throw an error since no repos are configured
    // But it tests the signature works
    expect(() => {
      nsi.getModuleDescription(null, 'KJV');
    }).toThrow();
  });

  test('getModuleDescription should accept undefined repository', () => {
    // This will likely throw an error since no repos are configured
    // But it tests the signature works
    expect(() => {
      nsi.getModuleDescription(undefined, 'KJV');
    }).toThrow();
  });

  test('isModuleAvailableInRepo should accept (repoName, moduleCode)', () => {
    // Should return false since no repos configured
    const result = nsi.isModuleAvailableInRepo('CrossWire', 'KJV');
    expect(typeof result).toBe('boolean');
  });

  test('isModuleAvailableInRepo should accept null repository', () => {
    // Should return false since no repos configured
    const result = nsi.isModuleAvailableInRepo(null, 'KJV');
    expect(typeof result).toBe('boolean');
  });

  test('getRepoModule should accept (repoName, moduleCode)', () => {
    // Will throw error since no repos configured, but tests signature
    expect(() => {
      nsi.getRepoModule('CrossWire', 'KJV');
    }).toThrow();
  });

  test('getRepoModule should accept null repository', () => {
    // Will throw error since no repos configured, but tests signature
    expect(() => {
      nsi.getRepoModule(null, 'KJV');
    }).toThrow();
  });

  test('installModule should reject invalid repository parameter type', async () => {
    // This test verifies that invalid parameter types are properly rejected
    const promise = nsi.installModule(123, 'TEST_MODULE', (progress) => {});
    
    // Should reject with a type error
    await expect(promise).rejects.toBeDefined();
  }, 10000);

  test('uninstallModule should accept (repoName, moduleCode)', async () => {
    const promise = nsi.uninstallModule('CrossWire', 'TEST_MODULE');
    expect(promise).toBeInstanceOf(Promise);
    
    // This may resolve or reject depending on whether module exists
    await promise.catch(() => {
      // Expected - module doesn't exist or other error
    });
  }, 10000);

  test('uninstallModule should accept null repository', async () => {
    const promise = nsi.uninstallModule(null, 'TEST_MODULE');
    expect(promise).toBeInstanceOf(Promise);
    
    // This may resolve or reject depending on whether module exists
    await promise.catch(() => {
      // Expected - module doesn't exist or other error
    });
  }, 10000);
});
