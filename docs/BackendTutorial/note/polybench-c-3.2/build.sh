#!/bin/sh -a

LLVM_INSTALL=~/test/polly/llvm_build
alias pollycc="${LLVM_INSTALL}/bin/clang -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c datamining/covariance/covariance.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o covariance

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/2mm/2mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 2mm

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/3mm/3mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 3mm

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/atax/atax.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o atax

${LLVM_INSTALL}/bin/clang -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/bicg/bicg.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o bicg






pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c datamining/covariance/covariance.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o covariance.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/2mm/2mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 2mm.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/3mm/3mm.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o 3mm.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/atax/atax.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o atax.polly

pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -I utilities -I datamining/covariance utilities/polybench.c linear-algebra/kernels/bicg/bicg.c -DPOLYBENCH_TIME -DPOLYBENCH_DUMP_ARRAYS -DPOLYBENCH_USE_SCALAR_LB -o bicg.polly





