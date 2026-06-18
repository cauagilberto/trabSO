#pragma once
#pragma comment(lib, "pthreadVC2.lib")
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define HAVE_STRUCT_TIMESPEC 

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define TAM_LINHAS 5
#define TAM_COLUNAS 5

#define THREADS_DISP 4

#define SEED 43

#define LIMITE_NUMB 32000

#define TAM_MACROBLOCO (100*100)

#define MODO_EXECUCAO 1 // 0 - Serial, 1 - Paralelo

int CONT_PRIMOS_SERIAL = 0;
int CONT_PRIMOS_PARALELO = 0;

double TEMPO_SERIAL;

long long CONTADOR_PRIMO_TOTAL = 0;

int MACROBLOCO_ATUAL = 0;

int TOTAL_MACRO;
int ELEMENTOS;

//struct para passar paramêtros globais
typedef struct {
	int id;
	int **matriz;
	int linhas, colunas;
	int macro_linhas, macro_colunas;
	int total_macroblocos;
	int *macrobloco_atual;
	int *contador_primos;
	pthread_mutex_t *mutex_compartilhado;
	pthread_mutex_t *mutex_indice;
} thread_parametros;

int **aloca_grade_dados(int linhas, int colunas) {
	//aloca espaço para as linhas
	int **matriz = (int **)malloc(linhas * sizeof(int *));
	if (matriz == NULL) {
		fprintf(stderr, "Erro ao alocar memória para as linhas da matriz.\n");
		exit(1);
		}
	for (int i = 0; i < linhas; i++) {
		//aloca espaço para as colunas
		(matriz)[i] = (int *)malloc(colunas * sizeof(int));
		if (matriz[i] == NULL) {
			fprintf(stderr, "Erro ao alocar memória para as linhas da matriz.\n");
			exit(1);
		}	
		
	}
	srand(SEED);

	for (int k = 0; k < linhas; k++) {
		for (int l = 0; l < colunas; l++) {
			(matriz)[k][l] = rand() % LIMITE_NUMB;
		}
	}

	return matriz;
}

void libera_memoria(int **matriz, int linhas) {

	for (int i = 0; i < TAM_LINHAS; i++) {
		free(matriz[i]);
	}
	free(matriz);
}

int is_primo(int num) {
	//numeros menores ou iguais a 1 não são primos
	if (num <= 1) return 0;
	//2 é o único número par primo
	if (num == 2) return 1;
	//nenhum outro número par é primo
	if (num % 2 == 0) return 0;

	//se o numero tiver uma raiz inteira então precisamos testar somente ao resultado da raiz para saber se tem divisor ou não
	int raiz = (int)sqrt((double)num);
	for (int i = 3; i <= raiz; i += 2) {//adiciona 2 pra pular par
		if (num % i == 0) return 0;//verifica se tem divisor
	}

	//não tem divisor é primo
	return 1;
}

void teste() {
	int **matriz = aloca_grade_dados(TAM_LINHAS, TAM_COLUNAS);
	
	printf("Matriz gerada:\n");
	for (int i = 0; i < TAM_LINHAS; i++) {
		for (int j = 0; j < TAM_COLUNAS; j++) {
			printf("%d ", matriz[i][j]);
		}
		printf("\n");
	}

	for (int i = 0; i < TAM_LINHAS; i++) {
		for (int j = 0; j < TAM_COLUNAS; j++) {
			if (is_primo(matriz[i][j])) {
				CONTADOR_PRIMO_TOTAL++;
			}
		}
	}

	printf("%lld\n", CONTADOR_PRIMO_TOTAL);
	
	libera_memoria(matriz, TAM_LINHAS);

}


int main() {
	printf("olá, mundo@");

	teste();

	return 0;
}
