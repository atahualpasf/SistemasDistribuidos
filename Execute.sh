#!/bin/bash
#
#SBTACH -J elkelolea
#SBATCH -o success.txt
#SBATCH -e error.txt
#SBATCH -p normal
#SBATCH -N 2
#SBATCH -n 3

prun ${1}.mpi -f ${2}