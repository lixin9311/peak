#!/bin/sh
#PBS -q u-lecture
#PBS -l select=1:mpiprocs=2:ompthreads=10
#PBS -W group_list=gt09
#PBS -l walltime=00:10:00
#PBS -N MPI_PingPong
cd $PBS_O_WORKDIR
. /etc/profile.d/modules.sh

mpirun ./stencil_mpi > ./output_mpi.log 2> ./bench_mpi.log
./stencil_serial > ./output_serial.log 2> ./bench_serial.log