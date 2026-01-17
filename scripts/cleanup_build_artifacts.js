#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const stampFile = process.argv[2];

// Directories to remove
const dirsToRemove = ['sword', 'sword_build'];

for (const dir of dirsToRemove) {
    const dirPath = path.resolve(__dirname, '..', dir);
    if (fs.existsSync(dirPath)) {
        fs.rmSync(dirPath, { recursive: true, force: true });
        console.log(`Removed: ${dirPath}`);
    }
}

// Create the stamp file to mark completion
if (stampFile) {
    fs.writeFileSync(stampFile, new Date().toISOString());
    console.log(`Created stamp file: ${stampFile}`);
}

console.log('Cleanup completed');
