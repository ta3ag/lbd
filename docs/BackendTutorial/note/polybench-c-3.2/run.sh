#!/bin/sh -a

TIME="time -f \"%E real, %U user, %S sys\""
#TIME=""

echo "time ./covariance"
./covariance 2> result
echo "time ./covariance.polly"
./covariance.polly 2> polly.result
diff result polly.result

echo "time ./2mm"
./2mm 2> result
echo "time ./2mm.polly"
./2mm.polly 2> polly.result
diff result polly.result

echo "time ./3mm"
./3mm 2> result
echo "time ./3mm.polly"
./3mm.polly 2> polly.result
diff result polly.result

echo "time ./atax"
./atax 2> result
echo "time ./atax.polly"
./atax.polly 2> polly.result
diff result polly.result

echo "time ./bicg"
./bicg 2> result
echo "time ./bicg.polly"
./bicg.polly 2> polly.result
diff result polly.result

#echo "time ./cholesky"
#./cholesky 2> result
#echo "time ./cholesky.polly"
#./cholesky.polly 2> polly.result
#diff result polly.result

echo "time ./doitgen"
./doitgen 2> result
echo "time ./doitgen.polly"
./doitgen.polly 2> polly.result
diff result polly.result

echo "time ./gemm"
./gemm 2> result
echo "time ./gemm.polly"
./gemm.polly 2> polly.result
diff result polly.result

echo "time ./gemver"
./gemver 2> result
echo "time ./gemver.polly"
./gemver.polly 2> polly.result
diff result polly.result

echo "time ./gesummv"
./gesummv 2> result
echo "time ./gesummv.polly"
./gesummv.polly 2> polly.result
diff result polly.result

echo "time ./mvt"
./mvt 2> result
echo "time ./mvt.polly"
./mvt.polly 2> polly.result
diff result polly.result

