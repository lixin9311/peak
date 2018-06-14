#include <mpi.h>
#include <stdio.h>
#include "timing.h"

#define START_SIZE 1
#define END_SIZE 10000000000
#define MULTIPLIER 2
#define ROUND 10

int main(int argc, char *argv[]) {
  int i, size, rank, err, buff_size;
  uint64 ts1, ts2, wtime;
  char *send, *recv;
  MPI_Status stat;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  for (buff_size = START_SIZE; buff_size < END_SIZE; buff_size *= MULTIPLIER) {
    // buff_size = 10;
    send = malloc(buff_size * sizeof(char));
    recv = malloc(buff_size * sizeof(char));
    for (i = 0; i < buff_size; i++) send[i] = rank;
    ts1 = rdtscp();
    for (i = 0; i < ROUND; i++) {
      if (rank == 0) {
        MPI_Send(send, buff_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(recv, buff_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &stat);
      } else if (rank == 1) {
        MPI_Recv(recv, buff_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &stat);
        MPI_Send(send, buff_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
      }
    }
    ts2 = rdtscp();
    wtime = tickToUsec(ts1, ts2) / ROUND;
    if (rank == 0)
      printf("Buff Size: %d Bytes, Used Time: %d usec, Thoughput: %0.0lf B/s\n",
             buff_size, wtime, buff_size * 1000000.0 / wtime);
    free(send);
    free(recv);
  }
  MPI_Finalize();
}