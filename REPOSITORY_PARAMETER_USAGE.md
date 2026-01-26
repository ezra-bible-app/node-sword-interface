# Module Name Collision Fix - Usage Examples

This document explains the changes made to address module name collision issues and how to use the new repository-aware API.

## Problem Statement

Previously, module names/IDs were assumed to be unique across all repositories. However, this assumption was incorrect - multiple repositories can have modules with the same name. For example:
- Official NET module in bible.org repository vs unofficial NET in Xiphos repository
- Official BSB in CrossWire repository vs unofficial engbsb2020eb in eBible repository (identified by Abbreviation=BSB)

## Solution

The API now **requires** repository names to be provided as the first parameter for module operations. This ensures explicit targeting of specific repositories to avoid ambiguity.

## API Changes

### Updated Methods (repository name is now **required** as the first parameter)

1. **installModule(repositoryName, moduleCode, progressCB)**
2. **uninstallModule(repositoryName, moduleCode)**
3. **getModuleDescription(repositoryName, moduleCode)**
4. **isModuleAvailableInRepo(repositoryName, moduleCode)**
5. **getRepoModule(repositoryName, moduleCode)**

**Note:** This is a breaking change. The repository parameter is **required** and must be a string.

## Usage Examples

### Installing a Module from a Specific Repository

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Install module from specific repository (required)
await nsi.installModule('Bible.org', 'NET', (progress) => {
  console.log(`Progress: ${progress}%`);
});

// Install from CrossWire repository
await nsi.installModule('CrossWire', 'KJV', (progress) => {
  console.log(`Progress: ${progress}%`);
});
```

### Getting Module Information

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Get module description from specific repository
const description = nsi.getModuleDescription('CrossWire', 'KJV');

// Get module object from specific repository
const module = nsi.getRepoModule('Bible.org', 'NET');

// Check if module available in specific repository
const available = nsi.isModuleAvailableInRepo('Bible.org', 'NET');
```

### Uninstalling Modules

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// Uninstall module from specific repository
await nsi.uninstallModule('CrossWire', 'KJV');
```

### Handling Module Name Collisions

```javascript
const NodeSwordInterface = require('node-sword-interface');
const nsi = new NodeSwordInterface();

// List available repositories
const repos = nsi.getRepoNames();
console.log('Available repositories:', repos);

// For each repository, check if module exists
for (const repo of repos) {
  const available = nsi.isModuleAvailableInRepo(repo, 'NET');
  if (available) {
    const module = nsi.getRepoModule(repo, 'NET');
    console.log(`Found NET in ${repo}:`, module.description);
  }
}

// Install from the specific repository you want
await nsi.installModule('Bible.org', 'NET', (progress) => {
  console.log(`Installing official NET from Bible.org: ${progress}%`);
});
```

### Finding Module Repository

If you don't know which repository contains a module, you can search:

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

// Install the one you want
const selectedRepo = 'Bible.org'; // User's choice
await nsi.installModule(selectedRepo, 'NET', (progress) => {
  console.log(`Installing from ${selectedRepo}: ${progress}%`);
});
```

## Migration Guide

### Breaking Changes

**The repository parameter is now required!** You must provide a repository name for all module operations.

### Before (Old API - No Longer Works)

```javascript
// Old API - NO LONGER WORKS
await nsi.installModule('KJV', progressCallback);
const desc = nsi.getModuleDescription('KJV');
await nsi.uninstallModule('KJV');
```

### After (New API - Required)

```javascript
// New API - repository name is required
await nsi.installModule('CrossWire', 'KJV', progressCallback);
const desc = nsi.getModuleDescription('CrossWire', 'KJV');
await nsi.uninstallModule('CrossWire', 'KJV');
```

## Implementation Details

### JavaScript API Layer

The JavaScript wrapper in `index.js`:
- Repository name is now the **required** first parameter in all module-related functions
- No backward compatibility - old API signatures will not work

### C++ Native Layer

The C++ implementation:
- Uses `INIT_SCOPE_AND_VALIDATE` macro for parameter validation
- Accepts 4 parameters for `installModule`: (repoName, moduleCode, progressCB, callback)
- Accepts 3 parameters for `uninstallModule`: (repoName, moduleCode, callback)
- Accepts 2 parameters for other functions: (repoName, moduleCode)
- All parameters are validated and required

## Best Practices

### 1. Always Specify Repository

```javascript
// Good - explicit repository
await nsi.installModule('Bible.org', 'NET', progressCallback);

// Bad - will not work (no default repository)
await nsi.installModule('NET', progressCallback); // ERROR!
```

### 2. List Available Repositories First

```javascript
// Get official repository names
const repos = nsi.getRepoNames();
// repos might include: ['CrossWire', 'Bible.org', 'Xiphos', 'eBible']

// Use these names when calling module functions
const module = nsi.getRepoModule(repos[0], 'KJV');
```

### 3. Handle Module Name Collisions Explicitly

```javascript
// Check each repository for the module
const repos = nsi.getRepoNames();
for (const repo of repos) {
  if (nsi.isModuleAvailableInRepo(repo, 'NET')) {
    console.log(`NET found in ${repo}`);
    const module = nsi.getRepoModule(repo, 'NET');
    console.log(`  Description: ${module.description}`);
  }
}
```

## Testing

The build succeeds and the code now properly requires repository parameters for all module operations, preventing any ambiguity about which repository's module is being accessed.

