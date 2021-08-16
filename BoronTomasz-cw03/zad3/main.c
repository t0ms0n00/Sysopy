#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

/* przygotowalem szesc testow pod make test:

Wstepne przygotowanie:

1. napis "pattern exists", ktory umiescilem w plikach o numerach: 3, 6, 7, 9, 13, 14, 15, 16

2. napis "process", ktory umiescilem w plikach o numerach: 1, 2, 3, 4, 8, 9, 11, 15

Testy:

Dwa testy zaczynajace sie w katalogu dir, na maksymalny poziom glebokosci przygotowanej struktury (obecnie 5 poziom)

Dwa testy zaczynajace sie w katalogu dir, na drugi poziom zaglebienia

Dwa testy z katalogu dir7, na drugi poziom zaglebienia

*/

bool findPattern(FILE* file,char* pattern){
    int patternLen = strlen(pattern);
    int stepBack = patternLen - 1;
    char* cut = calloc(patternLen,sizeof(char));
    while(!feof(file)){
        size_t check = fread(cut,1,patternLen,file);
        if(check < patternLen){
            break;
        }
        if(strcmp(cut,pattern)==0) return true;
        fseek(file,-stepBack,1);
        free(cut);
        cut = calloc(patternLen,sizeof(char));
    }
    free(cut);
    return false;
}

void getAllFilesFromDir(char* path, char* label, int depth, int max_depth){
    struct stat st_buf;
    pid_t child_pid;
    DIR* root = opendir(path);
    if(depth < max_depth){
        struct dirent* current = readdir(root);
        while(current != NULL){
            char* new_path = calloc(256,sizeof(char));
            strcpy(new_path,path);
            strcat(new_path,"/");
            strcat(new_path,current->d_name);
            stat(new_path, &st_buf);
            DIR* kid = opendir(new_path);
            if(strcmp(current->d_name,"..")==0 || strcmp(current->d_name,".")==0){
                current = readdir(root);
                continue;
            }
            if(S_ISDIR(st_buf.st_mode) && kid != NULL) {
                child_pid = fork();
                wait(NULL);
                if(child_pid == 0) {
                    getAllFilesFromDir(new_path, label, depth+1, max_depth);
                    exit(0);
                }
            }
            char* check = calloc(4,sizeof(char));
            strcpy(check,&new_path[strlen(new_path)-4]);
            FILE* file = fopen(new_path,"r");
            if(strcmp(check,".txt")==0 && S_ISREG(st_buf.st_mode) && file != NULL){
                printf("PID = %d, PATH = %s\n",getpid(),path);
                printf("FILE: %s\n",current->d_name);
                printf(findPattern(file,label) ? "FOUND\n" : "NOT FOUND\n");
                printf("\n");
                fclose(file);
            }
            free(check);
            free(new_path);
            current = readdir(root);
        }
    }
    closedir(root);
}

int main(int argc, char** argv){
    if(argc<2){
        perror("You should enter the directory path\n");
        exit(1);
    }
    if(argc<3){
        perror("You should enter the label you search for\n");
        exit(1);
    }
    if(argc<4){
        perror("You should enter max depth of searching\n");
        exit(1);
    }
    char* root = argv[1];
    char* label = argv[2];
    int max_depth = atoi(argv[3]);
    getAllFilesFromDir(root, label, 0, max_depth);
    return 0;
}