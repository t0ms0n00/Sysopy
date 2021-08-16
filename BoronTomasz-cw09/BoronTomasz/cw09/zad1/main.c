#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define REINDEERS 9
#define ELFS 10

pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_elfs_waiting = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_elfs_responded = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_santa_works = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_santa_fly = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reindeers_ready = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reindeers_responded = PTHREAD_COND_INITIALIZER;

int max_elfs_waiting = 3;
int elfs_waiting = 0;
int efls_responded = 0;
pthread_t elfs_ids[3] = {0, 0, 0};
int reindeers_needed = 9;
int reindeers_ready = 0;
int reindeers_responded = 0;
int santa_works = 0;
int santa_ready_to_fly = 0;
int series = 3;

void* reindeer_func(){
    while(series){
        sleep(5+rand()%6);

        pthread_mutex_lock(&reindeer_mutex);

        reindeers_ready++;
        printf("Renifer: czeka %d reniferów na Mikołaja, %ld\n", reindeers_ready, pthread_self());

        while(santa_works){
            pthread_cond_wait(&cond_santa_works, &reindeer_mutex);
        }

        if(reindeers_ready == reindeers_needed && series > 0){
            printf("Renifer: wybudzam Mikołaja, %ld\n", pthread_self());
            pthread_cond_broadcast(&cond_santa_works);
        }

        while(!santa_ready_to_fly && series > 0){
            pthread_cond_wait(&cond_santa_fly, &reindeer_mutex);
        }

        if(series == 0){
            printf("Renifer: %ld, koniec roboty\n", pthread_self());
            pthread_mutex_unlock(&reindeer_mutex);
            pthread_exit(NULL);
        }

        printf("Renifer: leci z Mikołajem, %ld\n", pthread_self());
        reindeers_responded++;
        if(reindeers_responded == reindeers_needed){
            pthread_cond_broadcast(&cond_reindeers_responded);
        }
        pthread_mutex_unlock(&reindeer_mutex);
    }
    printf("Renifer: %ld, koniec roboty\n", pthread_self());
    pthread_exit(NULL);
}

void* elf_func(){
    while(series){
        sleep(2+rand()%4);

        pthread_mutex_lock(&elf_mutex);
        while(elfs_waiting == max_elfs_waiting && series > 0){
            printf("Elf: czeka na powrót elfów, %ld\n", pthread_self());
            pthread_cond_wait(&cond_elfs_waiting, &elf_mutex);
        }

        if(series == 0) {
            printf("Elf: %ld, koniec roboty\n", pthread_self());
            pthread_mutex_unlock(&elf_mutex);
            pthread_exit(NULL);
        }

        elfs_ids[elfs_waiting++] = pthread_self();
        printf("Elf: czeka %d elfów na Mikołaja, %ld\n", elfs_waiting, pthread_self());

        while(santa_ready_to_fly){
            pthread_cond_wait(&cond_santa_fly, &elf_mutex);
        }

        if(pthread_self() == elfs_ids[2] && series > 0){
            printf("Elf: wybudzam Mikołaja, %ld\n", pthread_self());
            pthread_cond_broadcast(&cond_santa_works);
        }

        while(!santa_works && series > 0){
            pthread_cond_wait(&cond_santa_works, &elf_mutex);
        }

        if(series == 0) {
            printf("Elf: %ld, koniec roboty\n", pthread_self());
            pthread_mutex_unlock(&elf_mutex);
            pthread_exit(NULL);
        }

        printf("Elf: Mikołaj rozwiązuje problem, %ld\n", pthread_self());
        efls_responded++;
        if(efls_responded == max_elfs_waiting){
            pthread_cond_broadcast(&cond_elfs_responded);
        }
        pthread_mutex_unlock(&elf_mutex);

        sleep(1+rand()%2);
    }
    printf("Elf: %ld, koniec roboty\n", pthread_self());
    pthread_exit(NULL);
}

void* santa_claus_func(){
    while(series){
        pthread_mutex_lock(&santa_mutex);
        while(elfs_waiting != max_elfs_waiting && reindeers_ready != reindeers_needed){
            pthread_cond_wait(&cond_santa_works, &santa_mutex);
        }
        printf("Mikołaj: budzę się\n");
        pthread_mutex_unlock(&santa_mutex);

        if(reindeers_ready == reindeers_needed){
            pthread_mutex_lock(&reindeer_mutex);
            santa_ready_to_fly = 1;
            pthread_cond_broadcast(&cond_santa_fly);
            while(reindeers_responded != reindeers_needed){
                pthread_cond_wait(&cond_reindeers_responded, &reindeer_mutex);
            }
            printf("Mikołaj: dostarczam zabawki\n");
            sleep(2+rand()%3);
            santa_ready_to_fly = 0;
            printf("Mikołaj: zasypiam\n");
            pthread_cond_broadcast(&cond_santa_fly);
            series--;
            reindeers_ready = 0;
            reindeers_responded = 0;
            pthread_mutex_unlock(&reindeer_mutex);
        }

        else if(elfs_waiting == max_elfs_waiting){
            pthread_mutex_lock(&elf_mutex);
            printf("Mikołaj: rozwiązuje problemy elfów %ld %ld %ld, %ld\n", elfs_ids[0],elfs_ids[1],elfs_ids[2],pthread_self());
            santa_works = 1;
            pthread_cond_broadcast(&cond_santa_works);
            while(efls_responded != max_elfs_waiting){
                pthread_cond_wait(&cond_elfs_responded, &elf_mutex);
            }
            santa_works = 0;
            printf("Mikołaj: zasypiam\n");
            pthread_cond_broadcast(&cond_santa_works);
            sleep(1+rand()%2);
            efls_responded = 0;
            elfs_waiting = 0;
            elfs_ids[0] = elfs_ids[1] = elfs_ids[2] = 0;
            pthread_cond_broadcast(&cond_elfs_waiting);
            pthread_mutex_unlock(&elf_mutex);
        }
    }
    printf("Mikołaj: koniec roboty\n");
    pthread_cond_broadcast(&cond_elfs_waiting);
    pthread_cond_broadcast(&cond_santa_works);
    pthread_exit(NULL);
}

int main(){
    srand(time(NULL));
    pthread_t id_list [1+REINDEERS+ELFS];

    for(int i = 0; i < REINDEERS; i++){
        pthread_create(&id_list[i], NULL, &reindeer_func, NULL);
    }
    for(int i = 0; i < ELFS; i++){
        pthread_create(&id_list[i+REINDEERS], NULL, &elf_func, NULL);
    }
    pthread_create(&id_list[REINDEERS+ELFS], NULL, &santa_claus_func, NULL);

    for(int i = 0; i < REINDEERS + ELFS + 1; i++){
        pthread_join(id_list[i], NULL);
    }

    return 0;
}