#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/file.h>

void put_in_file(char* filename, int line, char* fragment){
    int act_line = 1;
    struct stat buff;
    if(stat(filename, &buff) == -1){
        char command[256];
        strcpy(command, "touch ");
        strcat(command, filename);
        system(command);
        FILE* prepare = fopen(filename, "w");
        for(int i=0; i<50; i++){
            fputs("\n", prepare);
        }
        fclose(prepare);
    }
    FILE* file = fopen(filename, "r");
    FILE* tmp = fopen("tmp.txt", "w");
    flock(fileno(tmp), LOCK_SH);
    size_t buffer_size = 50000;
    char* text = calloc(50000, sizeof(char));
    while(!feof(file)){
        getline(&text, &buffer_size, file);
        fputs(text, tmp);
        if(act_line == line){
            fseek(tmp, -1, 1);
            fputs(fragment, tmp);
            fputs("\n", tmp);
        }
        free(text);
        text = calloc(50000, sizeof(char));
        act_line++;
    }
    if(act_line <= line){
        free(text);
    }
    while(act_line <= line){
        if(act_line == line){
            fputs("\n", tmp);
            fputs(fragment, tmp);
        }
        else fputs("\n", tmp);
        act_line++;
    }
    flock(fileno(tmp), LOCK_UN);
    fclose(tmp);
    fclose(file);
    remove(filename);
    rename("tmp.txt", filename);
}

int main(int argc, char** argv){
    if(argc != 4){
        perror("./producer [FIFO name] [save file path] [signs to read]\n");
        exit(1);
    }
    FILE* fifo = fopen(argv[1], "r");
    printf("FIFO OPENED WITH READ MODE\n");
    char* save_file = calloc(100, sizeof(char));
    strcpy(save_file, argv[2]);
    size_t N = atoi(argv[3]);
    int row_num = 0;
    char text[N + 10];
    while(fscanf(fifo, "%d %s", &row_num, text) > 0){
        printf("READING FROM FIFO\n");
        put_in_file(save_file, row_num, text);
    }
    free(save_file);
    fclose(fifo);
    return 0;
}