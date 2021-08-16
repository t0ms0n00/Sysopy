#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>

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
    key_t table_k = table_key;
    key_t oven_k = oven_key;

    ind = 0;

    signal(SIGINT, handler);

    int oven_id = shmget(oven_k, sizeof(struct oven), IPC_CREAT | 0666);
    if(oven_id == -1){
        perror("Problem with creating shm for oven\n");
        exit(1);
    }

    int table_id = shmget(table_k, sizeof(struct table), IPC_CREAT | 0666);
    if(table_id == -1){
        perror("Problem with creating shm for table\n");
        exit(1);
    }

    struct oven* oven_state = shmat(oven_id, NULL, 0);
    struct table* table_state = shmat(table_id, NULL, 0);

    for(int i = 0; i < MAX_OVEN_CAPACITY; i++){
        oven_state->values[i] = -1;
    }
    oven_state->oven_index = 0;
    oven_state->get_from = 0;

    for(int i = 0; i < MAX_TABLE_CAPACITY; i++){
        table_state->values[i] = -1;
    }
    table_state->table_index = 0;
    table_state->get_from = 0;

    key_t sem_k = sem_key;
    int sem_id = semget(sem_k, 4, IPC_CREAT | 0666);
    semctl(sem_id, 0, SETVAL, MAX_OVEN_CAPACITY);
    semctl(sem_id, 1, SETVAL, MAX_TABLE_CAPACITY);
    semctl(sem_id, 2, SETVAL, 1);
    semctl(sem_id, 3, SETVAL, 1);

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

    if(shmdt(oven_state) == -1){
        perror("Problem with closing oven shm for main\n");
        exit(1);
    }
    if(shmdt(table_state) == -1){
        perror("Problem with closing table shm for main\n");
        exit(1);
    }

    shmctl(table_id, IPC_RMID, 0);
    shmctl(oven_id, IPC_RMID, 0);

    semctl(sem_id, 0, IPC_RMID);

    system("echo \"Test remove\" ");

    system("ipcs -s");

    system("ipcs -m");

    return 0;
}