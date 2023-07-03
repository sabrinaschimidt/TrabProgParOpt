// ----------------------------------------------------------------------------
// Soma de sufixos utilizando MPI 
// Para compilar: mpicc somasuf_mpi.c -o somasuf_mpi -Wall
// Para executar:  mpirun -oversubscribe -np 8 somasuf_mpi arquivo_entrada.txt arquivo_saida.txt
// Código ajustado para que o processo 0 seja o único que possui vetores vIn e
// vOut de tamanho n, enquanto os demais processos possuem vetores de tamanho n/p

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

void inicializa(int *n, int **vIn, int *nLocal, int *extraElements, char *nomeArqIn, int size)
{
    FILE *arqIn = fopen(nomeArqIn, "rt");
    if (arqIn == NULL)
    {
        printf("\nArquivo texto de entrada não encontrado\n");
        exit(1);
    }

    fscanf(arqIn, "%d", n);
    *nLocal = (*n) / size;
    *extraElements = (*n) % size;

    int localSize = *nLocal + (*extraElements > 0 ? 1 : 0);
    *vIn = (int *)malloc(localSize * sizeof(int));
    if (*vIn == NULL)
    {
        printf("\nErro na alocação de estruturas\n");
        exit(1);
    }

    for (int i = 0; i < localSize; i++)
    {
        fscanf(arqIn, "%d", &((*vIn)[i]));
    }

    fclose(arqIn);
}

void finaliza(int nLocal, int *vOut, char *nomeArqOut)
{
    FILE *arqOut = fopen(nomeArqOut, "wt");
    if (arqOut == NULL)
    {
        printf("\nErro na abertura do arquivo de saída\n");
        exit(1);
    }

    fprintf(arqOut, "%d\n", nLocal);
    for (int i = 0; i < nLocal; i++)
    {
        fprintf(arqOut, "%d ", vOut[i]);
    }
    fprintf(arqOut, "\n");

    fclose(arqOut);
}

void soma_sufixos(int n, int *vIn, int *vOut, int rank, int size, int nLocal, int extraElements)
{
    int localStart = rank * nLocal;
    int localEnd = localStart + nLocal;

    if (rank == size - 1)
    {
        localEnd += extraElements;
    }

    int *tempOut = (int *)malloc((nLocal + 1) * sizeof(int));
    if (tempOut == NULL)
    {
        printf("\nErro na alocação de estruturas\n");
        MPI_Finalize();
        exit(1);
    }

    tempOut[0] = 0;
    for (int i = 1; i <= nLocal; i++)
    {
        tempOut[i] = tempOut[i - 1] + vIn[localStart + i - 1];
    }

    int *prefixSum = (int *)malloc(size * sizeof(int));
    if (prefixSum == NULL)
    {
        printf("\nErro na alocação de estruturas\n");
        MPI_Finalize();
        exit(1);
    }

    MPI_Allgather(&tempOut[nLocal], 1, MPI_INT, prefixSum, 1, MPI_INT, MPI_COMM_WORLD);

    int sum = 0;
    for (int i = 0; i < rank; i++)
    {
        sum += prefixSum[i];
    }

    for (int i = 0; i < nLocal; i++)
    {
        vOut[i] = vIn[i] + sum;
    }

    free(tempOut);
    free(prefixSum);
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

    int nLocal, extraElements;

    inicializa(&n, &vIn, &nLocal, &extraElements, argv[1], size);

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

    soma_sufixos(n, vIn, vOut, rank, size, nLocal, extraElements);

    if (rank == 0)
    {
        gettimeofday(&tFim, NULL);
        long segundos = tFim.tv_sec - tIni.tv_sec;
        long microsegundos = tFim.tv_usec - tIni.tv_usec;
        double tempo = (segundos * 1e3) + (microsegundos * 1e-3);
        printf("Tempo=%.2f milissegundos\n", tempo);
    }

    int *gatherBuffer = NULL;
    if (rank == 0)
    {
        gatherBuffer = (int *)malloc(n * sizeof(int));
        if (gatherBuffer == NULL)
        {
            printf("\nErro na alocação de estruturas\n");
            MPI_Finalize();
            exit(1);
        }
    }

    MPI_Gather(vOut, nLocal, MPI_INT, gatherBuffer, nLocal, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        finaliza(n, gatherBuffer, argv[2]);
        free(gatherBuffer);
    }

    free(vIn);
    free(vOut);

    MPI_Finalize();

    return 0;
}
