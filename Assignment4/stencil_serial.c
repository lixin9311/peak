#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef NT
#define NT 300
#endif

#ifndef N
#define N 32
#endif

/* one time step of computations */
void timeStep(int n, double *u0, double *u1, double r) {
  int i, j, k;

  for (i=1; i< n-1; i++)
    for (j=1; j< n-1; j++)
      for (k=1; k< n-1; k++)
	u1[(i*n + j)*n + k]
	  = (1.0 - 6.0 * r)   * u0[(i*n + j)*n + k]
	  + r * (u0[((i+1)*n + j)*n + k] + u0[((i-1)*n + j)*n + k]
		 + u0[(i*n + j+1)*n + k] + u0[(i*n + j-1)*n + k]
		 + u0[(i*n + j)*n + k+1] + u0[(i*n + j)*n + k-1]);
} 

int main(void) {

  int n, i, j, k;

  /* n is size of memory allocation
     boundaries : index 0 and n-1
     inner part : index 1 to n-2
     mesh size (discretization) is 1.0/(n-1) */
  n = N;

  double *u0 = malloc(sizeof(double) * n * n * n);
  double *u1 = malloc(sizeof(double) * n * n * n);
  assert(u0 != NULL && u1 != NULL);

  /* boundary is set to zero */
  for (i=0; i< n; i++)
    for (j=0; j< n; j++) {
      u0[(i*n + j)*n + 0] = 0.0;	/* z = 0 */
      u0[(i*n + j)*n + n-1] = 0.0; /* z = 1 */
      u0[(i*n + 0)*n + j] = 0.0;	  /* y = 0 */
      u0[(i*n + n-1)*n + j] = 0.0; /* y = 1 */
      u0[(0*n + i)*n + j] = 0.0;	  /* x = 0 */
      u0[((n-1)*n + i)*n + j] = 0.0; /* x = 1 */
      
      u1[(i*n + j)*n + 0] = 0.0;	/* z = 0 */
      u1[(i*n + j)*n + n-1] = 0.0; /* z = 1 */
      u1[(i*n + 0)*n + j] = 0.0;	  /* y = 0 */
      u1[(i*n + n-1)*n + j] = 0.0; /* y = 1 */
      u1[(0*n + i)*n + j] = 0.0;	  /* x = 0 */
      u1[((n-1)*n + i)*n + j] = 0.0; /* x = 1 */
    }

  /* initial value for inner part is one */
  for (i=1; i< n-1; i++)
    for (j=1; j< n-1; j++)
      for (k=1; k< n-1; k++)
	u0[(i*n + j)*n + k] = 1.0;

  double T = 0.02;
  int nt = NT;
  double dt = T / nt;
  double dx = 1.0 / (n-1);
  double kappa = 1.0;
  double r = kappa * dt / (dx * dx);

  if (nt < n) {
    fprintf(stderr,
	    "nt (%d) is too small: should be >= n (%d)\n",
	    nt, n);
    exit(0);
  }
    
  if (6.0 * r >= 1.0) {
    fprintf(stderr,
	    "unstable condition (r=%e): nt should be larger\n",
	    r);
    exit(0);
  }
  
  int t;
  for (t = 0; t < nt; t += 2) {
    /* time steps 1, 3, 5, ... */
    timeStep(n, u0, u1, r);

    /* time steps 2, 4, 6, ... */
    timeStep(n, u1, u0, r);
  }

  /* plot result */
  int step = (n < 30 ? 1 : n / 30); /* plot about 30x30 */
  k = (n-1)/2;			/* about z = 0.5 */
  for (i=0; i< n; i+= step) {
    for (j=0; j< n; j+= step)
      printf("%e %e %e\n", i*dx, j*dx, u0[(i*n + j)*n + k]);
    printf("\n");
  }
  
  /* usage: a.out > res.txt
     plot it with gnuplot: splot "res.txt" with lines */
  
  return 0;
}