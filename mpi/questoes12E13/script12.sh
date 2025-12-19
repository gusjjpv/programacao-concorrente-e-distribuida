#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=24
#SBATCH -p sequana_cpu_dev
#SBATCH -J mpi_trap_np_scaling
#SBATCH --exclusive
#SBATCH --time=00:15:00
#SBATCH --output=resultado_questao12_%j.log

echo "Job ID: $SLURM_JOB_ID"
echo "Nodes allocated: $SLURM_JOB_NODELIST"
echo "======================================"

cd /scratch/pex1272-ufersa/joao.lima2/questoes12E13/ || exit 1

module load gcc/14.2.0_sequana
module load openmpi/gnu/4.1.4_sequana

# Compila
mpicc -O2 -Wall -o mpi_trap_time mpi_trap_time.c

A=0.0
B=1.0

# Diferentes números de trapézios
N_LIST="10000000000 20000000000 40000000000"

#######################################
# Loop em n
#######################################
for N in $N_LIST
do
    echo ""
    echo "######################################"
    echo "### Experimentos com n = $N"
    echo "######################################"
    echo ""

    ###################################
    # 1 NÓ — dobrando processos
    ###################################
    echo "===== 1 NODE SCALING ====="
    for NP in 1 2 4 8 16 24
    do
        echo ""
        echo ">>> n=$N | p=$NP"
        echo "$A $B $N" | mpirun -np $NP ./mpi_trap_time
    done

    ###################################
    # 2 NÓS COMPLETOS
    ###################################
    echo ""
    echo "===== 2 NODES (48 processes) ====="
    echo ">>> n=$N | p=48"
    echo "$A $B $N" | mpirun -np 48 ./mpi_trap_time

    ###################################
    # 4 NÓS COMPLETOS
    ###################################
    echo ""
    echo "===== 4 NODES (96 processes) ====="
    echo ">>> n=$N | p=96"
    echo "$A $B $N" | mpirun -np 96 ./mpi_trap_time
done

echo ""
echo "FIM DO JOB"