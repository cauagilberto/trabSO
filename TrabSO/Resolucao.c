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

#define SEED 42
#define LIMITE_NUMB 32000

// Variáveis de configuração dinâmicas
int tam_linhas = 30000;
int tam_colunas = 30000;
int num_threads = 16;
int macro_linhas = 1000;
int macro_colunas = 1000;
int usa_mutex = 1; // 1 = Com Mutex, 0 = Sem Mutex

int CONT_PRIMOS_SERIAL = 0;
int CONT_PRIMOS_PARALELO = 0;

double TEMPO_SERIAL;
double TEMPO_PARALELO;

int total_macro = 0;
int proximo_macrobloco = 0;

int** matriz = NULL;
pthread_mutex_t mutex_macrobloco;
pthread_mutex_t mutex_indice;


int** gera_aloca_matriz(int linhas, int colunas) {
	matriz = (int**)malloc(linhas * sizeof(int*));
	if (matriz == NULL) {
		fprintf(stderr, "Erro ao alocar memoria para as linhas da matriz.\n");
		exit(1);
	}
	for (int i = 0; i < linhas; i++) {
		matriz[i] = (int*)malloc(colunas * sizeof(int));
		if (matriz[i] == NULL) {
			fprintf(stderr, "Erro ao alocar memoria para as colunas da matriz.\n");
			exit(1);
		}
	}

	srand(SEED);
	for (int j = 0; j < linhas; j++) {
		for (int k = 0; k < colunas; k++) {
			matriz[j][k] = rand() % LIMITE_NUMB;
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
	matriz = NULL;
}

int is_primo(int num) {
	if (num <= 1) return 0;
	if (num == 2) return 1;
	if (num % 2 == 0) return 0;

	int raiz = (int)sqrt((double)num);
	for (int i = 3; i <= raiz; i += 2) {
		if (num % i == 0) return 0;
	}
	return 1;
}

void* trabalho_da_thread() {
	int macrobloco_atual;
	long long primos_locais = 0;

	int quantidade_macroblocos = (tam_colunas + macro_colunas - 1) / macro_colunas;

	while (1) {
		if (usa_mutex) pthread_mutex_lock(&mutex_macrobloco);

		if (proximo_macrobloco >= total_macro) {
			if (usa_mutex) pthread_mutex_unlock(&mutex_macrobloco);
			break;
		}
		macrobloco_atual = proximo_macrobloco;
		proximo_macrobloco++;

		if (usa_mutex) pthread_mutex_unlock(&mutex_macrobloco);

		int linha_inicial = (macrobloco_atual / quantidade_macroblocos) * macro_linhas;
		int linha_final = linha_inicial + macro_linhas;
		if (linha_final > tam_linhas) linha_final = tam_linhas;

		int coluna_inicial = (macrobloco_atual % quantidade_macroblocos) * macro_colunas;
		int coluna_final = coluna_inicial + macro_colunas;
		if (coluna_final > tam_colunas) coluna_final = tam_colunas;

		for (int i = linha_inicial; i < linha_final; i++) {
			for (int j = coluna_inicial; j < coluna_final; j++) {
				if (is_primo(matriz[i][j])) {
					primos_locais++;
				}
			}
		}
	}

	if (usa_mutex) {
		pthread_mutex_lock(&mutex_indice);
		CONT_PRIMOS_PARALELO += primos_locais;
		pthread_mutex_unlock(&mutex_indice);
	}
	else {
		// Sem proteção: causará as condições de corrida simuladas no seu relatório
		CONT_PRIMOS_PARALELO += primos_locais;
	}

	return NULL;
}

void busca_paralela() {
	CONT_PRIMOS_PARALELO = 0;
	proximo_macrobloco = 0;

	if (usa_mutex) {
		pthread_mutex_init(&mutex_macrobloco, NULL);
		pthread_mutex_init(&mutex_indice, NULL);
	}

	pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
	int threads_criadas = 0;

	int macro_linhas_qnt = (tam_linhas + macro_linhas - 1) / macro_linhas;
	int macro_colunas_qnt = (tam_colunas + macro_colunas - 1) / macro_colunas;
	total_macro = macro_linhas_qnt * macro_colunas_qnt;

	clock_t inicio = clock();

	for (int i = 0; i < num_threads; i++) {
		if (pthread_create(&threads[i], NULL, trabalho_da_thread, NULL) != 0) break;
		threads_criadas++;
	}

	for (int i = 0; i < threads_criadas; i++) {
		pthread_join(threads[i], NULL);
	}

	clock_t fim = clock();
	TEMPO_PARALELO = (double)(fim - inicio) / CLOCKS_PER_SEC;

	if (usa_mutex) {
		pthread_mutex_destroy(&mutex_macrobloco);
		pthread_mutex_destroy(&mutex_indice);
	}
	free(threads);
}

void busca_serial() {
	CONT_PRIMOS_SERIAL = 0;
	clock_t inicio = clock();
	for (int i = 0; i < tam_linhas; i++) {
		for (int j = 0; j < tam_colunas; j++) {
			if (is_primo(matriz[i][j])) {
				CONT_PRIMOS_SERIAL++;
			}
		}
	}
	clock_t fim = clock();
	TEMPO_SERIAL = (double)(fim - inicio) / CLOCKS_PER_SEC;
}

// BATERIA 1: Matrizes 30000x30000 Padrão (Com Mutex)
void rodar_bateria_padrao_30k() {
	tam_linhas = 30000; tam_colunas = 30000; usa_mutex = 1;
	int threads_test[] = { 16, 8, 4, 2 };
	int macro_test[] = { 10000, 1000, 100, 10 };
	int n_threads = sizeof(threads_test) / sizeof(threads_test[0]);
	int n_macros = sizeof(macro_test) / sizeof(macro_test[0]);

	printf("MATRIZES 30.000X30.000 (Com Mutex):\n");
	gera_aloca_matriz(tam_linhas, tam_colunas);
	busca_serial(); // Executa uma vez para a matriz 30k

	for (int m = 0; m < n_macros; m++) {
		macro_linhas = macro_test[m]; macro_colunas = macro_test[m];
		printf("- macrobloco %dx%d:\n", macro_linhas, macro_colunas);
		for (int t = 0; t < n_threads; t++) {
			num_threads = threads_test[t];
			busca_paralela();
			printf("\t- %d threads\n", num_threads);
			printf("\t\tTempo gasto para busca paralela: %lf segundos\n", TEMPO_PARALELO);
			printf("\t\tQuantidade macroblocos: %d\n", total_macro);
			printf("\t\tQuantidade de primos encontrados: %d\n", CONT_PRIMOS_PARALELO);
			printf("\t\tTempo gasto para busca serial: %lf segundos\n", TEMPO_SERIAL);
			printf("\t\tQuantidade de primos encontrados: %d\n", CONT_PRIMOS_SERIAL);
			printf("\t\tSpeedup: %lf\n", TEMPO_SERIAL / TEMPO_PARALELO);
		}
	}
	libera_memoria(tam_linhas);
}

// BATERIA 2: Matrizes 30000x30000 Sem Mutex
void rodar_bateria_sem_mutex_30k() {
	tam_linhas = 30000; tam_colunas = 30000; usa_mutex = 0; num_threads = 8;
	int macro_test[] = { 3000, 300, 100 };
	int n_macros = sizeof(macro_test) / sizeof(macro_test[0]);

	printf("\n- **sem mutexes:**\n\t- 8 threads:\n");
	gera_aloca_matriz(tam_linhas, tam_colunas);
	busca_serial();

	for (int m = 0; m < n_macros; m++) {
		macro_linhas = macro_test[m]; macro_colunas = macro_test[m];
		busca_paralela();
		printf("\t\t- macrobloco %dx%d:\n", macro_linhas, macro_colunas);
		printf("\t\t\tTempo gasto para busca paralela: %lf segundos\n", TEMPO_PARALELO);
		printf("\t\t\tQuantidade macroblocos: %d\n", total_macro);
		printf("\t\t\tQuantidade de primos encontrados: %d\n", CONT_PRIMOS_PARALELO); // Aqui o bloco 100x100 vai divergir
		printf("\t\t\tTempo gasto para busca serial: %lf segundos\n", TEMPO_SERIAL);
		printf("\t\t\tQuantidade de primos encontrados: %d\n", CONT_PRIMOS_SERIAL);
		printf("\t\t\tSpeedup: %lf\n", TEMPO_SERIAL / TEMPO_PARALELO);
	}
	libera_memoria(tam_linhas);
}

// BATERIA 3: Matrizes 20000x20000 - Teste de Overhead (x64 ou x86 dependendo de onde compilar)
void rodar_bateria_overhead_20k() {
	// Configura a matriz menor para evitar estouro de memória no x86
	tam_linhas = 20000;
	tam_colunas = 20000;
	usa_mutex = 1;
	macro_linhas = 1000;
	macro_colunas = 1000;

	// Array com as quantidades específicas de threads do seu relatório
	int threads_test[] = { 8, 16, 32, 64, 128, 512, 1024 };
	int n_threads = sizeof(threads_test) / sizeof(threads_test[0]);

	// Detecta automaticamente o modo de compilação da IDE para rotular o print
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
	printf("\nMATRIZES 20.000X20.000 (X64):\n- overhead(x64):\n");
#else
	printf("\nMATRIZES 20.000X20.000 (X86):\n- overhead(x86):\n");
#endif

	printf("\t- macrobloco %dx%d:\n", macro_linhas, macro_colunas);

	gera_aloca_matriz(tam_linhas, tam_colunas);

	// Busca serial de base (roda apenas uma vez para calcular o speedup)
	busca_serial();

	// Loop que varre o array iterando pelas 8, 16, 32... 1024 threads
	for (int t = 0; t < n_threads; t++) {
		num_threads = threads_test[t];
		busca_paralela();

		printf("\t\t- %d threads:\n", num_threads);
		printf("\t\t\tTempo gasto para busca paralela: %lf segundos\n", TEMPO_PARALELO);
		printf("\t\t\tQuantidade macroblocos: %d\n", total_macro);
		printf("\t\t\tQuantidade de primos encontrados: %d\n", CONT_PRIMOS_PARALELO);
		printf("\t\t\tTempo gasto para busca serial: %lf segundos\n", TEMPO_SERIAL);
		printf("\t\t\tQuantidade de primos encontrados: %d\n", CONT_PRIMOS_SERIAL);
		printf("\t\t\tSpeedup: %lf\n", TEMPO_SERIAL / TEMPO_PARALELO);
	}
	libera_memoria(tam_linhas);
}

int main() {
	int escolha;

	printf("Escolha o cenario de testes:\n");
	printf("1. Rodar APENAS a bateria padrao (Matriz 30k com Mutex)\n");
	printf("2. Rodar APENAS o teste Sem Mutex (Matriz 30k)\n");
	printf("3. Rodar APENAS o teste de Overhead de Threads (Matriz 20k)\n");
	printf("4. RODAR TUDO AUTOMATICAMENTE (Gera os dados completos do relatorio)\n");
	printf("Digite a opcao: ");
	scanf("%d", &escolha);
	printf("\nIniciando testes...\n\n");

	switch (escolha) {
	case 1:
		rodar_bateria_padrao_30k();
		break;
	case 2:
		rodar_bateria_sem_mutex_30k();
		break;
	case 3:
		rodar_bateria_overhead_20k();
		break;
	case 4:
		rodar_bateria_padrao_30k();
		rodar_bateria_sem_mutex_30k();
		rodar_bateria_overhead_20k();
		break;
	default:
		printf("Opcao invalida.\n");
	}

	printf("\n[Fim dos testes coordenados]\n");
	return 0;
}