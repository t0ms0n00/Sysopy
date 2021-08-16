#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shm_keys.h"

pid_t workers_pid[MAX_WORKERS];
int ind;

void handler(int signo){
    for(int i=0; i<ind; i++){
        kill(workers_pid[i], SIGINT);
    }
}

int main(int argc, char** argv){
    if(argc != 3){
        perror("Program format: ./main [number of chefs] [number of suppliers]\n");
        exit(1);
    }

    printf("SIGINT TO EXIT\n");
    printf("STARTING IN 3 SEC.\n");
    sleep(3);

    int chefs = atoi(argv[1]);
    int suppliers = atoi(argv[2]);
    ind = 0;

    signal(SIGINT, handler);

    int oven_id = shm_open(OVEN, O_CREAT | O_RDWR, 0666);
    if(oven_id < 0){
        perror("Problem with creating shm for oven\n");
        exit(1);
    }
    ftruncate(oven_id, MAX_OVEN_CAPACITY*sizeof(int));

    int table_id = shm_open(TABLE, O_CREAT | O_RDWR, 0666);
    if(table_id < 0){
        perror("Problem with creating shm for table\n");
        exit(1);
    }
    ftruncate(table_id, MAX_TABLE_CAPACITY*sizeof(int));

    struct oven* oven_state = mmap(NULL, sizeof(struct oven), PROT_WRITE, MAP_SHARED, oven_id, 0);
    struct table* table_state = mmap(NULL, sizeof(struct table),  PROT_WRITE, MAP_SHARED, table_id, 0);

    for(int i = 0;i < MAX_OVEN_CAPACITY; i++){
        oven_state->values[i] = -1;
    }
    oven_state->oven_index = 0;
    oven_state->get_from = 0;

    for(int i = 0; i < MAX_TABLE_CAPACITY; i++){
        table_state->values[i] = -1;
    }
    table_state->table_index = 0;
    table_state->get_from = 0;

    sem_t* table_space = sem_open(S_TABLE_FREE_SPACE, O_CREAT | O_RDWR, 0666, MAX_TABLE_CAPACITY);
    sem_t* oven_space = sem_open(S_OVEN_FREE_SPACE, O_CREAT | O_RDWR, 0666, MAX_OVEN_CAPACITY);
    sem_t* table_lock = sem_open(S_TABLE_LOCK, O_CREAT | O_RDWR, 0666, 1);
    sem_t* oven_lock = sem_open(S_OVEN_LOCK, O_CREAT | O_RDWR, 0666, 1);

    pid_t child_pid;

    for(int i = 0; i < suppliers; i++){
        child_pid = fork();
        if(child_pid == 0){
            if(execlp("./supplier", "./supplier", NULL) == -1){
                perror("Problem with exec for supplier\n");
                exit(1);
            }
        }
        else{
            workers_pid[ind++] = child_pid;
        }
    }

    for(int i = 0; i < chefs; i++){
        child_pid = fork();
        if(child_pid == 0){
            if(execlp("./chef", "./chef", NULL) == -1){
                perror("Problem with exec for chef\n");
                exit(1);
            }
        }
        else{
            workers_pid[ind++] = child_pid;
        }
    }


    for(int i = 0; i < chefs+suppliers; i++){
        wait(NULL);
    }

    sem_close(table_space);
    sem_close(oven_space);
    sem_close(table_lock);
    sem_close(oven_lock);

    sem_unlink(S_TABLE_FREE_SPACE);
    sem_unlink(S_OVEN_FREE_SPACE);
    sem_unlink(S_TABLE_LOCK);
    sem_unlink(S_OVEN_LOCK);

    munmap(oven_state, MAX_OVEN_CAPACITY*sizeof(int));
    munmap(table_state, MAX_TABLE_CAPACITY*sizeof(int));

    shm_unlink(TABLE);
    shm_unlink(OVEN);

    return 0;
}