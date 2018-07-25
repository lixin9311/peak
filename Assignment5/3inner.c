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

static inline void rdtscp(uint64 *u) {
  asm volatile("rdtscp;shlq $32,%%rdx;orq %%rdx,%%rax;movq %%rax,%0"
               : "=q"(*u)::"%rax", "%rdx", "%rcx");
}

uint64 tickToUsec(uint64 ts1, uint64 ts2) {
  return (ts2 - ts1) / (CPUSPEED_MHZ);
}

static float8 inner_vec(float8 *x0, float8 *x1, float8 *x2, float8 *x3,
                        float8 *x4, float8 *x5, float8 *x6, float8 *x7,
                        float8 *x8, float8 *x9, float8 *x10, float8 *x11,
                        float8 *x12, float8 *x13, float8 *x14, float8 *x15,
                        float8 *y0, float8 *y1, float8 *y2, float8 *y3,
                        float8 *y4, float8 *y5, float8 *y6, float8 *y7,
                        float8 *y8, float8 *y9, float8 *y10, float8 *y11,
                        float8 *y12, float8 *y13, float8 *y14, float8 *y15,
                        long n) {
  long i;
  asm volatile("# BEGIN!!!");
  float8 z0 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z1 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z2 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z3 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z4 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z5 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z6 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z7 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z8 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z9 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z10 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z11 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z12 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z13 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z14 = {0, 0, 0, 0, 0, 0, 0, 0};
  float8 z15 = {0, 0, 0, 0, 0, 0, 0, 0};
  for (i = 0; i < n; i += U) {
#if U > 0
    z0 += *(y0 + i) * *(x0 + i);
#endif
#if U > 1
    z1 += *(y2 + i) * *(x1 + i);
#endif
#if U > 2
    z2 += *(y2 + i) * *(x2 + i);
#endif
#if U > 3
    z3 += *(y3 + i) * *(x3 + i);
#endif
#if U > 4
    z4 += *(x4 + i) * *(y4 + i);
#endif
#if U > 5
    z5 += *(x5 + i) * *(y5 + i);
#endif
#if U > 6
    z6 += *(x6 + i) * *(y6 + i);
#endif
#if U > 7
    z7 += *(x7 + i) * *(y7 + i);
#endif
#if U > 8
    z8 += *(x8 + i) * *(y8 + i);
#endif
#if U > 9
    z9 += *(x9 + i) * *(y9 + i);
#endif
#if U > 10
    z10 += *(x10 + i) * *(y10 + i);
#endif
#if U > 11
    z11 += *(x11 + i) * *(y11 + i);
#endif
#if U > 12
    z12 += *(x12 + i) * *(y12 + i);
#endif
#if U > 13
    z13 += *(x13 + i) * *(y13 + i);
#endif
#if U > 14
    z14 += *(x14 + i) * *(y14 + i);
#endif
#if U > 15
    z15 += *(x15 + i) * *(y15 + i);
#endif
  }
  asm volatile("# END!!!");
  return z0 + z1 + z2 + z3 + z4 + z5 + z6 + z7 + z8 + z9 + z10 + z11 + z12 +
         z13 + z14 + z15;
}

int main(int argc, char **argv) {
  uint64 ts1, ts2;
  int err;
  omp_set_num_threads(NPROC);
  long size = (argc > 1 ? atol(argv[1]) : 80000);
  long seed = (argc > 2 ? atol(argv[2]) : 76843802738543);
  float *x_ = 0;
  float *y_ = 0;
  if ((err = posix_memalign((void **)&x_, 64, 8 * U * size * sizeof(float)))) {
    printf("posix_memalign %s\n", strerror(err));
  }

  if ((err = posix_memalign((void **)&y_, 64, 8 * U * size * sizeof(float)))) {
    printf("posix_memalign %s\n", strerror(err));
  }
  int i;
  unsigned short rg[3] = {seed >> 16, seed >> 8, seed};
  for (i = 0; i < 8 * U * size; i++) {
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
  volatile float8 result[NPROC];
  rdtscp(&ts1);
#pragma omp parallel for
  for (i = 0; i < NPROC; i++) {
    result[i] = inner_vec(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12,
                          x13, x14, x15, y0, y1, y2, y3, y4, y5, y6, y7, y8, y9,
                          y10, y11, y12, y13, y14, y15, size);
  }

  rdtscp(&ts2);
  double flops = 2 * U * 8 * size * NPROC;
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