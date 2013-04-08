#!/usr/bin/env bash
rm -rf LLVMBackendTutorialExampleCode.tar.gz LLVMBackendTutorialExampleCode
CURR_DIR=$(pwd)
echo ${CURR_DIR}
cd ../../lib/Target/Cpu0/ExampleCode 
sh ./genexample.sh 
tar -zcvf LLVMBackendTutorialExampleCode.tar.gz LLVMBackendTutorialExampleCode 
mv -f LLVMBackendTutorialExampleCode.tar.gz ${CURR_DIR}/. 
rm -rf LLVMBackendTutorialExampleCode
cd ${CURR_DIR}

