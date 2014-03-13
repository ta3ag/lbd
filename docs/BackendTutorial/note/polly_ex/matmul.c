#include <stdio.h>

#define N 1536*1
float A[N][N];
float B[N][N];
float C[N][N];

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

#if 1
void matmul(float a[N][N], float b[N][N], float c[N][N])
{
    int i, j, k;

    for(i=0; i<N; i++)  {
        for(j=0; j<N; j++)  {
            c[i][j] = 0;
            for(k=0; k<N; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}
#else
void matmul()
{
    int i, j, k;

    for(i=0; i<N; i++)  {
        for(j=0; j<N; j++)  {
            C[i][j] = 0;
            for(k=0; k<N; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
        }
    }
}
#endif

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
    double t_start, t_end;

    init_array();

#if 0
    for(i=0; i<N; i++)  {
        for(j=0; j<N; j++)  {
            C[i][j] = 0;
            for(k=0; k<N; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
        }
    }
#else
// The pointer such as A, B, C make polly cannot optimization.
// It can be solved by pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 matmul.c -o matmul.polly
    matmul(A, B, C);
#endif

#ifdef TEST
    print_array();
#endif
    return 0;
}
