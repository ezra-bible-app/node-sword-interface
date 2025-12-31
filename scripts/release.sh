#!/bin/bash
set -e

# Do not use any prefix for the version tag
npm config set tag-version-prefix=""

# Increment the patch version (creates commit and tag)
npm version patch

# Push the changes and the tag
git push --follow-tags

# Run the publish script
npm run pub
