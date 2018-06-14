#include "x86intrin.h"

#ifndef CPUSPEED_MHZ
#define CPUSPEED_MHZ 2100
#endif

typedef unsigned long long uint64;

static inline uint64 rdtscp() {
  uint64 u;
  asm volatile ("rdtscp;shlq $32,%%rdx;orq %%rdx,%%rax;movq %%rax,%0":"=q"(u)::"%rax", "%rdx", "%rcx");
  return u;
}

uint64 tickToUsec(uint64 ts1, uint64 ts2) {
  return (ts2 - ts1) / (CPUSPEED_MHZ);
}