#include <stdlib.h>
#include <string.h>
#include "omp.h"
#include "stdio.h"

#ifndef NPROC
#define NPROC 4
#endif

#define U 16

#ifndef CPUSPEED_MHZ
#define CPUSPEED_MHZ 2900
#endif

typedef float float8 __attribute__((vector_size(32)));
typedef unsigned long long uint64;
static unsigned int dummy;

static inline void rdtscp(uint64 *u) {
  asm volatile("rdtscp;shlq $32,%%rdx;orq %%rdx,%%rax;movq %%rax,%0"
               : "=q"(*u)::"%rax", "%rdx", "%rcx");
}

uint64 tickToUsec(uint64 ts1, uint64 ts2) {
  return (ts2 - ts1) / (CPUSPEED_MHZ);
}

static void copy_vec(
    float8 *restrict x0, float8 *restrict x1, float8 *restrict x2,
    float8 *restrict x3, float8 *restrict x4, float8 *restrict x5,
    float8 *restrict x6, float8 *restrict x7, float8 *restrict x8,
    float8 *restrict x9, float8 *restrict x10, float8 *restrict x11,
    float8 *restrict x12, float8 *restrict x13, float8 *restrict x14,
    float8 *restrict x15, float8 *restrict y0, float8 *restrict y1,
    float8 *restrict y2, float8 *restrict y3, float8 *restrict y4,
    float8 *restrict y5, float8 *restrict y6, float8 *restrict y7,
    float8 *restrict y8, float8 *restrict y9, float8 *restrict y10,
    float8 *restrict y11, float8 *restrict y12, float8 *restrict y13,
    float8 *restrict y14, float8 *restrict y15, long n) {
  long i;
  asm volatile("# BEGIN!!!");
  for (i = 0; i < n; i += U) {
#if U > 0
    *(x0 + i) = *(y0 + i);
#endif
#if U > 1
    *(x1 + i) = *(y2 + i);
#endif
#if U > 2
    *(x2 + i) = *(y2 + i);
#endif
#if U > 3
    *(x3 + i) = *(y3 + i);
#endif
#if U > 4
    *(x4 + i) = *(y4 + i);
#endif
#if U > 5
    *(x5 + i) = *(y5 + i);
#endif
#if U > 6
    *(x6 + i) = *(y6 + i);
#endif
#if U > 7
    *(x7 + i) = *(y7 + i);
#endif
#if U > 8
    *(x8 + i) = *(y8 + i);
#endif
#if U > 9
    *(x9 + i) = *(y9 + i);
#endif
#if U > 10
    *(x10 + i) = *(y10 + i);
#endif
#if U > 11
    *(x11 + i) = *(y11 + i);
#endif
#if U > 12
    *(x12 + i) = *(y12 + i);
#endif
#if U > 13
    *(x13 + i) = *(y13 + i);
#endif
#if U > 14
    *(x14 + i) = *(y14 + i);
#endif
#if U > 15
    *(x15 + i) = *(y15 + i);
#endif
  }
  asm volatile("# END!!!");
}

