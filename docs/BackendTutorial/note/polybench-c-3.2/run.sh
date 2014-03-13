#!/bin/sh -a

echo "time ./covariance"
time -f "%E real, %U user, %S sys" ./covariance 2> result
echo "time ./covariance.polly"
time -f "%E real, %U user, %S sys" ./covariance.polly 2> polly.result
diff result polly.result

echo "time ./2mm"
time -f "%E real, %U user, %S sys" ./2mm 2> result
echo "time ./2mm.polly"
time -f "%E real, %U user, %S sys" ./2mm.polly 2> polly.result
diff result polly.result

echo "time ./3mm"
time -f "%E real, %U user, %S sys" ./3mm 2> result
echo "time ./3mm.polly"
time -f "%E real, %U user, %S sys" ./3mm.polly 2> polly.result
diff result polly.result

echo "time ./atax"
time -f "%E real, %U user, %S sys" ./atax 2> result
echo "time ./atax.polly"
time -f "%E real, %U user, %S sys" ./atax.polly 2> polly.result
diff result polly.result

echo "time ./bicg"
time -f "%E real, %U user, %S sys" ./bicg 2> result
echo "time ./bicg.polly"
time -f "%E real, %U user, %S sys" ./bicg.polly 2> polly.result
diff result polly.result



