# Module Name Collision Fix - Usage Examples

This document explains the changes made to address module name collision issues and how to use the new repository-aware API.

## Problem Statement

Previously, module names/IDs were assumed to be unique across all repositories. However, this assumption was incorrect - multiple repositories can have modules with the same name. For example:
- Official NET module in bible.org repository vs unofficial NET in Xiphos repository
- Official BSB in CrossWire repository vs unofficial engbsb2020eb in eBible repository (identified by Abbreviation=BSB)

## Solution

The API now requires repository names to be provided as the first parameter for module operations. When a repository name is `undefined` or `null`, the system searches across all repositories (auto-discovery).

## API Changes

### Updated Methods (repository name is now the first parameter)

1. **installModule(repositoryName, moduleCode, progressCB)**
2. **uninstallModule(repositoryName, moduleCode)**
3. **getModuleDescription(repositoryName, moduleCode)**
4. **isModuleAvailableInRepo(repositoryName, moduleCode)**
5. **getRepoModule(repositoryName, moduleCode)**

**Note:** This is a breaking change. The parameter order has been changed to put `repositoryName` first.

## Usage Examples

### Installing a Module from a Specific Repository

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Install module from specific repository
await nsi.installModule('Bible.org', 'NET', (progress) => {
  console.log(`Progress: ${progress}%`);
});
```

### Auto-Discovery (Search All Repositories)

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Install module - repository auto-discovered (pass undefined or null)
await nsi.installModule(undefined, 'KJV', (progress) => {
  console.log(`Progress: ${progress}%`);
});

// Or explicitly use null
await nsi.installModule(null, 'KJV', (progress) => {
  console.log(`Progress: ${progress}%`);
});
```

### Getting Module Information

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Get module description from specific repository
const description = nsi.getModuleDescription('Bible.org', 'NET');

// Get module description from any repository (auto-discovery)
const description2 = nsi.getModuleDescription(null, 'NET');

// Check if module available in specific repository
const available = nsi.isModuleAvailableInRepo('Bible.org', 'NET');

// Get module object from specific repository
const module = nsi.getRepoModule('Bible.org', 'NET');
```

### Handling Module Name Collisions

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Find all repositories containing a module
function findModuleInRepositories(moduleCode) {
  const repos = nsi.getRepoNames();
  const found = [];
  
  for (const repo of repos) {
    if (nsi.isModuleAvailableInRepo(repo, moduleCode)) {
      const module = nsi.getRepoModule(repo, moduleCode);
      found.push({
        repository: repo,
        module: module
      });
    }
  }
  
  return found;
}

// Usage
const netModules = findModuleInRepositories('NET');
console.log(`Found ${netModules.length} versions of NET`);

netModules.forEach(item => {
  console.log(`  - ${item.repository}: ${item.module.description}`);
});

// Install from specific repository (recommended)
await nsi.installModule('Bible.org', 'NET', (progress) => {
  console.log(`Installing from Bible.org: ${progress}%`);
});
```

### Uninstalling Modules

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Uninstall module from specific repository
await nsi.uninstallModule('Bible.org', 'NET');

// Uninstall module from any repository (auto-discovery)
await nsi.uninstallModule(null, 'NET');
```

## Migration Guide

### Breaking Changes

**Parameter order has changed!** The repository name is now the first parameter.

### Before (Old API - No Longer Supported)

```javascript
// Old API - NO LONGER WORKS
await nsi.installModule('KJV', progressCallback);
const desc = nsi.getModuleDescription('KJV');
await nsi.uninstallModule('KJV');
```

### After (New API)

```javascript
// New API - repository name comes first
await nsi.installModule(null, 'KJV', progressCallback);  // null = auto-discover
const desc = nsi.getModuleDescription(null, 'KJV');
await nsi.uninstallModule(null, 'KJV');

// Or specify repository explicitly (recommended)
await nsi.installModule('CrossWire', 'KJV', progressCallback);
const desc = nsi.getModuleDescription('CrossWire', 'KJV');
await nsi.uninstallModule('CrossWire', 'KJV');
```

## Best Practices

### 1. Always Specify Repository When Possible

```javascript
// Good - explicit repository
await nsi.installModule('Bible.org', 'NET', progressCallback);

// Less ideal - auto-discovery (may be ambiguous)
await nsi.installModule(null, 'NET', progressCallback);
```

### 2. Handle Module Name Collisions Explicitly

```javascript
// List all repositories
const repos = nsi.getRepoNames();
console.log('Available repositories:', repos);

// Check each repository for the module
for (const repo of repos) {
  if (nsi.isModuleAvailableInRepo(repo, 'NET')) {
    console.log(`NET found in ${repo}`);
    const module = nsi.getRepoModule(repo, 'NET');
    console.log(`  Description: ${module.description}`);
  }
}
```

### 3. Use Repository Names from API

```javascript
// Get official repository names
const repos = nsi.getRepoNames();
// repos might include: ['CrossWire', 'Bible.org', 'Xiphos', 'eBible']

// Use these names when calling module functions
const module = nsi.getRepoModule(repos[0], 'KJV');
```

## Implementation Details

### JavaScript API Layer

The JavaScript wrapper in `index.js`:
- Repository name is now the first parameter in all module-related functions
- When repository is `undefined` or `null`, auto-discovery behavior is used
- No backward compatibility - old API signatures are no longer supported

### C++ Native Layer

The C++ implementation:
- Accepts 4 parameters for `installModule`: (repoName, moduleCode, progressCB, callback)
- Accepts 3 parameters for `uninstallModule`: (repoName, moduleCode, callback)
- Accepts 2 parameters for other functions: (repoName, moduleCode)
- Validates parameter types and throws appropriate errors for invalid input
- Uses repository-specific methods when repository is provided
- Falls back to auto-discovery when repository is null/undefined

## Testing

A comprehensive test suite (`test/test_repository_parameter.js`) verifies:
- API accepts repository as first parameter
- Null/undefined repository parameters work correctly (auto-discovery)
- Invalid parameter types are rejected with appropriate errors

All 13 tests pass, confirming the implementation is correct.
