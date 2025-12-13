#!/bin/bash
#SBATCH --job-name=MPI_Final
#SBATCH --account=pex1272-ufersa
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=36
#SBATCH --ntasks=144
#SBATCH --cpus-per-task=1
#SBATCH --time=00:15:00
#SBATCH --partition=sequana_cpu_dev
#SBATCH --exclusive
#SBATCH --output=resultado_escala_%j.log

# ===============================================================
# CONFIGURAÇÃO DO AMBIENTE
# ===============================================================
echo "--- Configurando módulos ---"
module purge

module load gcc/9.3.0_sequana
module load openmpi/gnu/4.1.6_sequana

echo "MPICC em uso:"
which mpicc
echo "MPIRUN em uso:"
which mpirun

# ===============================================================
# DEFINIR DIRETÓRIO DE TRABALHO
# ===============================================================
WORKDIR="/scratch/pex1272-ufersa/joao.lima2/questao12"
cd $WORKDIR || exit 1
echo "Diretório atual: $(pwd)"

# ===============================================================
# COMPILAÇÃO
# ===============================================================
echo "Compilando programa..."
mpicc -O3 -o mpi_trap mpi_trap3_timed.c -lm

echo "Binário gerado:"
ls -lh mpi_trap

# ===============================================================
# PARÂMETROS DO PROBLEMA
# ===============================================================
A=0
B=1
N=100000000      # Seu código lê via scanf

# Lista de números de processos:
P_LIST=(1 2 4 8 16 32 36 72 144)

echo "=========================================================="
echo "     INICIANDO EXPERIMENTOS MPI (N=$N)"
echo "     Nós alocados: $SLURM_JOB_NODELIST"
echo "=========================================================="

# ===============================================================
# LOOP PRINCIPAL DOS TESTES
# ===============================================================
for P in "${P_LIST[@]}"; do
    
    echo "----------------------------------------------------------"
    echo "Executando com P = $P processos"
    echo "----------------------------------------------------------"

    # Três execuções para média
    for i in {1..3}; do
        
        mpirun -np $P ./mpi_trap <<< "$A $B $N"
    
    done

    echo "----------------------------------------------------------"
done

echo "================== FIM DOS TESTES =================="
