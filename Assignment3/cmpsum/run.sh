#!/bin/sh 
#PBS -q u-lecture
#PBS -W group_list=gt09
#PBS -l select=1:mpiprocs=8:ompthreads=1
#PBS -l walltime=00:10:00

cd $PBS_O_WORKDIR
. /etc/profile.d/modules.sh

mpirun ./a.out 2>&1 > output.log
