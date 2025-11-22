#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define SHM_NAME "/counter_shm"
#define SEM_NAME "/counter_sem"
#define TARGET 1000

int main(int argc, char *argv[]) {
    int creator = 0;
    if (argc > 1) {
        if (strcmp(argv[1], "--init") == 0) creator = 1;
        else if (strcmp(argv[1], "--cleanup") == 0) {
            shm_unlink(SHM_NAME);
            sem_unlink(SEM_NAME);
            return 0;
        } else {
            return 1;
        }
    }

    if (creator) {
        int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        ftruncate(fd, sizeof(int));
        void *map = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        int *counter = (int*)map;
        *counter = 0;
        munmap(map, sizeof(int));
        close(fd);
        sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
        if (sem == SEM_FAILED) {
            sem_unlink(SEM_NAME);
            sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
        }
        sem_close(sem);
        return 0;
    }

    int fd = shm_open(SHM_NAME, O_RDWR, 0);
    void *map = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int *counter = (int*)map;
    sem_t *sem = sem_open(SEM_NAME, 0);
    srand((unsigned)getpid() ^ (unsigned)time(NULL));

    while (1) {
        sem_wait(sem);
        int val = *counter;

        if (val >= TARGET) {
            sem_post(sem);
            break;
        }

        int toss = (rand() % 2) + 1;
        if (toss == 2) {
            val++;
            *counter = val;
            printf("PID %d wrote %d\n", getpid(), val);
        } else {
            printf("PID %d tossed %d current=%d\n", getpid(), toss, val);
        }

        sem_post(sem);
        usleep(1000 + (rand() % 5000));
    }

    sem_close(sem);
    munmap(map, sizeof(int));
    close(fd);

    return 0;
}
