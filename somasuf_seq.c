// ----------------------------------------------------------------------------
// Soma de sufixos sequencial
// Para compilar: gcc somasuf_seq.c -o somasuf_seq -Wall
// Para executar: ./somasuf_seq arquivo_entrada arquivo_saida

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// ----------------------------------------------------------------------------
void inicializa(int *n, int **vIn, int **vOut, char *nomeArqIn)
{
	// Abre arquivo texto de entrada
	FILE *arqIn = fopen(nomeArqIn, "rt") ;

	if (arqIn == NULL)
	{
		printf("\nArquivo texto de entrada não encontrado\n") ;
		exit(1) ;
	}

	// Lê tamanho do vetor de entrada
	fscanf(arqIn, "%d", n) ;

	// Aloca vetores de entrada e saída
	*vIn = (int *) malloc((*n) * sizeof(int));
	if (*vIn == NULL)
	{
		printf("\nErro na alocação de estruturas\n") ;
		exit(1) ;
	}
	*vOut = (int *) malloc((*n) * sizeof(int));
	if (*vOut == NULL)
	{
		printf("\nErro na alocação de estruturas\n") ;
		exit(1) ;
	}

	// Lê vetor do arquivo de entrada
	for (int i = 0; i < (*n); i++)
	{
		fscanf(arqIn, "%d", &((*vIn)[i]));
	}

	// Fecha arquivo de entrada
	fclose(arqIn) ;
}
// ----------------------------------------------------------------------------
void finaliza(int n, int *vIn, int *vOut, char *nomeArqOut)
{
	// Cria arquivo texto de saída
	FILE *arqOut = fopen(nomeArqOut, "wt") ;

	// Escreve tamanho do vetor de saída
	fprintf(arqOut, "%d\n", n) ;

	// Escreve vetor do arquivo de saída
	for (int i = 0; i < n; i++)
	{
		fprintf(arqOut, "%d ", vOut[i]);
	}
	fprintf(arqOut, "\n");

	// Fecha arquivo de saída
	fclose(arqOut) ;

	// Libera vetores de entrada e saída
	free(vIn);
	free(vOut);
}
// ----------------------------------------------------------------------------
void soma_sufixos(int n, int *vIn, int *vOut)
{
	vOut[n-1] = vIn[n-1];
	for (int i = n - 2; i >= 0; i--)
	{
		vOut[i] = vOut[i+1] + vIn[i];
	}
}
// ----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int n,		// Tamanho dos vetores de entrada e saída
		 *vIn,	// Vetor de entrada de tamanho n
		 *vOut;	// Vetor de saída de tamanho n

	if(argc != 3)
	{
		printf("O programa foi executado com argumentos incorretos.\n") ;
		printf("Uso: ./somasuf_seq arquivo_entrada arquivo_saida\n") ;
		exit(1) ;
	}

	// Aloca vetores de entrada e saída, lê dados do arquivo de entrada
	inicializa(&n, &vIn, &vOut, argv[1]);

	struct timeval tIni, tFim;
	gettimeofday(&tIni, 0);

	// Calcula soma de sufixos
	soma_sufixos(n, vIn, vOut);

	gettimeofday(&tFim, 0);
	// Calcula tempo de execução em milissegundos
	long segundos = tFim.tv_sec - tIni.tv_sec;
	long microsegundos = tFim.tv_usec - tIni.tv_usec;
	double tempo = (segundos * 1e3) + (microsegundos * 1e-3);
	printf("Tempo=%.2f milissegundos\n", tempo);

	// Escreve dados no arquivo de saída, libera vetores de entrada e saída
	finaliza(n, vIn, vOut, argv[2]);

	return 0;
}
// ----------------------------------------------------------------------------