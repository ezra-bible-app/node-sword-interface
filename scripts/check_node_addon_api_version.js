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

const fs = require('fs');
const path = require('path');

const expectedVersion = '7.1.1';
const repoRoot = path.resolve(__dirname, '..');
const packageJsonPath = path.join(repoRoot, 'package.json');
const installedPackageJsonPath = path.join(repoRoot, 'node_modules', 'node-addon-api', 'package.json');

function readJson(filePath) {
  return JSON.parse(fs.readFileSync(filePath, 'utf8'));
}

function fail(message) {
  console.error(message);
  process.exit(1);
}

const packageJson = readJson(packageJsonPath);
const declaredVersion = packageJson.dependencies && packageJson.dependencies['node-addon-api'];

if (declaredVersion !== expectedVersion) {
  fail(
    `node-addon-api must stay pinned to ${expectedVersion} while node-sword-interface builds with C++11. ` +
    `Found: ${declaredVersion || 'missing'}.`
  );
}

if (fs.existsSync(installedPackageJsonPath)) {
  const installedPackageJson = readJson(installedPackageJsonPath);
  const installedVersion = installedPackageJson.version;

  if (typeof installedVersion !== 'string') {
    fail('Installed node-addon-api package metadata is missing a valid version string.');
  }

  if (!/^\d+\.\d+\.\d+([-.].+)?$/.test(installedVersion)) {
    fail(`Installed node-addon-api version '${installedVersion}' is not a supported semver string.`);
  }

  const installedMajor = Number.parseInt(installedVersion.split('.')[0], 10);

  if (Number.isNaN(installedMajor) || installedMajor > 7) {
    fail(
      `Installed node-addon-api version ${installedVersion} is not compatible with the C++11 build.`
    );
  }
}

console.log(`node-addon-api version check passed (${expectedVersion}).`);
