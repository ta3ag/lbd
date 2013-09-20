#!/usr/bin/env bash
GEN_DIR=LLVMBackendTutorialExampleCode
rm -rf Chapter3_1 Chapter3_2 Chapter3_2 Chapter3_4 
rm -rf Chapter4_1 Chapter4_2 Chapter4_4 Chapter4_4_2 Chapter4_5 Chapter4_6_1 Chapter4_6_2 Chapter4_6_4 
rm -rf Chapter5_1 Chapter6_1 Chapter6_3 Chapter7_1 
rm -rf Chapter8_1 Chapter8_2 Chapter8_3 Chapter8_4 Chapter8_4_2
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
patch -fp0 < 4-1.patch
mv Chapter3_4 Chapter4_1
cp -rf Chapter4_1 ${GEN_DIR}/.
patch -fp0 < 4-2.patch
mv Chapter4_1 Chapter4_2
cp -rf Chapter4_2 ${GEN_DIR}/.
patch -fp0 < 5-1.patch
mv Chapter4_2 Chapter5_1
cp -rf Chapter5_1 ${GEN_DIR}/.
patch -fp0 < 6-1.patch
mv Chapter5_1 Chapter6_1
cp -rf Chapter6_1 ${GEN_DIR}/.
patch -fp0 < 7-1.patch
mv Chapter6_1 Chapter7_1
cp -rf Chapter7_1 ${GEN_DIR}/.
patch -fp0 < 8-1.patch
mv Chapter7_1 Chapter8_1
cp -rf Chapter8_1 ${GEN_DIR}/.
patch -fp0 < 9-1.patch
mv Chapter8_1 Chapter9_1
cp -rf Chapter9_1 ${GEN_DIR}/.
patch -fp0 < 9-2.patch
mv Chapter9_1 Chapter9_2
cp -rf Chapter9_2 ${GEN_DIR}/.
patch -fp0 < 9-3.patch
mv Chapter9_2 Chapter9_3
cp -rf Chapter9_3 ${GEN_DIR}/.
patch -fp0 < 9-4.patch
mv Chapter9_3 Chapter9_4
cp -rf Chapter9_4 ${GEN_DIR}/.
patch -fp0 < 9-4-2.patch
mv Chapter9_4 Chapter9_4_2
cp -rf Chapter9_4_2 ${GEN_DIR}/.
rm -rf Chapter9_4_2
cp -rf ${GEN_DIR}/Chapter9_4 .
patch -fp0 < 10-1.patch
mv Chapter9_4 Chapter10_1
cp -rf Chapter10_1 ${GEN_DIR}/.
patch -fp0 < 11-1.patch
mv Chapter10_1 Chapter11_1
cp -rf Chapter11_1 ${GEN_DIR}/.
patch -fp0 < 11-2.patch
mv Chapter11_1 Chapter11_2
cp -rf Chapter11_2 ${GEN_DIR}/.
patch -fp0 < 12-1.patch
mv Chapter11_2 Chapter12_1
cp -rf Chapter12_1 ${GEN_DIR}/.
patch -fp0 < 12-2.patch
mv Chapter12_1 Chapter12_2
cp -rf Chapter12_2 ${GEN_DIR}/.
rm -rf Chapter12_2
cp -rf ${GEN_DIR}/Chapter2 .
cp -rf InputFiles src_files_modify ${GEN_DIR}/.
cp -rf 3.4_20130816_Chapter11_1 3.4_20130816_Chapter11_2 3.4_20130816_src_files_modify Cpu0_lld_20130816 llvm-objdump ${GEN_DIR}/.
cp -rf cpu0_verilog ${GEN_DIR}/.
cp -rf removecpu0.sh readme ${GEN_DIR}/.
rm -f `find . -name .DS_Store`
