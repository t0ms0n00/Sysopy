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
struct table* table_state;

void handler(int signo){
    if(shmdt(table_state) == -1){
        perror("Problem with closing table shm for chef\n");
        exit(1);
    }
    raise(9);
}

int main(int argc, char** argv){
    signal(SIGINT, handler);

    key_t table_k = table_key;
    int table_id = shmget(table_k, 0, 0666);
    table_state = shmat(table_id, NULL, 0);

    int sem_id = semget(sem_key, 0, 0666);
    struct sembuf operations[1];

    int rest = 0;
    int pizza_get;

    while(running){

        operations[0].sem_num = 3;  /// table lock
        operations[0].sem_op = -1;
        semop(sem_id, operations, 1);

        pizza_get = table_state->values[table_state->get_from];
        if(pizza_get == -1){
            operations[0].sem_num = 3;  /// table unlock
            operations[0].sem_op = 1;
            semop(sem_id, operations, 1);
            continue;
        }
        table_state->values[table_state->get_from] = -1;
        table_state->get_from = (table_state->get_from+1) % MAX_TABLE_CAPACITY;
        operations[0].sem_num = 1;
        operations[0].sem_op = 1;
        semop(sem_id, operations, 1);
        int free_space_on_table = semctl(sem_id, 1, GETVAL);
        printf("%d %lu Pobieram pizze: %d Liczba pizz na stole: %d\n",getpid(), time(NULL), pizza_get, MAX_TABLE_CAPACITY - free_space_on_table);

        operations[0].sem_num = 3;  /// table unlock
        operations[0].sem_op = 1;
        semop(sem_id, operations, 1);

        rest = 1 + rand() % 5;
        sleep(rest);

        printf("%d %lu Dostarczam pizze: %d\n",getpid(), time(NULL), pizza_get);

        rest = 1 + rand() % 5;
        sleep(rest);
    }
    return 0;
}