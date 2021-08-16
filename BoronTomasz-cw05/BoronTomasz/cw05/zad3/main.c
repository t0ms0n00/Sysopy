#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

void make_producer_file(int prod_num, int N){
    char filename[20];
    char num[5];
    sprintf(num, "%d", prod_num);
    strcpy(filename, "producer");
    strcat(filename, num);
    strcat(filename,".txt");
    FILE* file = fopen(filename, "w");
    char random_letter = 'A' + rand() % 26;
    int times = 1 + rand()%(9*N);
    for(int i=0; i<times; i++){
        fputc(random_letter, file);
    }
    fclose(file);
}

int compare_files(char* cust_file, char* prod_file){
    FILE* customer = fopen(cust_file, "r");
    FILE* producer = fopen(prod_file, "r");
    char* text = calloc(50000, sizeof(char));
    char* prod_text = calloc(50000, sizeof(char));
    size_t buff_size;
    getline(&prod_text, &buff_size, producer);
    strcat(prod_text,"\n");
    while(!feof(customer)){
        getline(&text, &buff_size, customer);
        if(strcmp(text,prod_text)==0){
            free(text);
            free(prod_text);
            fclose(producer);
            fclose(customer);
            return 1;
        }
        free(text);
        text = calloc(50000, sizeof(char));
    }
    free(text);
    free(prod_text);
    fclose(producer);
    fclose(customer);
    return 0;
}

int main(int argc, char** argv){
    if(argc != 4){
        perror("./main [num of signs to write/read from fifo] [num of test type] [result file]\n");
        exit(1);
    }
    srand(time(NULL));
    int N = atoi(argv[1]);
    int mode = atoi(argv[2]);
    if(mode == 1){
        for(int i=0; i<5; i++) make_producer_file(i+1, N);
        mkfifo("fifo", 0666);
        char* prod_arguments[5][6] = {
            {"./producer", "fifo", "1", "producer1.txt", argv[1], NULL},
            {"./producer", "fifo", "2", "producer2.txt", argv[1], NULL},
            {"./producer", "fifo", "3", "producer3.txt", argv[1], NULL},
            {"./producer", "fifo", "4", "producer4.txt", argv[1], NULL},
            {"./producer", "fifo", "5", "producer5.txt", argv[1], NULL}
        };
        char* cons_arguments[5] = {"./consumer", "fifo", argv[3], argv[1], NULL};
        pid_t child_pid;
        for(int i=0; i<5; i++){
            if((child_pid = fork()) == 0){
                if(execvp(prod_arguments[i][0], prod_arguments[i]) == -1){
                    perror("Producer exec error\n");
                    exit(1);
                }
            }
        }
        if((child_pid = fork()) == 0){
            if(execvp(cons_arguments[0], cons_arguments) == -1){
                perror("Consumer exec error\n");
                exit(1);
            }
        }
        for(int i=0; i<6; i++) wait(NULL);
        for(int i=0; i<5; i++){
            if(compare_files(argv[3], prod_arguments[i][3]) == 0){
                printf("ERROR FILE %s AND %s ARE NOT EQUAL\n", argv[3], prod_arguments[i][3]);
                exit(1);
            }
        }
        for(int i=0; i<5; i++) wait(NULL);
        printf("Everything done\n");
        printf("All files copied properly\n");
    }
    else if(mode == 2){
        make_producer_file(0, N);
        mkfifo("fifo", 0666);
        char* prod_arguments[6] = {"./producer", "fifo", "1", "producer0.txt", argv[1], NULL};
        char* cons_arguments[5][5] = {
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL}
        };
        pid_t child_pid;
        for(int i=0; i<5; i++){
            if((child_pid = fork()) == 0){
                if(execvp(cons_arguments[i][0], cons_arguments[i]) == -1){
                    perror("Producer exec error\n");
                    exit(1);
                }
            }
        }
        if((child_pid = fork()) == 0){
            if(execvp(prod_arguments[0], prod_arguments) == -1){
                perror("Consumer exec error\n");
                exit(1);
            }
        }
        for(int i=0; i<6; i++) wait(NULL);
        if(compare_files(argv[3], "producer0.txt") == 0){
            printf("ERROR FILE %s AND producer0.txt ARE NOT EQUAL\n", argv[3]);
            exit(1);
        }
        printf("Everything done\n");
        printf("All files copied properly\n");
    }
    else if(mode == 3){
        for(int i=0; i<5; i++) make_producer_file(i+1, N);
        mkfifo("fifo", 0666);
        char* prod_arguments[5][6] = {
            {"./producer", "fifo", "1", "producer1.txt", argv[1], NULL},
            {"./producer", "fifo", "2", "producer2.txt", argv[1], NULL},
            {"./producer", "fifo", "3", "producer3.txt", argv[1], NULL},
            {"./producer", "fifo", "4", "producer4.txt", argv[1], NULL},
            {"./producer", "fifo", "5", "producer5.txt", argv[1], NULL}
        };
        char* cons_arguments[5][5] = {
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL},
            {"./consumer", "fifo", argv[3], argv[1], NULL}
        };
        pid_t child_pid;
        for(int i=0; i<5; i++){
            if((child_pid = fork()) == 0){
                if(execvp(cons_arguments[i][0], cons_arguments[i]) == -1){
                    perror("Producer exec error\n");
                    exit(1);
                }
            }
        }
        for(int i=0; i<5; i++){
            if((child_pid = fork()) == 0){
                if(execvp(prod_arguments[i][0], prod_arguments[i]) == -1){
                    perror("Producer exec error\n");
                    exit(1);
                }
            }
        }
        for(int i=0; i<11; i++) wait(NULL);
        for(int i=0; i<5; i++){
            if(compare_files(argv[3], prod_arguments[i][3]) == 0){
                printf("ERROR FILE %s AND %s ARE NOT EQUAL\n", argv[3], prod_arguments[i][3]);
                exit(1);
            }
        }
        printf("Everything done\n");
        printf("All files copied properly\n");
    }
    return 0;
}