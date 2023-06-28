#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

void inicializa(int *n, int **vIn, int **vOut, char *nomeArqIn)
{
    // Código de inicialização não foi alterado
}

void finaliza(int n, int *vIn, int *vOut, char *nomeArqOut)
{
    // Código de finalização não foi alterado
}

void soma_sufixos(int n, int *vIn, int *vOut, int rank, int size)
{
    // Calcula o intervalo de elementos que cada processo deve processar
    int local_size = n / size;
    int local_start = rank * local_size;
    int local_end = local_start + local_size;

    if (rank == size - 1)
    {
        // O último processo cuida dos elementos restantes, se houver
        local_end += n % size;
    }

    // Calcula a soma de sufixos localmente
    vOut[local_end - 1] = vIn[local_end - 1];
    for (int i = local_end - 2; i >= local_start; i--)
    {
        vOut[i] = vOut[i + 1] + vIn[i];
    }

    // Comunicação entre os processos para combinar os resultados
    for (int i = size - 1; i > 0; i--)
    {
        int recv_count;
        if (rank == i)
        {
            // Envia os elementos locais para o processo 0
            MPI_Send(&vOut[local_start], local_end - local_start, MPI_INT, 0, 0, MPI_COMM_WORLD);
            break;
        }
        else if (rank == 0)
        {
            // Recebe os elementos dos outros processos
            MPI_Recv(&vOut[local_start], local_end - local_start, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Get_count(MPI_STATUS_IGNORE, MPI_INT, &recv_count);
            local_end = local_start + recv_count;
        }
    }

    // Broadcast dos resultados para todos os processos
    MPI_Bcast(vOut, n, MPI_INT, 0, MPI_COMM_WORLD);
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

    if (rank == 0)
    {
        // Processo mestre aloca vetores de entrada e saída, lê dados do arquivo de entrada
        inicializa(&n, &vIn, &vOut, argv[1]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Todos os processos alocam espaço para os vetores de entrada e saída
    vIn = (int *)malloc(n * sizeof(int));
    vOut = (int *)malloc(n * sizeof(int));

    // Processo mestre lê vetor do arquivo de entrada e envia para os outros processos
    if (rank == 0)
    {
        for (int i = 0; i < n; i++)
        {
            fscanf(arqIn, "%d", &vIn[i]);
        }
        fclose(arqIn);

        // Envio dos dados para os outros processos
        for (int i = 1; i < size; i++)
        {
            int local_size = n / size;
            int local_start = i * local_size;
            int local_end = local_start + local_size;

            if (i == size - 1)
            {
                local_end += n % size;
            }

            MPI_Send(&vIn[local_start], local_end - local_start, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        // Recebe o vetor de entrada do processo mestre
        MPI_Recv(vIn, n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    struct timeval tIni, tFim;
    if (rank == 0)
    {
        gettimeofday(&tIni, 0);
    }

    // Cada processo calcula a soma de sufixos localmente
    soma_sufixos(n, vIn, vOut, rank, size);

    if (rank == 0)
    {
        gettimeofday(&tFim, 0);
        long segundos = tFim.tv_sec - tIni.tv_sec;
        long microsegundos = tFim.tv_usec - tIni.tv_usec;
        double tempo = (segundos * 1e3) + (microsegundos * 1e-3);
        printf("Tempo=%.2f milissegundos\n", tempo);

        // Processo mestre escreve dados no arquivo de saída e libera vetores de entrada e saída
        finaliza(n, vIn, vOut, argv[2]);
    }
    else
    {
        // Processos não mestres enviam o resultado para o processo mestre
        MPI_Send(vOut, n, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    free(vIn);
    free(vOut);

    MPI_Finalize();

    return 0;
}