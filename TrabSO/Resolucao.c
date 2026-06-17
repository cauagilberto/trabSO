#pragma once
#pragma comment(lib, "pthreadVC2.lib")
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATE_NO_WARNINGS 1
#define HAVE_STRUCT_TIMESPEC 

#include <ptherad.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define TAM_LINHAS 10000
#define TAM_COLUNAS 10000

#define THREADS_DISP 4

#define SEED 42

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
	pthread_mutex_t* mutex_indice;
} thread_parametros;

int main() {

}