int main(int argc, char **argv) {
  uint64 ts1, ts2;
  omp_set_num_threads(NPROC);
  int err;
  long size = (argc > 1 ? atol(argv[1]) : 80000);
  long seed = (argc > 2 ? atol(argv[2]) : 76843802738543);
  float *x_ = 0;
  float *y_ = 0;
  if ((err = posix_memalign((void **)&x_, 64,
                            NPROC * 8 * U * size * sizeof(float)))) {
    printf("posix_memalign %s\n", strerror(err));
  }

  if ((err = posix_memalign((void **)&y_, 64,
                            NPROC * 8 * U * size * sizeof(float)))) {
    printf("posix_memalign %s\n", strerror(err));
  }
  int i;
  unsigned short rg[3] = {seed >> 16, seed >> 8, seed};
  for (i = 0; i < NPROC * 8 * U * size; i++) {
    x_[i] = erand48(rg);
    y_[i] = erand48(rg);
  }
  float8 *x0 = ((float8 *)&x_[0]);
  float8 *x1 = ((float8 *)&x_[8]);
  float8 *x2 = ((float8 *)&x_[16]);
  float8 *x3 = ((float8 *)&x_[24]);
  float8 *x4 = ((float8 *)&x_[32]);
  float8 *x5 = ((float8 *)&x_[40]);
  float8 *x6 = ((float8 *)&x_[48]);
  float8 *x7 = ((float8 *)&x_[56]);
  float8 *x8 = ((float8 *)&x_[64]);
  float8 *x9 = ((float8 *)&x_[72]);
  float8 *x10 = ((float8 *)&x_[80]);
  float8 *x11 = ((float8 *)&x_[88]);
  float8 *x12 = ((float8 *)&x_[96]);
  float8 *x13 = ((float8 *)&x_[104]);
  float8 *x14 = ((float8 *)&x_[112]);
  float8 *x15 = ((float8 *)&x_[120]);

  float8 *y0 = ((float8 *)&y_[0]);
  float8 *y1 = ((float8 *)&y_[8]);
  float8 *y2 = ((float8 *)&y_[16]);
  float8 *y3 = ((float8 *)&y_[24]);
  float8 *y4 = ((float8 *)&y_[32]);
  float8 *y5 = ((float8 *)&y_[40]);
  float8 *y6 = ((float8 *)&y_[48]);
  float8 *y7 = ((float8 *)&y_[56]);
  float8 *y8 = ((float8 *)&y_[64]);
  float8 *y9 = ((float8 *)&y_[72]);
  float8 *y10 = ((float8 *)&y_[80]);
  float8 *y11 = ((float8 *)&y_[88]);
  float8 *y12 = ((float8 *)&y_[96]);
  float8 *y13 = ((float8 *)&y_[104]);
  float8 *y14 = ((float8 *)&y_[112]);
  float8 *y15 = ((float8 *)&y_[120]);

  rdtscp(&ts1);
#pragma omp parallel for
  for (i = 0; i < NPROC; i++) {
    copy_vec(x0 + 128 * i, x1 + 128 * i, x2 + 128 * i, x3 + 128 * i,
             x4 + 128 * i, x5 + 128 * i, x6 + 128 * i, x7 + 128 * i,
             x8 + 128 * i, x9 + 128 * i, x10 + 128 * i, x11 + 128 * i,
             x12 + 128 * i, x13 + 128 * i, x14 + 128 * i, x15 + 128 * i,
             y0 + 128 * i, y1 + 128 * i, y2 + 128 * i, y3 + 128 * i,
             y4 + 128 * i, y5 + 128 * i, y6 + 128 * i, y7 + 128 * i,
             y8 + 128 * i, y9 + 128 * i, y10 + 128 * i, y11 + 128 * i,
             y12 + 128 * i, y13 + 128 * i, y14 + 128 * i, y15 + 128 * i, size);
  }

  rdtscp(&ts2);
  double flops = U * 8 * size * NPROC;
#ifndef PLOT
  printf("opt size %ld\n", size);
  printf("cache factor: %d\n", U);
  printf("processed data size: %ld bytes\n",
         8 * U * size * sizeof(float) * NPROC);
  printf("%.0f flops\n", flops);
  printf("%llu clocks\n", ts2 - ts1);
  printf("time: %llu us\n", tickToUsec(ts1, ts2));
  printf("%f flops/clock\n", flops / (ts2 - ts1));
  printf("%.1f GFLOP/s/core\n", flops / 1000 / tickToUsec(ts1, ts2));
  printf("throughput: %.2lf MBytes/s\n", NPROC * 8 * U * size * sizeof(float) *
                                             1000000.0 / tickToUsec(ts1, ts2) /
                                             1000000.0);
#else
  printf("%.2lf\n", NPROC * 8 * U * size * sizeof(float) * 1000000.0 /
                        tickToUsec(ts1, ts2) / 1000000.0);
#endif
  return 0;
}