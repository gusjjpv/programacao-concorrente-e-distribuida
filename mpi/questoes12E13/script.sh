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
mpicc -O3 -o mpi_sort_time mpi_odd_even_time.c

# ==============================================================================
# QUESTAO 12: REGRA DO TRAPEZIO
# ==============================================================================
echo "==========================================================="
echo "Q12 - TRAPEZIO"
echo "==========================================================="

# Niveis: 540k, 5.4M, 54M, 540M, 5.4B
NS_TRAP="540000 5400000 54000000 540000000 5400000000"

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

# ==============================================================================
# QUESTAO 13: ODD-EVEN SORT
# ==============================================================================
echo "==========================================================="
echo "Q13 - ODD-EVEN SORT"
echo "==========================================================="

# Niveis: 24k, 48k, 96k, 192k, 384k
NS_SORT="24000 48000 96000 192000 384000"

for n in $NS_SORT; do
    echo "--- [SORT] Testando com N = $n ---"

    # Escala Intranode
    for p in 1 2 4 8 12 24; do
        mpiexec -n $p ./mpi_sort_time $n
    done

    # Escala Internode
    mpiexec -n 48 ./mpi_sort_time $n
    mpiexec -n 96 ./mpi_sort_time $n
    echo ""
done

echo "FIM DO JOB $SLURM_JOB_ID"