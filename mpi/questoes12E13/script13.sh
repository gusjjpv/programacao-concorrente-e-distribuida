#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=24
#SBATCH -p sequana_cpu_dev
#SBATCH -J mpi_odd_even_scaling
#SBATCH --exclusive
#SBATCH --time=00:15:00
#SBATCH --output=resultado_questao13_%j.log

echo "Job ID: $SLURM_JOB_ID"
echo "Nodes allocated: $SLURM_JOB_NODELIST"
echo "======================================"

cd /scratch/pex1272-ufersa/joao.lima2/questoes12E13/ || exit 1

module load gcc/14.2.0_sequana
module load openmpi/gnu/4.1.4_sequana

# Compila
mpicc -O2 -Wall -o mpi_odd_even_time mpi_odd_even_time.c

GLOBAL_N=96000000   # divisível por 1,2,4,8,16,24,48,96

#######################################
# 1 NÓ — dobrando processos
#######################################
echo "===== 1 NODE SCALING ====="
for NP in 1 2 4 8 16 24
do
    echo ""
    echo ">>> Executando com $NP processo(s) em 1 nó"
    mpirun -np $NP ./mpi_odd_even_time g $GLOBAL_N
done

#######################################
# 2 NÓS COMPLETOS
#######################################
echo ""
echo "===== 2 NODES (48 processes) ====="
mpirun -np 48 ./mpi_odd_even_time g $GLOBAL_N

#######################################
# 4 NÓS COMPLETOS
#######################################
echo ""
echo "===== 4 NODES (96 processes) ====="
mpirun -np 96 ./mpi_odd_even_time g $GLOBAL_N

echo ""
echo "FIM DO JOB"
