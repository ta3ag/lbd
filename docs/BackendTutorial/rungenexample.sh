#!/usr/bin/env bash
rm -rf LLVMBackendTutorialExampleCode.tar.gz LLVMBackendTutorialExampleCode
CURR_DIR=$(pwd)
echo ${CURR_DIR}
cd source_ExampleCode 
sh ./genexample.sh 
tar -zcvf LLVMBackendTutorialExampleCode.tar.gz LLVMBackendTutorialExampleCode 
mv -f LLVMBackendTutorialExampleCode.tar.gz ${CURR_DIR}/. 
rm -rf ../../../lib/Target/Cpu0/LLVMBackendTutorialExampleCode
cp -rf LLVMBackendTutorialExampleCode ../.
mv -f LLVMBackendTutorialExampleCode ../../../lib/Target/Cpu0/.
cd ${CURR_DIR}

