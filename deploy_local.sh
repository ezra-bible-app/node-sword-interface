#!/bin/bash

PROJECT_DIR='ezra-project-git'
WORK_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
rm -rf $WORK_DIR/../$PROJECT_DIR/node_modules/node-sword-interface
cp -a $WORK_DIR $WORK_DIR/../$PROJECT_DIR/node_modules/node-sword-interface
RESULT="$( cd $WORK_DIR/../${PROJECT_DIR}/ && npm run rebuild-nsi )"
