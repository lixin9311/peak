#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "timing.h"
#include "string.h"

#define INNER 1.0
#define OUTTER 0.0

#ifdef N
#define X_SIZE N
#define Y_SIZE N
#define Z_SIZE N
#endif

#ifndef X_SIZE
#define X_SIZE 32
#endif

#ifndef Y_SIZE
#define Y_SIZE 32
#endif

#ifndef Z_SIZE
#define Z_SIZE 32
#endif

#ifndef NT
#define NT 300
#endif

#ifndef NTICKS
#define NTICKS 1
#endif

#define SEND_TO_LEFT(buf, nticks)                                      \
  MPI_Send(&(buf)[(nticks)*y_size * z_size], y_size *z_size *(nticks), \
           MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD)
#define SEND_TO_RIGHT(buf, nticks)                                \
  MPI_Send(&(buf)[(slice_size - 2 * (nticks)) * y_size * z_size], \
           y_size *z_size *(nticks), MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD)
#define RECV_FROM_LEFT(buf, nticks)                                      \
  MPI_Recv(&(buf)[0], y_size *z_size *(nticks), MPI_DOUBLE, rank - 1, 0, \
           MPI_COMM_WORLD, &status)
#define RECV_FROM_RIGHT(buf, nticks)                                          \
  MPI_Recv(&(buf)[(slice_size - (nticks)) * y_size * z_size],                 \
           y_size *z_size *(nticks), MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, \
           &status)

#define U0(I, J, K) u0[(I)*y_size * z_size + (J)*z_size + (K)]
#define U1(I, J, K) u1[(I)*y_size * z_size + (J)*z_size + (K)]

/* one time step of computations */
static inline void vector_plus(int n, double *u0, double *u1, double r) {
  int di;
  for (di = 0; di < n; di++) {
    u1[di] += r * u0[di];
  }
}

void timeStep(int nticks, int numprocs, int rank, int slice_size, int y_size,
              int z_size, double *u0, double *u1, double r) {
  static MPI_Status status;
  int i, j, k;
  int x_start, x_end;
  int tick;
  double *tmp;
  // step 1, tik tok
  for (tick = 0; tick < nticks; tick++) {
    if (rank == 0) {
      x_start = 1;
      x_end = slice_size - tick - 1;
    } else if (rank == numprocs - 1) {
      x_start = tick + 1;
      x_end = slice_size - 1;
    } else {
      x_start = tick + 1;
      x_end = slice_size - tick - 1;
    }
    // middle
    for (i = x_start; i < x_end; i++) {  // only X will be restricted
      for (j = 1; j < y_size - 1; j++) {
        for (k = 1; k < z_size - 1; k++) {
          U1(i, j, k) = (1.0 - 6.0 * r) * U0(i, j, k);
          U1(i, j, k) += r * (U0(i, j, k + 1) + U0(i, j, k - 1));
        }
        vector_plus(z_size, &U0(i, j + 1, 0), &U1(i, j, 0), r);
        vector_plus(z_size, &U0(i, j - 1, 0), &U1(i, j, 0), r);
      }
      vector_plus(y_size * z_size, &U0(i + 1, 0, 0), &U1(i, 0, 0), r);
      vector_plus(y_size * z_size, &U0(i - 1, 0, 0), &U1(i, 0, 0), r);
    }
    // u0 is always the newest
    tmp = u1;
    u1 = u0;
    u0 = tmp;
  }

  // exchange and update the left boundry and right boundry
  // odd number rank
  if (rank % 2 == 1) {
    // send left to left & recv right
    if (rank != 0) SEND_TO_LEFT(u0, nticks);
    if (rank != numprocs - 1) RECV_FROM_RIGHT(u0, nticks);

    // send right to right & recv left
    if (rank != numprocs - 1) SEND_TO_RIGHT(u0, nticks);
    if (rank != 0) RECV_FROM_LEFT(u0, nticks);

    // even number rank
  } else if (rank % 2 == 0) {
    // recv right from right & send left
    if (rank != numprocs - 1) RECV_FROM_RIGHT(u0, nticks);
    if (rank != 0) SEND_TO_LEFT(u0, nticks);

    // recv left from left & send right
    if (rank != 0) RECV_FROM_LEFT(u0, nticks);
    if (rank != numprocs - 1) SEND_TO_RIGHT(u0, nticks);
  }
}

