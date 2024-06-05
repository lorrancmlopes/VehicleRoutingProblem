#!/bin/bash
#SBATCH -N 2
#SBATCH --partition=espec
#SBATCH --mem=1G
#SBATCH --nodes=2
#SBATCH --cpus-per-task=16


# Set the number of OpenMP threads
export OMP_NUM_THREADS=16

# Run the MPI program
mpirun ./4_mpi
