#!/bin/bash
set -e

# Increment the patch version (creates commit and tag)
npm version patch

# Push the changes and the tag
git push --follow-tags

# Run the publish script
npm run pub
