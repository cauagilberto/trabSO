#pragma once
#pragma comment(lib, "libpthread.lib")
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#define HAVE_STRUCT_TIMESPEC 

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define TAM_LINHAS 20000
#define TAM_COLUNAS 20000

#define THREADS_DISP 512

#define SEED 42

#define LIMITE_NUMB 32000

#define MACROBLOCO_COLUNAS 1000
#define MACROBLOCO_LINHAS 1000

int CONT_PRIMOS_SERIAL = 0;
int CONT_PRIMOS_PARALELO = 0;

double TEMPO_SERIAL;
double TEMPO_PARALELO;

int total_macro = 0;
int proximo_macrobloco = 0;

int **matriz;
int total_macroblocos;
int macrobloco_atual;
pthread_mutex_t mutex_macrobloco;
pthread_mutex_t mutex_indice;


int **gera_aloca_matriz(int linhas, int colunas) {
	//aloca espaço para as linhas
	matriz = (int**)malloc(linhas * sizeof(int*));
	if (matriz == NULL) {
		//verifica se o espaço foi alocado
		fprintf(stderr, "Erro ao alocar memória para as linhas da matriz.\n");
		exit(1);
		}
	for (int i = 0; i < linhas; i++) {
		//aloca espaço para as colunas
		matriz[i] = (int *)malloc(colunas * sizeof(int));
		if (matriz[i] == NULL) {
			//verifica se o espaço foi alocado 
			fprintf(stderr, "Erro ao alocar memória para as linhas da matriz.\n");
			exit(1);
		}	
		
	}
	srand(SEED);

	//aloca os numeros aleatórios para a matriz
	for (int j = 0; j < linhas; j++) {
		for (int k = 0; k < colunas; k++) {
			(matriz)[j][k] = rand() % LIMITE_NUMB;
		}
	}

	return matriz;
}

void libera_memoria(int linhas) {

	if (matriz == NULL) return;
	for (int i = 0; i < linhas; i++) {
		if (matriz[i] != NULL) {
			free(matriz[i]);
		}
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

void *trabalho_da_thread() {
	int macrobloco_atual;
	long long primos_locais = 0;
	
	//int primos_locais = 0;

	int quantidade_macroblocos = ((TAM_COLUNAS + MACROBLOCO_COLUNAS - 1)/ MACROBLOCO_COLUNAS); //ex: 10000/100 = 100 macroblocos por linha

	while (1) {

		pthread_mutex_lock(&mutex_macrobloco); //trava o mutex para acessar o macrobloco atual
		if (proximo_macrobloco >= total_macro) {
			pthread_mutex_unlock(&mutex_macrobloco); //destrava o mutex 
			break;
		}
		macrobloco_atual = proximo_macrobloco; //deixa o macro atual pra thread
		proximo_macrobloco++; //disponibiliza o proxmio macro pra outra thread

		pthread_mutex_unlock(&mutex_macrobloco);

		int linha_inicial = (macrobloco_atual / quantidade_macroblocos) * MACROBLOCO_LINHAS; //calcula a linha inicial do macrobloco
		int linha_final = linha_inicial + MACROBLOCO_LINHAS; //calcula a linha final do macrobloco
		if (linha_final > TAM_LINHAS) linha_final = TAM_LINHAS; //verifica se ta na borda da matriz
		
		int coluna_inicial = (macrobloco_atual % quantidade_macroblocos) * MACROBLOCO_COLUNAS; //calcula a coluna inicial do macrobloco
		int coluna_final = coluna_inicial + MACROBLOCO_COLUNAS; //calcula a linha coluna do macrobloco
		if (coluna_final > TAM_COLUNAS) coluna_final = TAM_COLUNAS; //verifica se ta na borda da matriz

		for (int i = linha_inicial; i < linha_final; i++) {
			for (int j = coluna_inicial; j < coluna_final; j++) { //percorre o macrobloco
				if(is_primo(matriz[i][j])){
					primos_locais++; 
				}
			}
		}
	}
	pthread_mutex_lock(&mutex_indice);//trava o mutex para acessar o macrobloco atual
	CONT_PRIMOS_PARALELO+=primos_locais; //incrementa o contador de primos paralelo
	pthread_mutex_unlock(&mutex_indice);//destrava o mutex 

	return NULL;
}

void busca_paralela() {

	pthread_mutex_init(&mutex_macrobloco, NULL); //inicializa o mutex do macrobloco
	pthread_mutex_init(&mutex_indice, NULL); //inicializa o mutex do contador de primos paralelo

	pthread_t threads[THREADS_DISP]; //declara as threads
	int threads_criadas = 0;

	total_macro = (TAM_LINHAS / MACROBLOCO_LINHAS) * (TAM_COLUNAS / MACROBLOCO_COLUNAS); //calcula a quantidade total de macroblocos

	clock_t inicio = clock(); 

	for (int i = 0; i < THREADS_DISP; i++) {
		int contador_threads = pthread_create(&threads[i], NULL, trabalho_da_thread, NULL); //cria as threads e aloca a variavel para verificação

		if (contador_threads != 0) break;

		threads_criadas++;
	}

	for (int i = 0; i < threads_criadas; i++) {
		pthread_join(threads[i], NULL); //espera as threads terminarem
	}

	clock_t fim = clock();

	TEMPO_PARALELO = (double)(fim - inicio)/CLOCKS_PER_SEC;

	pthread_mutex_destroy(&mutex_macrobloco); //destroi o mutex do macrobloco
	pthread_mutex_destroy(&mutex_indice); //destroi o mutex do contador de primos paralelo

}

void busca_serial() {
	clock_t inicio = clock();
	for (int i = 0; i < TAM_LINHAS; i++) {
		for (int j = 0; j < TAM_COLUNAS; j++) {
			if(is_primo(matriz[i][j])){
				CONT_PRIMOS_SERIAL++;
			}
		}
	}
	clock_t fim = clock();
	TEMPO_SERIAL = (double)(fim - inicio)/CLOCKS_PER_SEC;
}

int main() {
	int escolha;

	printf("Escolha o qual método deseja encontrar os primos em uma matriz: \n");
	printf("1. Busca Paralela\n");
	printf("2. Busca Serial\n");
	printf("3. Ambos e o speedup\n");
	printf("Digite a opção desejada: ");
	scanf("%d", &escolha);


	//printf("olá, mundo@");
	gera_aloca_matriz(TAM_LINHAS, TAM_COLUNAS);
	
	if (escolha == 1 || escolha == 3) {
		busca_paralela();
		printf("Tempo gasto para busca paralela: %lf segundos\n", TEMPO_PARALELO);
		printf("Quantidade macroblocos: %d\n", total_macro);
		printf("Quantidade de primos encontrados: %d\n", CONT_PRIMOS_PARALELO);
	}
	
	if (escolha == 2 || escolha == 3) {
		busca_serial();
		printf("Tempo gasto para busca serial: %lf segundos\n", TEMPO_SERIAL);
		printf("Quantidade de primos encontrados: %d\n", CONT_PRIMOS_SERIAL);

		if (escolha == 3) {
			double speedup = TEMPO_SERIAL / TEMPO_PARALELO;
			printf("Speedup: %lf\n", speedup);
		}
	}
	
	libera_memoria(TAM_LINHAS);

	return 0;
}
