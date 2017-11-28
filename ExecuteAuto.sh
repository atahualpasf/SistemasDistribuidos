#!/bin/bash
#
#SBATCH -p normal

prun ${1}.mpi -f ${2} -o ${3} -e ${4} -k ${5}