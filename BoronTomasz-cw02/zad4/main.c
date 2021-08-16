#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

void timeDiff(FILE *f,clock_t start,clock_t end){
    double seconds=(double)((end-start)/CLOCKS_PER_SEC);
    fprintf(f,"Time for lib: %f\n",seconds);
}

void switchText(FILE* from,FILE* to,char* n1,char* n2){
    int patternLen=strlen(n1);
    int stepBack=patternLen-1;
    char* cut=calloc(patternLen,sizeof(char));
    while(!feof(from)){
        size_t check=fread(cut,1,patternLen,from);
        if(check<patternLen){
            fseek(from,-strlen(cut),1);
            fwrite(cut,1,strlen(cut),to);
            break;
        }
        if(strcmp(cut,n1)==0){
            fwrite(n2,1,strlen(n2),to);
        }
        else{
            fwrite(&cut[0],1,1,to);
            fseek(from,-stepBack,1);
        }
        free(cut);
        cut=calloc(patternLen,sizeof(char));
    }
    free(cut);
}

int main(int argc,char** argv){
    char* fileFrom=calloc(256,sizeof(char));
    char* fileTo=calloc(256,sizeof(char));
    char* n1=calloc(256,sizeof(char));
    char* n2=calloc(256,sizeof(char));
    strcpy(fileFrom,argv[1]);
    strcpy(fileTo,argv[2]);
    strcpy(n1,argv[3]);
    strcpy(n2,argv[4]);
    FILE* from = fopen(fileFrom,"r");
    if(from==NULL){
        perror(fileFrom);
        exit(1);
    }
    FILE* to = fopen(fileTo,"w");
    if(to==NULL){
        perror(fileTo);
        exit(1);
    }
    clock_t time1=clock();
    switchText(from,to,n1,n2);
    clock_t time2=clock();
    FILE* res=fopen("pomiar_zad_4.txt","a");
    if(res==NULL){
        perror("pomiar_zad_4.txt");
        exit(1);
    }
    fprintf(res,"Test copy from %s to %s, switch %s to %s\n",fileFrom,fileTo,n1,n2);
    timeDiff(res,time1,time2);
    fclose(res);
    fclose(to);
    fclose(from);
    free(fileFrom);
    free(fileTo);
    free(n1);
    free(n2);
    return 0;
}