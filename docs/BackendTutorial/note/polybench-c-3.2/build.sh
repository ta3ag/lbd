#!/bin/bash -a

rm -rf polybench
mkdir polybench

#BUILD_ARM=0
BUILD_ARM=1

#DUMP_RESULT=0
DUMP_RESULT=1

if [ ${BUILD_ARM} == 1 ]; then
CPU="-mcpu=cortex-a9 -mfloat-abi=hard -mfpu=neon"
LLVM_INSTALL=~/test/polly/llvm_arm_build
else
CPU=""
LLVM_INSTALL=~/test/polly/llvm_build
fi

if [ ${DUMP_RESULT} == 1 ]; then
echo "DUMP_RESULT == 1"
CFLAG="-O3 -I utilities -I datamining/covariance utilities/polybench.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB ${CPU}"
PCFLAG="-mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB ${CPU}"
else
CFLAG="-O3 -I utilities -I datamining/covariance utilities/polybench.c -DPOLYBENCH_TIME -DPOLYBENCH_USE_SCALAR_LB ${CPU}"
PCFLAG="-mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c -DPOLYBENCH_TIME -DPOLYBENCH_USE_SCALAR_LB ${CPU}"
fi

pollycc="${LLVM_INSTALL}/bin/clang -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"

${LLVM_INSTALL}/bin/clang ${CFLAG} datamining/covariance/covariance.c -o polybench/covariance

${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/2mm/2mm.c -o polybench/2mm
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/3mm/3mm.c -o polybench/3mm
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/atax/atax.c -o polybench/atax
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/bicg/bicg.c -o polybench/bicg
#${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/cholesky/cholesky.c -o polybench/cholesky
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/doitgen/doitgen.c -o polybench/doitgen
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/gemm/gemm.c -o polybench/gemm
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/gemver/gemver.c -o polybench/gemver
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/gesummv/gesummv.c -o polybench/gesummv
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/mvt/mvt.c -o polybench/mvt
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/symm/symm.c -o polybench/symm
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/syr2k/syr2k.c -o polybench/syr2k
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/syrk/syrk.c -o polybench/syrk
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/trisolv/trisolv.c -o polybench/trisolv
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/kernels/trmm/trmm.c -o polybench/trmm

${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/solvers/durbin/durbin.c -o polybench/durbin
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/solvers/dynprog/dynprog.c -o polybench/dynprog
#${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/solvers/gramschmidt/gramschmidt.c -o polybench/gramschmidt
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/solvers/lu/lu.c -o polybench/lu
${LLVM_INSTALL}/bin/clang ${CFLAG} linear-algebra/solvers/ludcmp/ludcmp.c -o polybench/ludcmp

${LLVM_INSTALL}/bin/clang ${CFLAG} medley/floyd-warshall/floyd-warshall.c -o polybench/floyd-warshall
${LLVM_INSTALL}/bin/clang ${CFLAG} medley/reg_detect/reg_detect.c -o polybench/reg_detect

${LLVM_INSTALL}/bin/clang ${CFLAG} stencils/adi/adi.c -o polybench/adi
${LLVM_INSTALL}/bin/clang ${CFLAG} stencils/fdtd-2d/fdtd-2d.c -o polybench/fdtd-2d
${LLVM_INSTALL}/bin/clang ${CFLAG} stencils/fdtd-apml/fdtd-apml.c -o polybench/fdtd-apml
${LLVM_INSTALL}/bin/clang ${CFLAG} stencils/jacobi-1d-imper/jacobi-1d-imper.c -o polybench/jacobi-1d-imper
${LLVM_INSTALL}/bin/clang ${CFLAG} stencils/jacobi-2d-imper/jacobi-2d-imper.c -o polybench/jacobi-2d-imper
${LLVM_INSTALL}/bin/clang ${CFLAG} stencils/seidel-2d/seidel-2d.c -o polybench/seidel-2d



${pollycc} ${PCFLAG} datamining/covariance/covariance.c -o polybench/covariance.polly

${pollycc} ${PCFLAG} linear-algebra/kernels/2mm/2mm.c -o polybench/2mm.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/3mm/3mm.c -o polybench/3mm.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/atax/atax.c -o polybench/atax.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/bicg/bicg.c -o polybench/bicg.polly
#${pollycc} ${PCFLAG} linear-algebra/kernels/cholesky/cholesky.c -o polybench/cholesky.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/doitgen/doitgen.c -o polybench/doitgen.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/gemm/gemm.c -o polybench/gemm.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/gemver/gemver.c -o polybench/gemver.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/gesummv/gesummv.c -o polybench/gesummv.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/mvt/mvt.c -o polybench/mvt.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/symm/symm.c -o polybench/symm.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/syr2k/syr2k.c -o polybench/syr2k.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/syrk/syrk.c -o polybench/syrk.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/trisolv/trisolv.c -o polybench/trisolv.polly
${pollycc} ${PCFLAG} linear-algebra/kernels/trmm/trmm.c -o polybench/trmm.polly

${pollycc} ${PCFLAG} linear-algebra/solvers/durbin/durbin.c -o polybench/durbin.polly
${pollycc} ${PCFLAG} linear-algebra/solvers/dynprog/dynprog.c -o polybench/dynprog.polly
#${pollycc} ${PCFLAG} linear-algebra/solvers/gramschmidt/gramschmidt.c -o polybench/gramschmidt.polly
${pollycc} ${PCFLAG} linear-algebra/solvers/lu/lu.c -o polybench/lu.polly
${pollycc} ${PCFLAG} linear-algebra/solvers/ludcmp/ludcmp.c -o polybench/ludcmp.polly

${pollycc} ${PCFLAG} medley/floyd-warshall/floyd-warshall.c -o polybench/floyd-warshall.polly
${pollycc} ${PCFLAG} medley/reg_detect/reg_detect.c -o polybench/reg_detect.polly

${pollycc} ${PCFLAG} stencils/adi/adi.c -o polybench/adi.polly
${pollycc} ${PCFLAG} stencils/fdtd-2d/fdtd-2d.c -o polybench/fdtd-2d.polly
${pollycc} ${PCFLAG} stencils/fdtd-apml/fdtd-apml.c -o polybench/fdtd-apml.polly
${pollycc} ${PCFLAG} stencils/jacobi-1d-imper/jacobi-1d-imper.c -o polybench/jacobi-1d-imper.polly
${pollycc} ${PCFLAG} stencils/jacobi-2d-imper/jacobi-2d-imper.c -o polybench/jacobi-2d-imper.polly
${pollycc} ${PCFLAG} stencils/seidel-2d/seidel-2d.c -o polybench/seidel-2d.polly

