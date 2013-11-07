#!/usr/bin/env bash
rm -rf lbdex.tar.gz lbdex
CURR_DIR=$(pwd)
echo ${CURR_DIR}
cd source_ExampleCode 
sh ./genexample.sh 
tar -zcvf lbdex.tar.gz lbdex 
mv -f lbdex.tar.gz ${CURR_DIR}/. 
cp -rf lbdex ../.
cd ${CURR_DIR}

