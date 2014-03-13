#!/bin/sh -a

TIME="time -f \"%E real, %U user, %S sys\""
#TIME=""

echo "\ntime ./covariance"
./covariance 2> result
echo "time ./covariance.polly"
./covariance.polly 2> polly.result
diff result polly.result

echo "\ntime ./2mm"
./2mm 2> result
echo "time ./2mm.polly"
./2mm.polly 2> polly.result
diff result polly.result

echo "\ntime ./3mm"
./3mm 2> result
echo "time ./3mm.polly"
./3mm.polly 2> polly.result
diff result polly.result

echo "\ntime ./atax"
./atax 2> result
echo "time ./atax.polly"
./atax.polly 2> polly.result
diff result polly.result

echo "\ntime ./bicg"
./bicg 2> result
echo "\ntime ./bicg.polly"
./bicg.polly 2> polly.result
diff result polly.result

#echo "time ./cholesky"
#./cholesky 2> result
#echo "time ./cholesky.polly"
#./cholesky.polly 2> polly.result
#diff result polly.result

echo "\ntime ./doitgen"
./doitgen 2> result
echo "time ./doitgen.polly"
./doitgen.polly 2> polly.result
diff result polly.result

echo "\ntime ./gemm"
./gemm 2> result
echo "time ./gemm.polly"
./gemm.polly 2> polly.result
diff result polly.result

echo "\ntime ./gemver"
./gemver 2> result
echo "time ./gemver.polly"
./gemver.polly 2> polly.result
diff result polly.result

echo "\ntime ./gesummv"
./gesummv 2> result
echo "time ./gesummv.polly"
./gesummv.polly 2> polly.result
diff result polly.result

echo "\ntime ./mvt"
./mvt 2> result
echo "time ./mvt.polly"
./mvt.polly 2> polly.result
diff result polly.result

echo "\ntime ./symm"
./symm 2> result
echo "time ./symm.polly"
./symm.polly 2> polly.result
diff result polly.result

echo "\ntime ./syr2k"
./syr2k 2> result
echo "time ./syr2k.polly"
./syr2k.polly 2> polly.result
diff result polly.result

echo "\ntime ./syrk"
./syrk 2> result
echo "time ./syrk.polly"
./syrk.polly 2> polly.result
diff result polly.result

echo "\ntime ./trisolv"
./trisolv 2> result
echo "time ./trisolv.polly"
./trisolv.polly 2> polly.result
diff result polly.result

echo "\ntime ./trmm"
./trmm 2> result
echo "time ./trmm.polly"
./trmm.polly 2> polly.result
diff result polly.result

