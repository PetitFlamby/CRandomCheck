#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>

#define FORK_NUMBERS 5
#define SHM_KEY 1

// Structure pour la mémoire partagée
struct SharedMemory {
    int data[FORK_NUMBERS];
};

// Déclaration du sémaphore
sem_t *sem;

void child_process(int id, struct SharedMemory *shared_memory) {
    // Génération d'un nombre aléatoire
    int random_number = rand() % 100;

    // Attente du sémaphore
    sem_wait(sem);

    // Écriture du nombre aléatoire dans la mémoire partagée
    shared_memory->data[id] = random_number;
    printf("Child %d wrote %d to shared memory\n", id, random_number);

    // Libération du sémaphore
    sem_post(sem);

    exit(0);
}

int main() {

    // Création du sémaphore
    sem = sem_open("/my_semaphore", O_CREAT, 0666, 1);

    // Vérification des erreurs
    if (sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        exit(EXIT_FAILURE);
    }

    // Création de la mémoire partagée
    int shmid = shmget(SHM_KEY, sizeof(struct SharedMemory), IPC_CREAT | 0666);

    // Vérification des erreurs
    if (shmid == -1) {
        perror("Shared memory creation failed");
        exit(EXIT_FAILURE);
    }

    // Attachement de la mémoire partagée
    struct SharedMemory *shared_memory = (struct SharedMemory *)shmat(shmid, NULL, 0);

    // Création des processus fils (forks)
    for (int i = 0; i < FORK_NUMBERS; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Processus fils
            srand(getpid());
            child_process(i, shared_memory);
        }
    }

    // Attente de la fin de tous les processus fils
    for (int i = 0; i < FORK_NUMBERS; ++i) {
        wait(NULL);
    }

    // Affichage des données dans la mémoire partagée
    printf("\nData in shared memory:\n");
    for (int i = 0; i < FORK_NUMBERS; ++i) {
        printf("%d ", shared_memory->data[i]);
    }
    printf("\n");

    // Détachement de la mémoire partagée
    shmdt(shared_memory);

    // Suppression de la mémoire partagée
    shmctl(shmid, IPC_RMID, NULL);

    // Fermeture du sémaphore
    sem_close(sem);

    return 0;
}