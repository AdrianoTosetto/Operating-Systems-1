#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

unsigned int nthreads = 30;
#define SIZE 9

int load_grid(int grid[][SIZE], char *filename);
int grid[9][9];
int reorgGrid[9][9];
int nextCell = 0;
int nextReorgCell = 0;
int erros = 0;
pthread_t tidpai;

pthread_mutex_t mutexCell, mutexReorg, mutexErr;
pthread_barrier_t trump;

void checkCol(int cell) {
	int col = cell % 9;
	int row = cell / 9;
	
	//pthread_t tid = pthread_self();
	//printf("coluna %d e thread %u\n", col + 1, (unsigned int)tid);
	
	for (int i = row + 1; i < SIZE; i++) {
		if (grid[row][col] == grid[i][col]) {
			printf("Erro na coluna %d!\n", col + 1);
			
			pthread_mutex_lock(&mutexErr);
			erros++;
			pthread_mutex_unlock(&mutexErr);
			return;
		}
	}
}

void checkRow(int cell) {
	int col = cell % 9;
	int row = cell / 9;
	
	//pthread_t tid = pthread_self();
	//printf("linha %d e thread %u\n", row + 1, (unsigned int)tid);
	
	for (int i = col + 1; i < SIZE; i++) {
		if (grid[row][col] == grid[row][i]) {
			printf("Erro na linha %d!\n", row + 1);
			
			pthread_mutex_lock(&mutexErr);
			erros++;
			pthread_mutex_unlock(&mutexErr);
			return;
		}
	}
}

void checkReg(int cell) {
	 
	int row = (cell / 27) * 3 + (cell / 3) % 3;
	//printf("%d\n", row);
	int col = (cell % 3) + ((cell % 27) / 9) * 3;
	
	//pthread_t tid = pthread_self();
	//printf("região %d e thread %u\n", row + 1, (unsigned int)tid);
	
	for (int i = col + 1; i < SIZE; i++) {
		//printf("%d ", reorgGrid[row][i]);
		if (reorgGrid[row][col] == reorgGrid[row][i]) {
			printf("Erro na região %d!\n", row + 1);
			
			pthread_mutex_lock(&mutexErr);
			erros++;
			pthread_mutex_unlock(&mutexErr);
			return;
		}
	}
	//printf("\n");
}

void reorganizeRegions(int cell) {
	int rowNew = (cell / 27) * 3 + (cell / 3) % 3;
	int colNew = (cell % 3) + ((cell % 27) / 9) * 3;
	
	int rowOld = cell / 9;
	int colOld = cell % 9;
	
	reorgGrid[rowNew][colNew] = grid[rowOld][colOld];
}

void *check(void* arg) {
	int cell;
	
	while (true) {
		pthread_mutex_lock(&mutexReorg);
		if (nextReorgCell > 81) {
			pthread_mutex_unlock(&mutexReorg);
			break;
		}
		int reorgCell = nextReorgCell;
		nextReorgCell++;
		pthread_mutex_unlock(&mutexReorg);
		
		reorganizeRegions(reorgCell);
	}
	
	pthread_barrier_wait(&trump);
	
	
	
	pthread_mutex_lock(&mutexCell);
	
	while (nextCell < 242) {
		switch (nextCell % 3) {
			case 0:
				cell = nextCell / 3;
				nextCell++;
				pthread_mutex_unlock(&mutexCell);
				checkCol(cell);				
				break;
			case 1:
				cell = nextCell / 3;
				nextCell++;
				pthread_mutex_unlock(&mutexCell);
				checkRow(cell);
				break;
			case 2:
				cell = nextCell / 3;
				nextCell++;
				pthread_mutex_unlock(&mutexCell);
				checkReg(cell);
				break;
		}
		pthread_mutex_lock(&mutexCell);
	}
	pthread_mutex_unlock(&mutexCell);
	return NULL;
}

int main(int argc, char **argv) {
	
	if(argc != 3) {
		printf("Usage: ./t1_v1 file.txt number_threads\n");
		exit(0);
	}

	char *gridFile = argv[1];
	nthreads   = atoi(argv[2]);
	
	tidpai = pthread_self();
	
	//nextRow = nextCell / 9;
	//nextReg = (nextCell / 27) * 3 + (nextCell / 9) / 3;
	
	if(nthreads > 80) nthreads = 80;
	
	load_grid(grid, gridFile);
	
	pthread_mutex_init(&mutexCell, NULL);
	pthread_mutex_init(&mutexReorg, NULL);
	pthread_mutex_init(&mutexErr, NULL);
	pthread_barrier_init(&trump, NULL, nthreads);
	
	pthread_t thread[nthreads];
	for (int i = 0; i < nthreads; i++) {
		pthread_create(&thread[i], NULL, check, NULL);
	}
	
	for (int i = 0; i < nthreads; i++) {
		pthread_t t = thread[i];
		pthread_join(t, NULL);
	}
	
	printf("%d erros.\n", erros);
	
	pthread_barrier_destroy(&trump);
	pthread_mutex_destroy(&mutexCell);
	pthread_mutex_destroy(&mutexReorg);
	pthread_mutex_destroy(&mutexErr);
	
	/*for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			printf("%d ", reorgGrid[i][j]); 
		}
		printf("\n");  
	}*/
	
	//printf("\n");
	
	/*for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			printf("%d", grid[i][j]); 
		}
		printf("\n"); 
	}*/
	
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
