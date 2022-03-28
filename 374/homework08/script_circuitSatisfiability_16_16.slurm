#!/bin/bash
# Example with 1 nodes, 4 processes each = 4 processes
#
# Set the number of nodes to use (max 20)
#SBATCH -N 1
#
# Set the number of processes per node (max 16)
#SBATCH --ntasks-per-node=1
#
#
#SBATCH --cpus-per-task=16

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

# Load the compiler and MPI library
module load openmpi-2.0/gcc

# Run the program
mpirun ./circuitSatisfiability
