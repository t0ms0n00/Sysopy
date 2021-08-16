#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shm_keys.h"

int running = 1;
struct oven* oven_state;
struct table* table_state;
sem_t* table_space;
sem_t* table_lock;
sem_t* oven_space;
sem_t* oven_lock;

void handler(int signo){
    munmap(oven_state, MAX_OVEN_CAPACITY*sizeof(int));
    munmap(table_state, MAX_TABLE_CAPACITY*sizeof(int));
    sem_close(table_space);
    sem_close(oven_space);
    sem_close(table_lock);
    sem_close(oven_lock);
    raise(9);
}

int main(int argc, char** argv){
    signal(SIGINT, handler);

    int oven_id = shm_open(OVEN, O_RDWR, 0666);
    if(oven_id < 0){
        perror("Problem with creating shm for oven\n");
        exit(1);
    }
    ftruncate(oven_id, MAX_OVEN_CAPACITY*sizeof(int));

    int table_id = shm_open(TABLE, O_RDWR, 0666);
    if(table_id < 0){
        perror("Problem with creating shm for table\n");
        exit(1);
    }
    ftruncate(table_id, MAX_TABLE_CAPACITY*sizeof(int));

    oven_state = mmap(NULL, sizeof(struct oven), PROT_READ | PROT_WRITE, MAP_SHARED, oven_id, 0);
    table_state = mmap(NULL, sizeof(struct table), PROT_WRITE | PROT_READ, MAP_SHARED, table_id, 0);

    table_space = sem_open(S_TABLE_FREE_SPACE, O_RDWR, 0666);
    oven_space = sem_open(S_OVEN_FREE_SPACE, O_RDWR, 0666);
    table_lock = sem_open(S_TABLE_LOCK, O_RDWR, 0666);
    oven_lock = sem_open(S_OVEN_LOCK, O_RDWR, 0666);

    srand(getpid());

    int rest = 0;
    int free_space_in_oven;
    int free_space_on_table;

     while(running){
        int pizza_type = rand() % 10;
        printf("%d %lu Przygotowuję pizzę: %d\n", getpid(), time(NULL), pizza_type);
        rest = 1 + rand() % 2;
        sleep(rest);

        sem_wait(oven_lock);

        sem_wait(oven_space);
        oven_state->values[((oven_state->oven_index++) % MAX_OVEN_CAPACITY)] = pizza_type;
        sem_getvalue(oven_space, &free_space_in_oven);
        printf("%d %lu Dodałem pizze: %d Liczba pizz w piecu: %d\n",getpid(), time(NULL), pizza_type, MAX_OVEN_CAPACITY - free_space_in_oven);

        sem_post(oven_lock);

        rest = 1 + rand() % 5;
        sleep(rest);

        sem_wait(oven_lock);

        int pizza_from_oven = oven_state->values[oven_state->get_from];
        oven_state->values[oven_state->get_from] = -1;
        oven_state->get_from = (oven_state->get_from+1) % MAX_OVEN_CAPACITY;
        sem_post(oven_space);

        sem_post(oven_lock);

        sem_wait(table_space);
        table_state->values[(table_state->table_index++) % MAX_TABLE_CAPACITY] = pizza_from_oven;
        sem_getvalue(oven_space, &free_space_in_oven);
        sem_getvalue(table_space, &free_space_on_table);
        printf("%d %lu Wyjmuje pizze: %d Liczba pizz w piecu: %d Liczba pizz na stole: %d\n",getpid(), time(NULL), pizza_type, 
            MAX_OVEN_CAPACITY - free_space_in_oven, MAX_TABLE_CAPACITY - free_space_on_table);
    }
    return 0;
}