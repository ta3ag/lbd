// sudo time ./ocount --events=l1d_pend_miss:pending  ./matmul

#include <stdio.h>

// When N is small (256 or 512), the instruction execution time is the major
// while L1 data cache miss time is minor. 
// (execution time / L1 data cache miss time) is huge.
// When N is large (20), the instruction execution time is the major
// while L1 data cache miss time is minor. 

//#define N 256 // (0.02/0.02=1, 0.304/0.015=20)
/*
cschen@cschen-BM6835-BM6635-BP6335:~/install/bin$ sudo time ./ocount --events=CPU_CLK_UNHALTED,l1d_pend_miss:pending  ./matmul

Events were actively counted for 27259443 nanoseconds.
Event counts (actual) for /home/cschen/install/bin/matmul:
	Event                   Count                    % time counted
	CPU_CLK_UNHALTED        69,964,278               100.00
	l1d_pend_miss           378,939,217              100.00
0.02user 0.00system 0:00.04elapsed 48%CPU (0avgtext+0avgdata 6864maxresident)k
0inputs+0outputs (0major+4438minor)pagefaults 0swaps
cschen@cschen-BM6835-BM6635-BP6335:~/install/bin$ sudo time ./ocount --events=CPU_CLK_UNHALTED,l1d_pend_miss:pending  ./matmul.polly 

Events were actively counted for 20448161 nanoseconds.
Event counts (actual) for /home/cschen/install/bin/matmul.polly:
	Event                   Count                    % time counted
	CPU_CLK_UNHALTED        62,394,225               100.00
	l1d_pend_miss           15,619,139               100.00
0.02user 0.00system 0:00.04elapsed 46%CPU (0avgtext+0avgdata 6864maxresident)k
0inputs+0outputs (0major+4443minor)pagefaults 0swaps*/

//#define N 512 // (0.18/0.13=1.3, 41/13=3)

//#define N 768 // (1.38/0.44=3.1, 123/27)
//#define N 1024 // (6.52/1.26=5.1, 314/169..601/91)

#define N 1536 // (22.3/3.52=6.3, 1519/304)
//#define N 2048 // (67.8/9.8=6.9, 18369/1063=17..15727/1298=12)
//#define N 3072 // (239.9/33.0=7.24, 67141/3755=17)



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

int main()
{
//int l;
    int i, j, k;
    double t_start, t_end;

    init_array();
//for (l=0; l < N; l++)
    for(i=0; i<N; i++)  {
        for(j=0; j<N; j++)  {
            C[i][j] = 0;
            for(k=0; k<N; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
        }
    }

#ifdef TEST
    print_array();
#endif
    return 0;
}
