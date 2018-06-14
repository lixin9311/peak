#!/bin/sh
#PBS -q h-lecture
#PBS -l select=1:mpiprocs=2:ompthreads=1
#PBS -W group_list=gt09
#PBS -l walltime=00:10:00
#PBS -N MPI_PingPong
cd $PBS_O_WORKDIR
. /etc/profile.d/modules.sh

mpirun ./a.out 2>&1 > ./output.log