int main(int argc, char **argv) {
  int i, j, k, err;
  int slice_size;
  int numtasks, rank;
  double *u0, *u1;
  int x_size, y_size, z_size;
  int effective_size;
  uint64 ts1, ts2, ts3, wtime;

  int nticks = NTICKS;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* n is size of memory allocation
     boundaries : index 0 and n-1
     inner part : index 1 to n-2
     mesh size (discretization) is 1.0/(n-1) */
  x_size = X_SIZE;
  y_size = Y_SIZE;
  z_size = Z_SIZE;
  int oversize = 0;
  double T = 0.02;
  int nt = NT;
  double dt = T / nt;
  double dx = 1.0 / (x_size - 1);
  double kappa = 1.0;
  double r = kappa * dt / (dx * dx);

  if (nt < x_size) {
    fprintf(stderr, "nt (%d) is too small: should be >= x_size (%d)\n", nt,
            x_size);
    exit(0);
  }

  if (6.0 * r >= 1.0) {
    fprintf(stderr, "unstable condition (r=%e): nt should be larger\n", r);
    exit(0);
  }
  effective_size = (x_size - 2) / numtasks;
  if (rank == 0 || rank == numtasks - 1) {
    slice_size = effective_size + 1 + nticks;
  } else {
    slice_size = effective_size + 2 * nticks;
  }

  if (effective_size < 1) {
    fprintf(stderr,
            "too many processes, num of procs(%d) must be larger than "
            "x_size-2(%d)\n",
            numtasks, x_size - 2);
    exit(1);
  }

  if (nticks > effective_size / 2) {
    fprintf(stderr,
            "too many ticks per tick, num of ticks(%d) must be smaller than "
            "effective_size(%d) / 2\n",
            nticks, effective_size / 2);
    exit(1);
  }

  if (rank == 0) {
    oversize = (x_size - 2) % numtasks;
    slice_size += oversize;
  }
  if (rank == 0) {  // master have full size
    if ((err = posix_memalign((void **)&u0, 64,
                              sizeof(double) * x_size * y_size * z_size))) {
      fprintf(stderr, "posix_memalign %s\n", strerror(err));
    }
    if ((err = posix_memalign((void **)&u1, 64,
                              sizeof(double) * x_size * y_size * z_size))) {
      fprintf(stderr, "posix_memalign %s\n", strerror(err));
    }

  } else {
    if ((err = posix_memalign((void **)&u0, 64,
                              sizeof(double) * slice_size * y_size * z_size))) {
      fprintf(stderr, "posix_memalign %s\n", strerror(err));
    }
    if ((err = posix_memalign((void **)&u1, 64,
                              sizeof(double) * slice_size * y_size * z_size))) {
      fprintf(stderr, "posix_memalign %s\n", strerror(err));
    }
  }

  assert(u0 != NULL && u1 != NULL);

  // init the first and last slice
  if (rank == 0) {  // for master

    for (j = 0; j < y_size; j++) {
      for (k = 0; k < z_size; k++) {
        U0(0, j, k) = OUTTER;           // x = 0
        U0(x_size - 1, j, k) = OUTTER;  // x = 1
        U1(0, j, k) = OUTTER;           // x = 0
        U1(x_size - 1, j, k) = OUTTER;  // x = 1
      }
    }

    for (j = 1; j < y_size - 1; j++) {
      for (k = 1; k < z_size - 1; k++) {
        U0(slice_size - 1, j, k) = INNER;
        U1(slice_size - 1, j, k) = INNER;
      }
    }

  } else if (rank == numtasks - 1) {  // for the last one

    for (j = 0; j < y_size; j++) {
      for (k = 0; k < z_size; k++) {
        U0(slice_size - 1, j, k) = OUTTER;
        U1(slice_size - 1, j, k) = OUTTER;
      }
    }

    for (j = 1; j < y_size - 1; j++) {
      for (k = 1; k < z_size - 1; k++) {
        U0(0, j, k) = INNER;
        U1(0, j, k) = INNER;
      }
    }

  } else {  // in the middle

    for (j = 1; j < y_size - 1; j++) {
      for (k = 1; k < z_size - 1; k++) {
        U0(0, j, k) = INNER;
        U0(slice_size - 1, j, k) = INNER;
        U1(0, j, k) = INNER;
        U1(slice_size - 1, j, k) = INNER;
      }
    }
  }

  // init other fields
  for (i = 0; i < slice_size; i++) {
    for (j = 0; j < y_size; j++) {
      U0(i, j, 0) = OUTTER;
      U0(i, j, z_size - 1) = OUTTER;

      U1(i, j, 0) = OUTTER;
      U1(i, j, z_size - 1) = OUTTER;
    }
  }

  for (i = 0; i < slice_size; i++) {
    for (k = 0; k < z_size; k++) {
      U0(i, 0, k) = OUTTER;
      U0(i, y_size - 1, k) = OUTTER;

      U1(i, 0, k) = OUTTER;
      U1(i, y_size - 1, k) = OUTTER;
    }
  }

  /* initial value for inner part is one */
  for (i = 1; i < slice_size - 1; i++)
    for (j = 1; j < y_size - 1; j++)
      for (k = 1; k < z_size - 1; k++) U0(i, j, k) = 1.0;

  // wait here
  MPI_Barrier(MPI_COMM_WORLD);
  ts1 = rdtscp();
  int t;
  for (t = 0; t < nt; t += 2 * nticks) {
    /* time steps 1, 3, 5, ... */
    timeStep(nticks, numtasks, rank, slice_size, y_size, z_size, u0, u1, r);

    /* time steps 2, 4, 6, ... */
    if (nticks % 2 == 0) {
      timeStep(nticks, numtasks, rank, slice_size, y_size, z_size, u0, u1, r);
    } else {
      timeStep(nticks, numtasks, rank, slice_size, y_size, z_size, u1, u0, r);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  ts2 = rdtscp();
  // sync the result
  int index;
  if (rank == 0) {
    slice_size -= oversize;

    for (i = 1; i < numtasks; i++) {
      index = 1 + oversize + effective_size * i;
      MPI_Recv(&U0(index, 0, 0), y_size * z_size * effective_size, MPI_DOUBLE,
               i, 0, MPI_COMM_WORLD, &status);
    }
  } else {
    MPI_Send(&U0(nticks, 0, 0), y_size * z_size * effective_size, MPI_DOUBLE, 0,
             0, MPI_COMM_WORLD);
  }
  ts3 = rdtscp();
  /* plot result */
  if (rank == 0) {
    int x_step = (x_size < 30 ? 1 : x_size / 30); /* plot about 30x30 */
    int y_step = (y_size < 30 ? 1 : y_size / 30);
    k = (z_size - 1) / 2; /* about z = 0.5 */
    for (i = 0; i < x_size; i += x_step) {
      for (j = 0; j < y_size; j += y_step)
        printf("%e %e %e\n", i * dx, j * dx, U0(i, j, k));
      printf("\n");
    }
  }
  wtime = tickToUsec(ts1, ts2);
  if (rank == 0) {
    fprintf(stderr, "N Size: %d, NT: %d , Used Time: %llu usec", x_size, nt, wtime);
  }
  /* usage: a.out > res.txt
     plot it with gnuplot: splot "res.txt" with lines */
  MPI_Finalize();
  return 0;
}