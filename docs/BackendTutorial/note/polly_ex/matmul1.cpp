// clang -O3 -DTEST matmul1.cpp -o matmul1
// clang -O3 -DTEST matmul-raw.c -o matmul-raw
// time matmul1 > 1
// time matmul-raw > 2
// diff 1 2

#include <stdio.h>

//#define T 8 // 3.81 vs 22.76 (original)
//#define T 32 // 3.10
//#define T 48 // 3.42

#define T 12 // best value 1.60 vs 16.0

//#define N 1536
#define N 512
float A[N][N];
float B[N][N];
float C[N][N];

int min(int a, int b)
{
  if (a < b)
    return a;
  else
    return b;
}

void init_array()
{
    int i, j;

    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            A[i][j] = (1+(i*j)%1024)/2.0;
            B[i][j] = (1+(i*j)%1024)/2.0;
        }
    }
}

#ifdef TEST
void print_array()
{
    int i, j;

    for (i=0; i<N; i++) {
        for (j=0; j<N; j++) {
            fprintf(stdout, "%lf ", C[i][j]);
            if (j%80 == 79) fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
    }
}
#endif

int main()
{
    int i, j, k;
    int ii, jj, kk;
    double t_start, t_end;

    init_array();
#if 1
    for (i=0;i<N;i=i+T) {
      for (j=0;j<N;j=j+T) {
        for (k=0;k<N;k=k+T) {
          for (ii=i;ii<=min(i+T-1,N);ii++) {
            for (jj=j;jj<=min(j+T-1,N);jj++) {
              for (kk=k;kk<=min(k+T-1,N);kk++) {
                C[ii][jj] = C[ii][jj] + A[ii][kk] * B[kk][jj];
              }
            }
          }
        }
      }
    }
#else
    static int n = N;
    for (i=0;i<n;i=i+T) {
      for (j=0;j<n;j=j+T) {
        for (k=0;k<n;k=k+T) {
          for (ii=i;ii<=min(i+T-1,n);ii++) {
            for (jj=j;jj<=min(j+T-1,n);jj++) {
              for (kk=k;kk<=min(k+T-1,n);kk++) {
                C[ii][jj] = C[ii][jj] + A[ii][kk] * B[kk][jj];
              }
            }
          }
        }
      }
    }
#endif

#ifdef TEST
    print_array();
#endif
    return 0;
}
