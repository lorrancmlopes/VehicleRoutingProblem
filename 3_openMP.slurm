#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --partition=espec
#SBATCH --nodes=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=1G

./3_openMP