#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>
#include<sys/wait.h>

// Shared Memory
#include <sys/ipc.h>
#include <sys/shm.h>

// semaphores
#include <fcntl.h>
#include <semaphore.h>


// CONSTANTS
#define NBR_FORKS 4
#define NBR_TESTS 1000000000
#define SHM_KEY 1
#define RAM_MAX 8.5 // in GigaBytes

// GLOBAL VARIABLES
sem_t *sem ;
int shmid ;
struct SharedMemory *shared_memory ;

int sharedMemorySize = 1000000000;
int sizeOfChildStockage = 100000 ;

// Structure pour la mémoire partagée
struct SharedMemory {
	int data[NBR_TESTS] ;
};

void MemorySizeCalculations() {
	sharedMemorySize = (int)(((RAM_MAX * pow(10,9)) * 0.9)/4) ;
	sizeOfChildStockage = (int)(((RAM_MAX * pow(10,9)) * 0.1) / NBR_FORKS) ;
}


void child_process() {
	// Randomizer utilisant le pid des processus
	srand(getpid()) ;

	// Compteurs
	int localTablePointer = 0 ;
	int cptRemainingNbrs = NBR_TESTS / NBR_FORKS ;

	// Génération des Nombres Nécessaires au Remplissage Complet de la Mémoire Partagée
	while(cptRemainingNbrs != 0) {

		// Génération du Tableau Local de Nombres Aléatoires
		int* localTable = malloc(sizeOfChildStockage * sizeof(int)) ;
		int toGenerateThisRound = (cptRemainingNbrs < sizeOfChildStockage) ? cptRemainingNbrs : sizeOfChildStockage;
		int generatedThisRound = 0 ;

		while(toGenerateThisRound != 0) {
			localTable[localTablePointer] = rand() % sharedMemorySize ;

			localTablePointer++ ;
			cptRemainingNbrs-- ;
			toGenerateThisRound-- ;
			generatedThisRound++ ;
		}
		//printf("[%d] attend de rentrer dans la mémoire partagée !\n", getpid()) ;
		sem_wait(sem) ;
		//printf("[%d] est entré dans la mémoire partagée et inscris les données générées !\n", getpid()) ;
		// Ecriture dans la mémoire partagée
		for(int i = 0 ; i < generatedThisRound ; i++) {
			shared_memory->data[localTable[i]]++ ;
		}
		//printf("[%d] sort de la mémoire partagée et laisse sa place !\n", getpid()) ;
		sem_post(sem) ;

		localTablePointer = 0 ;
		free(localTable) ; // On la libère pour éviter d'avoir à réinitialiser chaque "case" à 0
	}
}

void launchGenerativeProcesses() {
	for(int i = 0 ; i < NBR_FORKS ; ++i) {
		if(fork()) {
			// in parent
			usleep(10000) ;
		} else {
			// in child
			child_process() ;
			exit(0) ;
		}
	}
	for(int j = 0 ; j < NBR_FORKS ; j++) {
		wait(NULL) ; // Continue when all sub-processes will have finished there tasks
		printf("Tout le monde a fini ! La mémoire partagée est remplie\n") ;
	}
}

int createSemaphore() {
	// Création du sémaphore
	sem = sem_open("/my_semaphore", O_CREAT, 0666, 1);

	// Vérification des erreurs
	if (sem == SEM_FAILED) {
		perror("Semaphore creation failed");
		exit(EXIT_FAILURE) ;
	}
	return EXIT_SUCCESS ;
}

int createSharedMemory() {
	// Création de la mémoire partagée
	shmid = shmget(SHM_KEY, sizeof(struct SharedMemory), IPC_CREAT | 0666);

	// Vérification des erreurs
	if (shmid == -1) {
		perror("Shared memory creation failed");
		exit(EXIT_FAILURE) ;
	}

	// Attachement de la mémoire partagée
	shared_memory = (struct SharedMemory *)shmat(shmid, NULL, 0);

	return EXIT_SUCCESS ;
}

/*
int askNbProc() {
	int nb_proc ;
	printf("Combien de processus voulez-vous utiliser ? : ") ;
	scanf("%d", &nb_proc) ;

	return nb_proc ;
}
*/

void unsetupAll() {
	// Détachement de la mémoire partagée
	shmdt(shared_memory);

	// Suppression de la mémoire partagée
	shmctl(shmid, IPC_RMID, NULL);

	// Fermeture du sémaphore
	sem_close(sem);
}



int main() {
	createSharedMemory() ;
	createSemaphore() ;
	launchGenerativeProcesses() ;

	// TEST
	int cptTotalCheck = 0 ;
	printf("\n") ;
	for(int i = 0 ; i < sharedMemorySize ; i++) {
		cptTotalCheck += shared_memory->data[i] ;
	}
	printf("Total Count Equals : %d", cptTotalCheck) ;

	unsetupAll() ;
}