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
struct table* table_state;
sem_t* table_space;
sem_t* table_lock;

void handler(int signo){
    munmap(table_state, MAX_TABLE_CAPACITY*sizeof(int));
    sem_close(table_space);
    sem_close(table_lock);
    raise(9);
}

int main(int argc, char** argv){
    signal(SIGINT, handler);

    int table_id = shm_open(TABLE, O_RDWR, 0666);
    if(table_id < 0){
        perror("Problem with creating shm for table\n");
        exit(1);
    }
    ftruncate(table_id, MAX_TABLE_CAPACITY*sizeof(int));

    table_state = mmap(NULL, sizeof(struct table), PROT_WRITE | PROT_READ, MAP_SHARED, table_id, 0);

    table_space = sem_open(S_TABLE_FREE_SPACE, O_RDWR, 0666);
    table_lock = sem_open(S_TABLE_LOCK, O_RDWR, 0666);

    int rest = 0;
    int pizza_get;
    int free_space_on_table;

    while(running){
        sem_wait(table_lock);

        pizza_get = table_state->values[table_state->get_from];
        if(pizza_get == -1){
            sem_post(table_lock);
            continue;
        }

        table_state->values[table_state->get_from] = -1;
        table_state->get_from = (table_state->get_from+1) % MAX_TABLE_CAPACITY;
        sem_post(table_space);
        sem_getvalue(table_space, &free_space_on_table);
        printf("%d %lu Pobieram pizze: %d Liczba pizz na stole: %d\n",getpid(), time(NULL), pizza_get, MAX_TABLE_CAPACITY - free_space_on_table);

        sem_post(table_lock);

        rest = 1 + rand() % 5;
        sleep(rest);

        printf("%d %lu Dostarczam pizze: %d\n",getpid(), time(NULL), pizza_get);

        rest = 1 + rand() % 5;
        sleep(rest);
    }
    return 0;
}