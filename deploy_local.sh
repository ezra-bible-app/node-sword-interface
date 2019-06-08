#!/bin/sh

EZRA_PROJECT_DIR="ezra-project"
NODE_SWORD_INTERFACE_DIR="node-sword-interface"

cd ..
rm -rf ${EZRA_PROJECT_DIR}/node_modules/node-sword-interface
cp -a ${NODE_SWORD_INTERFACE_DIR} ${EZRA_PROJECT_DIR}/node_modules/node-sword-interface
cd ${EZRA_PROJECT_DIR}
npm run rebuild-linux
