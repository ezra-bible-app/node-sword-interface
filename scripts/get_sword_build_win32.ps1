#   This file is part of node-sword-interface.
#
#   Copyright (C) 2019 - 2020 Tobias Klein <contact@ezra-project.net>
#
#   node-sword-interface is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 2 of the License, or
#   (at your option) any later version.
#
#   node-sword-interface is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of 
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#   See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with node-sword-interface. See the file COPYING.
#   If not, see <http://www.gnu.org/licenses/>.

# see https://www.helloitscraig.co.uk/2016/02/download-the-latest-repo.html
# see https://www.saotn.org/unzip-file-powershell/

Add-Type -AssemblyName System.IO.Compression.FileSystem
function unzip {
	param( [string]$ziparchive, [string]$extractpath )
	[System.IO.Compression.ZipFile]::ExtractToDirectory( $ziparchive, $extractpath )
}

# --- Only allow TLS1.1 and TLS1.2
$AllProtocols = [System.Net.SecurityProtocolType]'Tls11,Tls12'
[System.Net.ServicePointManager]::SecurityProtocol = $AllProtocols

# --- Set the uri for the release
$URI = "https://api.github.com/repos/tobias-klein/sword-build-win32/releases/tags/v1.8.900-2021-01-24"

# --- Query the API to get the url of the zip
$Response = Invoke-RestMethod -Method Get -Uri $URI
$ZipName = $Response.assets[0].name
$ZipUrl = $Response.assets[0].browser_download_url

# --- Remove the existing folder
if (Test-Path sword-build-win32) {
		Remove-Item -Recurse -Force sword-build-win32
}

# --- Download the file to the current location
$OutputPath = "$((Get-Location).Path)\$ZipName"
Invoke-RestMethod -Method Get -Uri $ZipUrl -OutFile $OutputPath

# --- Unzip the zip file and remove it afterwards
unzip $OutputPath $((Get-Location).Path)
Remove-Item $OutputPath

# --- Rename the extracted folder to a standard name
$LibDirName = $ZipName.Replace(".zip", "")
Rename-Item -Path $LibDirName -NewName sword-build-win32