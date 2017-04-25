#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

unsigned int nthreads = 30;
#define SIZE 9

int load_grid(int grid[][SIZE], char *filename);
int grid[9][9];
int erros = 0;
int nextTask = 0;


pthread_mutex_t mutexErr, mutexTask;

void checkCol(int col) {
	//pthread_t tid = pthread_self();
	//printf("coluna %d e thread %u\n", col + 1, (unsigned int)tid);
	
	for (int i = 0; i < SIZE; i++) {
		for (int j = i + 1; j < SIZE; j++) {
			if (grid[i][col] == grid[j][col]) {
				printf("Erro na coluna %d!\n", col + 1);
				
				pthread_mutex_lock(&mutexErr);
				erros++;
				pthread_mutex_unlock(&mutexErr);
				return;
			}
		}
	}
}

void checkRow(int row) {
	//pthread_t tid = pthread_self();
	//printf("linha %d e thread %u\n", row + 1, (unsigned int)tid);
	
	for (int i = 0; i < SIZE; i++) {
		for (int j = i + 1; j < SIZE; j++) {
			if (grid[row][i] == grid[row][j]) {
				printf("Erro na linha %d!\n", row + 1);
				
				pthread_mutex_lock(&mutexErr);
				erros++;
				pthread_mutex_unlock(&mutexErr);
				return;
			}
		}
	}
}

void checkReg(int reg) {
	//pthread_t tid = pthread_self();
	//printf("região %d e thread %u\n", reg + 1, (unsigned int)tid);
	
	int rowBase = (reg / 3) * 3;
	int rowLimit = ((reg + 3)/ 3) * 3;
	int colBase = (reg % 3) * 3;
	int colLimit = ((reg % 3) + 1) * 3;
	int regVector[SIZE]; 
	int k = -1;
	
	for (int i = rowBase; i < rowLimit; i++) {
		for (int j = colBase; j < colLimit; j++) {
			regVector[++k] = grid[i][j];
		}
	}
	
	for (int i = 0; i < SIZE; i++) {
		for (int j = i + 1; j < SIZE; j++) {
			if (regVector[i] == regVector[j]) {
				printf("Erro na região %d!\n", reg + 1);
				
				pthread_mutex_lock(&mutexErr);
				erros++;
				pthread_mutex_unlock(&mutexErr);
				return;
			}
		}
	}
}

void *check(void* arg) {	
	
	pthread_mutex_lock(&mutexTask);
	
	while (nextTask < 27) {
		
		int task;
		switch ((nextTask / 9)) {
			case 0:
				task = nextTask % 9;
				nextTask++;
				pthread_mutex_unlock(&mutexTask);
				checkCol(task);				
				break;
			case 1:
				task = nextTask % 9;
				nextTask++;
				pthread_mutex_unlock(&mutexTask);
				checkRow(task);
				break;
			case 2:
				task = nextTask % 9;
				nextTask++;
				pthread_mutex_unlock(&mutexTask);
				checkReg(task);
				break;
		}
		pthread_mutex_lock(&mutexTask);
	}
	pthread_mutex_unlock(&mutexTask);
	return NULL;
}

int main(int argc, char **argv) {

	if(argc != 3) {
		printf("Usage: ./t1_v1 file.txt number_threads\n");
		exit(0);
	}

	char *gridFile = argv[1];
	nthreads   = atoi(argv[2]);

	if(nthreads > 27) nthreads = 27;
	
	load_grid(grid, gridFile);
	
	//pthread_mutex_init(&mutexRow, NULL);
	//pthread_mutex_init(&mutexCol, NULL);
	//pthread_mutex_init(&mutexReg, NULL);
	pthread_mutex_init(&mutexErr, NULL);
	pthread_mutex_init(&mutexTask, NULL);
	
	pthread_t thread[nthreads];
	for (int i = 0; i < nthreads; i++) {
		pthread_create(&thread[i], NULL, check, NULL);
	}
	
	for (int i = 0; i < nthreads; i++) {
		pthread_t t = thread[i];
		pthread_join(t, NULL);
	}
	
	printf("%d erros.\n", erros);
	
	pthread_mutex_destroy(&mutexErr);
	pthread_mutex_destroy(&mutexTask);
	
	pthread_exit(NULL);
}




/* Funcao que le um grid do arquivo "filename" e o armazena em uma matriz */
int load_grid(int grid[][SIZE], char *filename) {
	FILE *input_file = fopen(filename, "r");

	if (input_file != NULL) {
		for(int i = 0; i < SIZE; i++)
			for(int j = 0; j < SIZE; j++)
				fscanf(input_file, "%d", &grid[i][j]);
		fclose(input_file);
		return 1;
	}
	return 0;
}
