#!/bin/bash

#DIFF=0
DIFF=1

TIME="time -f \"%E real, %U user, %S sys\""
#TIME=""

echo "\ntime ./covariance"
./covariance 2> result
echo "time ./covariance.polly"
./covariance.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result;
fi



echo "\ntime ./2mm"
./2mm 2> result
echo "time ./2mm.polly"
./2mm.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./3mm"
./3mm 2> result
echo "time ./3mm.polly"
./3mm.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./atax"
./atax 2> result
echo "time ./atax.polly"
./atax.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./bicg"
./bicg 2> result
echo "time ./bicg.polly"
./bicg.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

#echo "time ./cholesky"
#./cholesky 2> result
#echo "time ./cholesky.polly"
#./cholesky.polly 2> polly.result
#if [ ${DIFF} == 1 ]; then
#  diff result polly.result
#fi

echo "\ntime ./doitgen"
./doitgen 2> result
echo "time ./doitgen.polly"
./doitgen.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./gemm"
./gemm 2> result
echo "time ./gemm.polly"
./gemm.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./gemver"
./gemver 2> result
echo "time ./gemver.polly"
./gemver.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./gesummv"
./gesummv 2> result
echo "time ./gesummv.polly"
./gesummv.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./mvt"
./mvt 2> result
echo "time ./mvt.polly"
./mvt.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./symm"
./symm 2> result
echo "time ./symm.polly"
./symm.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./syr2k"
./syr2k 2> result
echo "time ./syr2k.polly"
./syr2k.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./syrk"
./syrk 2> result
echo "time ./syrk.polly"
./syrk.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./trisolv"
./trisolv 2> result
echo "time ./trisolv.polly"
./trisolv.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./trmm"
./trmm 2> result
echo "time ./trmm.polly"
./trmm.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi



echo "\ntime ./durbin"
./durbin 2> result
echo "time ./durbin.polly"
./durbin.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./dynprog"
./dynprog 2> result
echo "time ./dynprog.polly"
./dynprog.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

#echo "\ntime ./gramschmidt"
#./gramschmidt 2> result
#echo "time ./gramschmidt.polly"
#./gramschmidt.polly 2> polly.result
#if [ ${DIFF} == 1 ]; then
#  diff result polly.result
#fi

echo "\ntime ./lu"
./lu 2> result
echo "time ./lu.polly"
./lu.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./ludcmp"
./ludcmp 2> result
echo "time ./ludcmp.polly"
./ludcmp.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi



echo "\ntime ./floyd-warshall"
./floyd-warshall 2> result
echo "time ./floyd-warshall.polly"
./floyd-warshall.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./reg_detect"
./reg_detect 2> result
echo "time ./reg_detect.polly"
./reg_detect.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi



echo "\ntime ./adi"
./adi 2> result
echo "time ./adi.polly"
./adi.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./fdtd-2d"
./fdtd-2d 2> result
echo "time ./fdtd-2d.polly"
./fdtd-2d.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./fdtd-apml"
./fdtd-apml 2> result
echo "time ./fdtd-apml.polly"
./fdtd-apml.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./jacobi-1d-imper"
./jacobi-1d-imper 2> result
echo "time ./jacobi-1d-imper.polly"
./jacobi-1d-imper.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./jacobi-2d-imper"
./jacobi-2d-imper 2> result
echo "time ./jacobi-2d-imper.polly"
./jacobi-2d-imper.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

echo "\ntime ./seidel-2d"
./seidel-2d 2> result
echo "time ./seidel-2d.polly"
./seidel-2d.polly 2> polly.result
if [ ${DIFF} == 1 ]; then
  diff result polly.result
fi

