#!/bin/bash
#
#SBATCH -o success.txt
#SBATCH -e error.txt
#SBATCH -p normal

echo $SLURM_NTASKS
prun ${1}.mpi -f ${2} -o ${3} -e ${4}
