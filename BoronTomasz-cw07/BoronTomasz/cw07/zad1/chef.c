#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>

#include "shm_keys.h"

int running = 1;
struct oven* oven_state;
struct table* table_state;

void handler(int signo){
    if(shmdt(oven_state) == -1){
        perror("Problem with closing oven shm for chef\n");
        exit(1);
    }
    if(shmdt(table_state) == -1){
        perror("Problem with closing table shm for chef\n");
        exit(1);
    }
    raise(9);
}

int main(int argc, char** argv){
    signal(SIGINT, handler);

    key_t table_k = table_key;

    key_t oven_k = oven_key;
    int table_id = shmget(table_k, 0, 0666);
    int oven_id = shmget(oven_k, 0, 0666);
    oven_state = shmat(oven_id, NULL, 0);
    table_state = shmat(table_id, NULL, 0);

    key_t sem_k = sem_key;
    int sem_id = semget(sem_k, 0, 0666);
    struct sembuf operations[1];

    srand(getpid());

    int rest = 0;

    while(running){
        int pizza_type = rand() % 10;
        printf("%d %lu Przygotowuję pizzę: %d\n", getpid(), time(NULL), pizza_type);
        rest = 1 + rand() % 2;
        sleep(rest);

        operations[0].sem_num = 2;  /// oven lock
        operations[0].sem_op = -1;
        semop(sem_id, operations, 1);

        operations[0].sem_num = 0;
        operations[0].sem_op = -1;
        semop(sem_id, operations, 1);
        oven_state->values[(oven_state->oven_index++) % MAX_OVEN_CAPACITY] = pizza_type;
        int free_space_in_oven = semctl(sem_id, 0, GETVAL);
        printf("%d %lu Dodałem pizze: %d Liczba pizz w piecu: %d\n",getpid(), time(NULL), pizza_type, MAX_OVEN_CAPACITY - free_space_in_oven);

        operations[0].sem_num = 2;  /// oven unlock
        operations[0].sem_op = 1;
        semop(sem_id, operations, 1);

        rest = 1 + rand() % 5;
        sleep(rest);

        operations[0].sem_num = 2;  /// oven lock
        operations[0].sem_op = -1;
        semop(sem_id, operations, 1);

        int pizza_from_oven = oven_state->values[oven_state->get_from];
        oven_state->values[oven_state->get_from] = -1;
        oven_state->get_from = (oven_state->get_from+1) % MAX_OVEN_CAPACITY;
        operations[0].sem_num = 0;
        operations[0].sem_op = 1;
        semop(sem_id, operations, 1);

        operations[0].sem_num = 2;  /// oven unlock
        operations[0].sem_op = 1;
        semop(sem_id, operations, 1);

        operations[0].sem_num = 1;
        operations[0].sem_op = -1;
        semop(sem_id, operations, 1);
        table_state->values[(table_state->table_index++) % MAX_TABLE_CAPACITY] = pizza_from_oven;
        free_space_in_oven = semctl(sem_id, 0, GETVAL);
        int free_space_on_table = semctl(sem_id, 1, GETVAL);
        printf("%d %lu Wyjmuje pizze: %d Liczba pizz w piecu: %d Liczba pizz na stole: %d\n",getpid(), time(NULL), pizza_type, 
            MAX_OVEN_CAPACITY - free_space_in_oven, MAX_TABLE_CAPACITY - free_space_on_table);

    }
    return 0;
}