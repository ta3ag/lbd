#!/bin/sh -a

LLVM_INSTALL=~/test/polly/llvm_build
alias pollycc="${LLVM_INSTALL}/bin/clang -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c datamining/covariance/covariance.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o covariance

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/2mm/2mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 2mm

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/3mm/3mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 3mm

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/atax/atax.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o atax

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/bicg/bicg.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o bicg

#${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/cholesky/cholesky.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o cholesky

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/doitgen/doitgen.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o doitgen

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/gemm/gemm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o gemm

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/gemver/gemver.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o gemver

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/gesummv/gesummv.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o gesummv

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/mvt/mvt.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o mvt






pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c datamining/covariance/covariance.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o covariance.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/2mm/2mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 2mm.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/3mm/3mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 3mm.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/atax/atax.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o atax.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/bicg/bicg.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o bicg.polly

#pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/cholesky/cholesky.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o cholesky

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/doitgen/doitgen.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o doitgen.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/gemm/gemm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o gemm.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/gemver/gemver.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o gemver.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/gesummv/gesummv.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o gesummv.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/mvt/mvt.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o mvt.polly




