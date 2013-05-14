#!/usr/bin/env bash
GEN_DIR=LLVMBackendTutorialExampleCode
rm -rf Chapter3_1 Chapter3_2 Chapter3_2 Chapter3_4 Chapter3_5 
rm -rf Chapter4_1 Chapter4_2 Chapter4_4 Chapter4_4_2 Chapter4_5 Chapter4_6_1 Chapter4_6_2 Chapter4_6_4 
rm -rf Chapter5_1 Chapter6_1 Chapter6_2 Chapter6_3 Chapter7_1 
rm -rf Chapter8_2 Chapter8_3 Chapter8_4 Chapter8_5 Chapter8_6 Chapter8_7 Chapter8_8 Chapter8_8_2 Chapter8_9 Chapter8_10
rm -rf Chapter9_1 Chapter10_1 Chapter10_2 Chapter11_1 Chapter11_2
rm -rf ${GEN_DIR}
mkdir ${GEN_DIR}
cp -rf Chapter2 ${GEN_DIR}/.
patch -fp0 < 3-1.patch
mv Chapter2 Chapter3_1
cp -rf Chapter3_1 ${GEN_DIR}/.
patch -fp0 < 3-2.patch
mv Chapter3_1 Chapter3_2
cp -rf Chapter3_2 ${GEN_DIR}/.
patch -fp0 < 3-3.patch
mv Chapter3_2 Chapter3_3
cp -rf Chapter3_3 ${GEN_DIR}/.
patch -fp0 < 3-4.patch
mv Chapter3_3 Chapter3_4
cp -rf Chapter3_4 ${GEN_DIR}/.
patch -fp0 < 3-5.patch
mv Chapter3_4 Chapter3_5
cp -rf Chapter3_5 ${GEN_DIR}/.
patch -fp0 < 4-1.patch
mv Chapter3_5 Chapter4_1
cp -rf Chapter4_1 ${GEN_DIR}/.
patch -fp0 < 4-2.patch
mv Chapter4_1 Chapter4_2
cp -rf Chapter4_2 ${GEN_DIR}/.
patch -fp0 < 4-4.patch
mv Chapter4_2 Chapter4_4
cp -rf Chapter4_4 ${GEN_DIR}/.
patch -fp0 < 4-4_2.patch
mv Chapter4_4 Chapter4_4_2
cp -rf Chapter4_4_2 ${GEN_DIR}/.
patch -fp0 < 4-5.patch
mv Chapter4_4_2 Chapter4_5
cp -rf Chapter4_5 ${GEN_DIR}/.
patch -fp0 < 4-6_1.patch
mv Chapter4_5 Chapter4_6_1
cp -rf Chapter4_6_1 ${GEN_DIR}/.
patch -fp0 < 4-6_2.patch
mv Chapter4_6_1 Chapter4_6_2
cp -rf Chapter4_6_2 ${GEN_DIR}/.
patch -fp0 < 4-6_4.patch
mv Chapter4_6_2 Chapter4_6_4
cp -rf Chapter4_6_4 ${GEN_DIR}/.
patch -fp0 < 5-1.patch
mv Chapter4_6_4 Chapter5_1
cp -rf Chapter5_1 ${GEN_DIR}/.
patch -fp0 < 6-1.patch
mv Chapter5_1 Chapter6_1
cp -rf Chapter6_1 ${GEN_DIR}/.
patch -fp0 < 6-2.patch
mv Chapter6_1 Chapter6_2
cp -rf Chapter6_2 ${GEN_DIR}/.
patch -fp0 < 6-3.patch
mv Chapter6_2 Chapter6_3
cp -rf Chapter6_3 ${GEN_DIR}/.
patch -fp0 < 7-1.patch
mv Chapter6_3 Chapter7_1
cp -rf Chapter7_1 ${GEN_DIR}/.
patch -fp0 < 8-2.patch
mv Chapter7_1 Chapter8_2
cp -rf Chapter8_2 ${GEN_DIR}/.
patch -fp0 < 8-3.patch
mv Chapter8_2 Chapter8_3
cp -rf Chapter8_3 ${GEN_DIR}/.
patch -fp0 < 8-4.patch
mv Chapter8_3 Chapter8_4
cp -rf Chapter8_4 ${GEN_DIR}/.
patch -fp0 < 8-5.patch
mv Chapter8_4 Chapter8_5
cp -rf Chapter8_5 ${GEN_DIR}/.
patch -fp0 < 8-6.patch
mv Chapter8_5 Chapter8_6
cp -rf Chapter8_6 ${GEN_DIR}/.
patch -fp0 < 8-7.patch
mv Chapter8_6 Chapter8_7
cp -rf Chapter8_7 ${GEN_DIR}/.
patch -fp0 < 8-8.patch
mv Chapter8_7 Chapter8_8
cp -rf Chapter8_8 ${GEN_DIR}/.
patch -fp0 < 8-8_2.patch
mv Chapter8_8 Chapter8_8_2
cp -rf Chapter8_8_2 ${GEN_DIR}/.
rm -rf Chapter8_8_2
cp -rf ${GEN_DIR}/Chapter8_8 .
patch -fp0 < 8-9.patch
mv Chapter8_8 Chapter8_9
cp -rf Chapter8_9 ${GEN_DIR}/.
patch -fp0 < 8-10.patch
mv Chapter8_9 Chapter8_10
cp -rf Chapter8_10 ${GEN_DIR}/.
patch -fp0 < 9-1.patch
mv Chapter8_10 Chapter9_1
cp -rf Chapter9_1 ${GEN_DIR}/.
patch -fp0 < 10-1.patch
mv Chapter9_1 Chapter10_1
cp -rf Chapter10_1 ${GEN_DIR}/.
patch -fp0 < 10-2.patch
mv Chapter10_1 Chapter10_2
cp -rf Chapter10_2 ${GEN_DIR}/.
patch -fp0 < 11-1.patch
mv Chapter10_2 Chapter11_1
cp -rf Chapter11_1 ${GEN_DIR}/.
patch -fp0 < 11-2.patch
mv Chapter11_1 Chapter11_2
cp -rf Chapter11_2 ${GEN_DIR}/.
rm -rf Chapter11_2
cp -rf ${GEN_DIR}/Chapter2 .
cp -rf InputFiles llvm3.1 llvm3.2 ${GEN_DIR}/.
cp -rf 3.1_src_files_modify 3.2_src_files_modify src_files_modify ${GEN_DIR}/.
cp -rf cpu0_verilog ${GEN_DIR}/.
cp -rf removecpu0.sh readme ${GEN_DIR}/.
rm -f `find . -name .DS_Store`
