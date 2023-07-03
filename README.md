Os requisitos do código são:

1. Cada processo (inclusive a raiz e exceto o último) soma o valor recebido com sua parte dos p-1 processos em paralelo, sequencialmente dentro de cada processo. Isso é feito na função `soma_sufixos()` utilizando as operações MPI `MPI_Allgather()` e um loop para calcular a soma acumulada.

2. O processo raiz distribui os valores pelos processos utilizando a função `MPI_Gather()`, que coleta os resultados individuais de cada processo em um único array no processo raiz.

3. No segundo nível, o processo raiz calcula a soma dos sufixos dos primeiros elementos sequencialmente dentro do próprio processo raiz. Essa soma é realizada na função `soma_sufixos()`.

4. O processo raiz coleta o primeiro elemento calculado por cada processo através da função `MPI_Gather()` e armazena os resultados individuais em um array no processo raiz.

5. Cada processo, incluindo a raiz, calcula a soma dos sufixos da sua parte com os p processos em paralelo e sequencialmente dentro de cada processo. Isso é feito na função `soma_sufixos()`.

6. O vetor de entrada é dividido em p partes de tamanho nLocal = n/p, onde n é o tamanho total do vetor de entrada, e o processo raiz distribui essas partes do vetor de entrada pelos processos usando a função `MPI_Gather()`.

7. O processo raiz coleta as partes do vetor de saída dos processos utilizando a função `MPI_Gather()` e armazena os resultados individuais em um array no processo raiz.
