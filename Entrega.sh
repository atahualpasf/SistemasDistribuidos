#!/bin/bash

if (($4 == 1))
then
	min_number=4
	#min_number=2
	max_number=0
	if (($1 == 1)) || (($1 == 2)) || (($1 == 3)) #|| (($1 == 4))
	then
		min_number=$(($1 * $min_number)) #[$1 * min_number(4)] para 3 fases o [$1 * min_number(2)] para 4 fases
		max_number=$(($min_number + 12)) #[min_number + 12] para determinar el máximo número de nucleos a usar
		echo $min_number
		echo $max_number
	else
		echo "Argumentos incorrectos"
		exit 128 #Invalid argument to exit
	fi
	nodes=$(($min_number / 2))
	corrida=1
	output_name=$(echo $3 | cut -d'.' -f 1)
	output_ext=$(echo $3 | cut -d'.' -f 2)
	job_id=$(echo $( sbatch -J F$1C$corrida'P'$min_number -N $nodes -n $min_number -e error.txt -o output.txt ExecuteAuto.sh Project $2 $output_name$1$corrida'.'$output_ext $4 $5) | cut -d' ' -f 4)
	min_number=$(($min_number + 4))
	for ((i = min_number; i <= max_number ; i+=4))
	do
		nodes=$(($i / 2))
		corrida=$(($corrida + 1))
		job_id=$(echo $( sbatch --dependency=afterok:$job_id -J F$1C$corrida'P'$i -N $nodes -n $i ExecuteAuto.sh Project $output_name$1$(($corrida-1))'.'$output_ext $output_name$1$corrida'.'$output_ext $4 $5) | cut -d' ' -f 4)
	done
elif (($4 == 0))
then
	min_number=4
	#min_number=2
	max_number=0
	if (($1 == 1)) || (($1 == 2)) || (($1 == 3)) #|| (($1 == 4))
	then
		min_number=$(($1 * $min_number)) #[$1 * min_number(4)] para 3 fases o [$1 * min_number(2)] para 4 fases
		max_number=$(($min_number + 12)) #[min_number + 12] para determinar el máximo número de nucleos a usar
		echo $min_number
		echo $max_number
	else
		echo "Argumentos incorrectos"
		exit 128 #Invalid argument to exit
	fi
	nodes=$(($max_number / 2))
	corrida=4
	output_name=$(echo $3 | cut -d'.' -f 1)
	output_ext=$(echo $3 | cut -d'.' -f 2)
	job_id=$(echo $( sbatch -J F$1C$corrida'P'$max_number -N $nodes -n $max_number -e error.txt -o output.txt ExecuteAuto.sh Project $2 $output_name$1$corrida'.'$output_ext $4 $5) | cut -d' ' -f 4)
	max_number=$(($max_number - 4))
	for ((i = max_number; i >= min_number ; i-=4))
	do
		nodes=$(($i / 2))
		corrida=$(($corrida - 1))
		job_id=$(echo $( sbatch --dependency=afterok:$job_id -J F$1C$corrida'P'$i -N $nodes -n $i ExecuteAuto.sh Project $output_name$1$(($corrida+1))'.'$output_ext $output_name$1$corrida'.'$output_ext $4 $5) | cut -d' ' -f 4)
	done
fi