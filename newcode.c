#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

void inicializa(int *n, int **vIn, int *nLocal, char *nomeArqIn, int rank, int size)
{
    FILE *arqIn = fopen(nomeArqIn, "rt");
    if (arqIn == NULL)
    {
        printf("\nArquivo texto de entrada não encontrado\n");
        MPI_Finalize();
        exit(1);
    }

    if (rank == 0)
    {
        fscanf(arqIn, "%d", n);
    }

    MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    *nLocal = (*n + size - 1) / size;  // Ajuste na divisão para garantir que todos os processos tenham informações suficientes

    *vIn = (int *)malloc((*nLocal) * sizeof(int));
    if (*vIn == NULL)
    {
        printf("\nErro na alocação de estruturas\n");
        MPI_Finalize();
        exit(1);
    }

    int *tempVIn = NULL;

    if (rank == 0)
    {
        tempVIn = (int *)malloc((*n) * sizeof(int));
        if (tempVIn == NULL)
        {
            printf("\nErro na alocação de estruturas\n");
            MPI_Finalize();
            exit(1);
        }

        for (int i = 0; i < *n; i++)
        {
            fscanf(arqIn, "%d", &(tempVIn[i]));
        }

        fclose(arqIn);
    }

    MPI_Scatter(tempVIn, *nLocal, MPI_INT, *vIn, *nLocal, MPI_INT, 0, MPI_COMM_WORLD);

    free(tempVIn);
}

void finaliza(int nLocal, int *vOut, char *nomeArqOut, int rank, int size)
{
    int *vOutGlobal = NULL;

    if (rank == 0)
    {
        vOutGlobal = (int *)malloc(nLocal * size * sizeof(int));
        if (vOutGlobal == NULL)
        {
            printf("\nErro na alocação de estruturas\n");
            MPI_Finalize();
            exit(1);
        }
    }

    MPI_Gather(vOut, nLocal, MPI_INT, vOutGlobal, nLocal, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        FILE *arqOut = fopen(nomeArqOut, "wt");
        if (arqOut == NULL)
        {
            printf("\nErro na abertura do arquivo de saída\n");
            MPI_Finalize();
            exit(1);
        }

        fprintf(arqOut, "%d\n", nLocal * size);
        for (int i = 0; i < nLocal * size; i++)
        {
            fprintf(arqOut, "%d ", vOutGlobal[i]);
        }
        fprintf(arqOut, "\n");

        fclose(arqOut);

        free(vOutGlobal);
    }
}


void soma_sufixos(int n, int *vIn, int *vOut, int rank, int size, int nLocal)
{
    int localSum = 0;
    for (int i = 0; i < nLocal; i++)
    {
        localSum += vIn[i];
        vOut[i] = localSum;
    }

    int sum = 0;
    MPI_Allreduce(&vOut[nLocal - 1], &sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);  // Usar MPI_Allreduce para calcular a soma acumulada de todos os elementos

    int prefixSum = 0;
    for (int i = 0; i < rank; i++)
    {
        prefixSum += sum;
    }

    for (int i = 0; i < nLocal; i++)
    {
        vOut[i] += prefixSum;
    }
}

int main(int argc, char **argv)
{
    int n, *vIn, *vOut;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3)
    {
        if (rank == 0)
        {
            printf("O programa foi executado com argumentos incorretos.\n");
            printf("Uso: mpirun -np <num_procs> ./somasuf_mpi arquivo_entrada arquivo_saida\n");
        }
        MPI_Finalize();
        exit(1);
    }

    int nLocal;

    inicializa(&n, &vIn, &nLocal, argv[1], rank, size);

    vOut = (int *)malloc(nLocal * sizeof(int));
    if (vOut == NULL)
    {
        printf("\nErro na alocação de estruturas\n");
        MPI_Finalize();
        exit(1);
    }

    struct timeval tIni, tFim;
    if (rank == 0)
    {
        gettimeofday(&tIni, NULL);
    }

    soma_sufixos(n, vIn, vOut, rank, size, nLocal);

    if (rank == 0)
    {
        gettimeofday(&tFim, NULL);
        long segundos = tFim.tv_sec - tIni.tv_sec;
        long microsegundos = tFim.tv_usec - tIni.tv_usec;
        double tempo = (segundos * 1e3) + (microsegundos * 1e-3);
        printf("Tempo=%.2f milissegundos\n", tempo);
    }

    finaliza(nLocal, vOut, argv[2], rank, size);

    free(vIn);
    free(vOut);

    MPI_Finalize();

    return 0;
}
