#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char** argv){
    if(argc != 5){
        perror("./producer [FIFO name] [row number] [text path] [signs to read]\n");
        exit(1);
    }
    FILE* fifo = fopen(argv[1], "w");
    printf("FIFO OPENED WITH WRITE MODE\n");
    int row_num = atoi(argv[2]);
    FILE* text = fopen(argv[3], "r");
    size_t N = atoi(argv[4]);
    srand(time(NULL));
    while(!feof(text)){
        char* check = calloc(2, sizeof(char));
        fread(check, sizeof(char), 1, text);
        if(strcmp(check, "\0") == 0) break;
        fseek(text, -1, 1);
        free(check);
        printf("WRITING TO FIFO\n");
        int blocker = 1+rand()%2;
        sleep(blocker);
        char* fragment = calloc(N+1, sizeof(char));
        fread(fragment, sizeof(char), N, text);
        fprintf(fifo, "%d %s ", row_num, fragment);
        free(fragment);
    }
    fclose(text);
    fclose(fifo);
    return 0;
}