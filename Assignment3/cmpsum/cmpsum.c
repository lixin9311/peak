/* compute sum of data: same function as MPI_AllReduce
    Note: deadlock-free version with Isend and Irecv */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double cmpsum(double data) {
  int nproc, myid, ix, dst;
  double recv;
  MPI_Status st;
  MPI_Request sreq, rreq;

  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  for (ix = 1; ix < nproc; ix *= 2) {
    dst = myid ^ ix;

    MPI_Irecv((void*)&recv, 1, MPI_DOUBLE, dst, 0, MPI_COMM_WORLD, &rreq);
    MPI_Isend((void*)&data, 1, MPI_DOUBLE, dst, 0, MPI_COMM_WORLD, &sreq);

    MPI_Wait(&rreq, &st);
    MPI_Wait(&sreq, &st);

    data += recv;
  }

  return data;
}

int main(int argc, char **argv) {
  int nproc, myid;
  double data, sum;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  data = myid;
  sum = cmpsum(data);
  if (myid == 0)
    printf("%f %d\n", sum, nproc * (nproc - 1) / 2);

  MPI_Finalize();
}