
// Soma de sufixos utilizando MPI 
// Para compilar: mpicc somasuf_mpi.c -o somasuf_mpi -Wall
// Para executar:  mpirun -oversubscribe -np 8 somasuf_mpi arquivo_entrada.txt arquivo_saida.txt
// Código ajustado para que o processo 0 seja o único que possui vetores vIn e
// vOut de tamanho n, enquanto os demais processos possuem vetores de tamanho n/p
// ----------------------------------------------------------------------------
// Nomes: Sabrina Renata Gonçalves Schimidt e
// Cláudia Magno Pereira de Brito


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

typedef struct {
  int num_total;
  int num_local;
  int *vIn;
} instancia_local_t;

instancia_local_t inicializa(char *nome_arq_in, int rank, int num_proc) {
  instancia_local_t instancia_local;
  int *buffer;

  if (rank == 0) {
    FILE *arqIn = fopen(nome_arq_in, "rt");

    if (arqIn == NULL) {
      printf("\nArquivo texto de entrada não encontrado\n");
      exit(1);
    }

    fscanf(arqIn, "%d", &instancia_local.num_total);
    instancia_local.num_local = instancia_local.num_total / num_proc;

    buffer = (int *)malloc(instancia_local.num_total * sizeof(int));
    if (buffer == NULL) {
      printf("\nErro na alocação de estruturas\n");
      exit(1);
    }

    for (int i = 0; i < instancia_local.num_total; i++) {
      fscanf(arqIn, "%d", &buffer[i]);
    }

    fclose(arqIn);
  }

  // Envia quantidade de elementos por thread e total de elementos para todas as threads
  MPI_Bcast(&instancia_local.num_total, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&instancia_local.num_local, 1, MPI_INT, 0, MPI_COMM_WORLD);

  instancia_local.vIn = (int *)malloc(instancia_local.num_local * sizeof(int));

  MPI_Scatter(buffer, instancia_local.num_local, MPI_INT, instancia_local.vIn,instancia_local.num_local, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    free(buffer);
  }

  return instancia_local;
}

void finaliza(const instancia_local_t *instancia, char *nome_arq_out, int rank,int num_proc) {

  int *buffer = NULL;

  if (rank == 0) {
    buffer = (int *)malloc(instancia->num_total * sizeof(int));
  }

  MPI_Gather(instancia->vIn, instancia->num_local, MPI_INT, buffer,instancia->num_local, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    FILE *file = fopen(nome_arq_out, "wt");

    if (file == NULL) {
      printf("Erro ao abrir arquivo de saída.\n");
      exit(1);
    }

    fprintf(file, "%d\n", instancia->num_total);
    for (int i = 0; i < instancia->num_total; i++) {
      fprintf(file, "%d ", buffer[i]);
    }
    fprintf(file, "\n");

    fclose(file);
    free(buffer);
  }

  free(instancia->vIn);
}

void primeiro_nivel(const instancia_local_t *instancia, int rank) {
  for (int i = instancia->num_local - 2; i >= 0; i--) {
    instancia->vIn[i] += instancia->vIn[i + 1];
  }
}

void segundo_nivel(const instancia_local_t *instancia, int soma_nivel2) {
  for (int i = 0; i < instancia->num_local; i++) {
    instancia->vIn[i] += soma_nivel2;
  }
}

void soma_sufixos(const instancia_local_t *instancia, int rank, int num_proc) {
  int *suffix_sums = NULL;

  primeiro_nivel(instancia, rank);

  if (rank == 0) {
    suffix_sums = (int *)malloc(num_proc * sizeof(int));
  }

  // Envia a soma de sufixos de cada processo para o processo raiz.
  MPI_Gather(&(instancia->vIn[0]), 1, MPI_INT, suffix_sums, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int i = num_proc - 2; i >= 0; i--) {
      suffix_sums[i] += suffix_sums[i + 1];
    }
  }

  int soma_nivel2;
  if (rank < num_proc - 1) {
    MPI_Scatter(&suffix_sums[rank + 1], 1, MPI_INT, &soma_nivel2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    segundo_nivel(instancia, soma_nivel2);
  }

  if (rank == 0) {
    free(suffix_sums);
  }
}

int main(int argc, char **argv) {
  int rank, num_proc;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

  if (argc != 3) {
    if (rank == 0) {
      printf("O programa foi executado com argumentos incorretos.\n");
      printf("Use: mpirun -np <num_procs> ./somasuf_mpi arquivo_entrada arquivo_saida\n");
    }

    MPI_Finalize();
    exit(1);
  }

  double start_time = MPI_Wtime();

  instancia_local_t instancia_local = inicializa(argv[1], rank, num_proc);
  soma_sufixos(&instancia_local, rank, num_proc);
  finaliza(&instancia_local, argv[2], rank, num_proc);

  double end_time = MPI_Wtime();
  double execution_time = end_time - start_time;

  if (rank == 0){
    printf("Tempo = %.2f segundos\n", execution_time);

  }

  MPI_Finalize();

  return 0;
}
