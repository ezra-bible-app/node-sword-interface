# Module Name Collision Fix - Usage Examples

This document explains the changes made to address module name collision issues and how to use the new repository-aware API.

## Problem Statement

Previously, module names/IDs were assumed to be unique across all repositories. However, this assumption was incorrect - multiple repositories can have modules with the same name. For example:
- Official NET module in bible.org repository vs unofficial NET in Xiphos repository
- Official BSB in CrossWire repository vs unofficial engbsb2020eb in eBible repository (identified by Abbreviation=BSB)

## Solution

We've added optional `repositoryName` parameters to key API methods while maintaining full backward compatibility. When a repository name is provided, operations are performed on the specific repository. When it's undefined/null, the system uses the original behavior of searching across all repositories.

## API Changes

### Updated Methods

1. **installModule(moduleCode, repositoryName, progressCB)**
2. **uninstallModule(moduleCode, repositoryName)**
3. **getModuleDescription(moduleCode, repositoryName)**
4. **isModuleAvailableInRepo(moduleCode, repositoryName)**
5. **getRepoModule(moduleCode, repositoryName)**

## Usage Examples

### Old API (Still Works - Backward Compatible)

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Install module - repository auto-detected
await nsi.installModule('NET', (progress) => {
  console.log(`Progress: ${progress}%`);
});

// Get module description - searches all repositories
const description = nsi.getModuleDescription('NET');

// Check if module available - searches all repositories
const available = nsi.isModuleAvailableInRepo('NET');

// Uninstall module
await nsi.uninstallModule('NET');
```

### New API (Repository-Aware)

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Install module from specific repository
await nsi.installModule('NET', 'Bible.org', (progress) => {
  console.log(`Progress: ${progress}%`);
});

// Get module description from specific repository
const description = nsi.getModuleDescription('NET', 'Bible.org');

// Check if module available in specific repository
const available = nsi.isModuleAvailableInRepo('NET', 'Bible.org');

// Get module object from specific repository
const module = nsi.getRepoModule('NET', 'Bible.org');

// Uninstall module from specific repository
await nsi.uninstallModule('NET', 'Bible.org');
```

### Using null/undefined for Default Behavior

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Explicitly use undefined to search all repositories
const description1 = nsi.getModuleDescription('NET', undefined);

// Use null to search all repositories
const description2 = nsi.getModuleDescription('NET', null);

// Both are equivalent to the old API:
const description3 = nsi.getModuleDescription('NET');
```

## Handling Module Name Collisions

### Best Practice: Always Specify Repository

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// List available repositories
const repos = nsi.getRepoNames();
console.log('Available repositories:', repos);

// For each repository, check if module exists
for (const repo of repos) {
  const available = nsi.isModuleAvailableInRepo('NET', repo);
  if (available) {
    const module = nsi.getRepoModule('NET', repo);
    console.log(`Found NET in ${repo}:`, module.description);
  }
}

// Install from the specific repository you want
await nsi.installModule('NET', 'Bible.org', (progress) => {
  console.log(`Installing official NET from Bible.org: ${progress}%`);
});
```

### Resolving Conflicts

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Find all repositories containing a module
function findModuleInRepositories(moduleCode) {
  const repos = nsi.getRepoNames();
  const found = [];
  
  for (const repo of repos) {
    if (nsi.isModuleAvailableInRepo(moduleCode, repo)) {
      const module = nsi.getRepoModule(moduleCode, repo);
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

// Let user choose which one to install
const selectedRepo = 'Bible.org'; // User's choice
await nsi.installModule('NET', selectedRepo, (progress) => {
  console.log(`Installing from ${selectedRepo}: ${progress}%`);
});
```

## Migration Guide

### For Existing Code

Your existing code will continue to work without any changes:

```javascript
// This still works exactly as before
await nsi.installModule('KJV', (progress) => {
  console.log(`Progress: ${progress}%`);
});
```

### Recommended Updates

For applications where module name collisions could occur, update to use the repository parameter:

```javascript
// Before (could have collisions)
await nsi.installModule('NET', progressCallback);

// After (explicit repository)
await nsi.installModule('NET', 'Bible.org', progressCallback);
```

## Implementation Details

### JavaScript API Layer

The JavaScript wrapper in `index.js` handles backward compatibility by:
1. Detecting if the second parameter is a function (old API) or string/null/undefined (new API)
2. Adjusting parameters accordingly before calling the native interface

### C++ Native Layer

The C++ implementation:
1. Accepts both 3-parameter (old) and 4-parameter (new) signatures for `installModule`
2. Accepts both 2-parameter (old) and 3-parameter (new) signatures for `uninstallModule`
3. Validates parameter types and throws appropriate errors for invalid input
4. Uses the existing repository-specific installation methods when repository is provided
5. Falls back to auto-discovery when repository is null/undefined

## Testing

A comprehensive test suite (`test/test_repository_parameter.js`) verifies:
- Old API still works (backward compatibility)
- New API with repository parameter works
- Null/undefined repository parameters work correctly
- Invalid parameter types are rejected with appropriate errors

All 13 tests pass, confirming the implementation is correct and backward compatible.
