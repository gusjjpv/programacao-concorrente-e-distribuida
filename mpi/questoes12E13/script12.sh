#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=24
#SBATCH -p sequana_cpu_dev
#SBATCH -J mpi_fast_fix
#SBATCH --exclusive
#SBATCH --time=00:05:00           # <--- ALTERADO PARA 5 MINUTOS
#SBATCH --output=resultado_final_%j.log

echo "Job ID: $SLURM_JOB_ID - Executando em $SLURM_JOB_NODELIST"

cd /scratch/pex1272-ufersa/joao.lima2/questoes12E13/

module load gcc/14.2.0_sequana
module load openmpi/gnu/4.1.4_sequana

# Recompila só pra garantir
mpicc -O3 -o mpi_trap_time mpi_trap_time.c

# ==============================================================================
# QUESTAO 12: REGRA DO TRAPEZIO
# ==============================================================================
echo "==========================================================="
echo "Q12 - TRAPEZIO"
echo "==========================================================="

# Niveis: 340k, 3.4M, 34M, 340M, 3.4B
NS_TRAP="340000 3400000 34000000 340000000 3400000000"

for n in $NS_TRAP; do
    echo "--- [TRAP] Testando com N = $n ---"
    
    # Escala Intranode (1 nó)
    for p in 1 2 4 8 12 24; do
        mpiexec -n $p ./mpi_trap_time $n
    done

    # Escala Internode (2 e 4 nós)
    mpiexec -n 48 ./mpi_trap_time $n
    mpiexec -n 96 ./mpi_trap_time $n
    echo ""
done