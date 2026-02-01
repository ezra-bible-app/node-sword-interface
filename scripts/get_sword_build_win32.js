#!/usr/bin/env node
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

const https = require('https');
const fs = require('fs');
const path = require('path');
const { execFileSync } = require('child_process');

const RELEASE_TAG = 'v1.8.900-2022-11-06';
const API_URL = `https://api.github.com/repos/ezra-bible-app/sword-build-win32/releases/tags/${RELEASE_TAG}`;

function fetchJson(url, headers = {}) {
  return new Promise((resolve, reject) => {
    const req = https.get(url, {
      headers: {
        'User-Agent': 'node-sword-interface',
        'Accept': 'application/vnd.github+json',
        ...headers,
      }
    }, res => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => {
        if (res.statusCode && res.statusCode >= 200 && res.statusCode < 300) {
          try {
            resolve(JSON.parse(data));
          } catch (e) {
            reject(e);
          }
        } else {
          reject(new Error(`Request failed: ${res.statusCode} ${data}`));
        }
      });
    });
    req.on('error', reject);
  });
}

function downloadFile(url, outPath, headers = {}, maxRedirects = 5) {
  return new Promise((resolve, reject) => {
    let redirects = 0;

    const doRequest = (reqUrl) => {
      const file = fs.createWriteStream(outPath);
      const req = https.get(reqUrl, {
        headers: {
          'User-Agent': 'node-sword-interface',
          'Accept': 'application/octet-stream',
          ...headers,
        }
      }, res => {
        // Handle redirects
        if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
          file.close(); // ensure stream closed before re-request
          fs.unlink(outPath, () => {
            if (redirects >= maxRedirects) {
              reject(new Error(`Too many redirects downloading: ${reqUrl}`));
              return;
            }
            redirects++;
            const nextUrl = res.headers.location.startsWith('http')
              ? res.headers.location
              : new URL(res.headers.location, reqUrl).toString();
            doRequest(nextUrl);
          });
          return;
        }

        if (res.statusCode && (res.statusCode < 200 || res.statusCode >= 300)) {
          file.close();
          fs.unlink(outPath, () => reject(new Error(`Download failed: ${res.statusCode}`)));
          return;
        }

        res.pipe(file);
        file.on('finish', () => file.close(resolve));
      });

      req.on('error', err => {
        file.close();
        fs.unlink(outPath, () => reject(err));
      });
    };

    doRequest(url);
  });
}

async function main() {
  try {
    const isCI = process.env.CI === 'true';
    const githubToken = process.env.GITHUB_TOKEN;
    const authHeader = (isCI && githubToken) ? { Authorization: `Bearer ${githubToken}` } : {};

    if (isCI) {
      console.log('GitHub actions build ... using GITHUB_TOKEN for authentication!');
    }

    const release = await fetchJson(API_URL, authHeader);
    if (!release.assets || !release.assets.length) {
      throw new Error('No assets found in release.');
    }

    const asset = release.assets[0];
    const zipName = asset.name;
    const zipUrl = asset.browser_download_url;

    const cwd = process.cwd();

    // Remove existing folder if present
    const existingDir = path.join(cwd, 'sword-build-win32');
    if (fs.existsSync(existingDir)) {
      fs.rmSync(existingDir, { recursive: true, force: true });
    }

    const zipPath = path.join(cwd, zipName);
    console.log(`Downloading ${zipUrl} -> ${zipPath}`);
    await downloadFile(zipUrl, zipPath, authHeader);

    // Extract ZIP file
    const extractedDirName = zipName.replace(/\.zip$/i, '');
    try {
      if (process.platform === 'win32') {
        execFileSync('powershell', ['-NoProfile', '-Command', `Expand-Archive -Path '${zipPath}' -DestinationPath '.' -Force`], { stdio: 'inherit' });
      } else {
        // Prefer system unzip if available
        try {
          execFileSync('unzip', ['-o', zipPath], { stdio: 'inherit' });
        } catch (unzipErr) {
          // Fallback to 7z if unzip is not present
          execFileSync('7z', ['x', zipPath], { stdio: 'inherit' });
        }
      }
    } catch (e) {
      throw new Error(`Extraction failed: ${e.message}`);
    }

    // Remove zip file
    fs.unlinkSync(zipPath);

    // Rename extracted folder
    if (!fs.existsSync(path.join(cwd, extractedDirName))) {
      throw new Error(`Expected extracted directory '${extractedDirName}' not found.`);
    }
    fs.renameSync(path.join(cwd, extractedDirName), existingDir);

    console.log('Download of Windows library artifacts completed!');
  } catch (err) {
    console.error('Error:', err.message);
    process.exit(1);
  }
}

if (require.main === module) {
  main();
}
