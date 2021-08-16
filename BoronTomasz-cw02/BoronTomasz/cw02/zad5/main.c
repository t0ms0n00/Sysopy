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

void copyFiles(FILE* from,FILE* to){
    fpos_t pos;
    fpos_t prev;
    pos.__pos=-1;
    char* line=calloc(50,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    int counter;
    while(!feof(from)){
        counter=0;
        pos.__pos++;
        prev=pos;
        fread(sign,1,1,from);
        while(strcmp(sign,"\n")!=0){
            counter++;
            if(counter==50){
                fsetpos(from,&prev);
                fread(line,1,50,from);
                prev=pos;
                prev.__pos++;
                fwrite(line,1,50,to);
                fwrite("\n",1,1,to);
                counter=0;
                free(line);
                line=calloc(50,sizeof(char));
            }
            pos.__pos++;
            fread(sign,1,1,from);
        }
        fsetpos(from,&prev);
        fread(line,1,pos.__pos-prev.__pos+1,from);
        fwrite(line,1,pos.__pos-prev.__pos+1,to);
        free(line);
        line=calloc(50,sizeof(char));
    }
    free(line);
}    

int main(int argc,char** argv){
    char* f1=calloc(256,sizeof(char));
    char* f2=calloc(256,sizeof(char));
    if(argc==1){
        printf("Copy from file:");
        scanf("%s",f1);
        printf("To file:");
        scanf("%s",f2);
    }
    else if (argc==2){
        strcpy(f1,argv[1]);
        printf("To file:");
        scanf("%s",f2);
    }
    else{
        strcpy(f1,argv[1]);
        strcpy(f2,argv[2]);
    }
    FILE* from = fopen(f1,"r");
    FILE* to = fopen(f2,"w");
    if(from==NULL){
        perror(f1);
        exit(1);
    }
    if(to==NULL){
        perror(f2);
        exit(1);
    }
    clock_t time1=clock();
    copyFiles(from,to);
    clock_t time2=clock();
    FILE* res=fopen("pomiar_zad_5.txt","a");
    if(res==NULL){
        perror("pomiar_zad_5.txt");
        exit(1);
    }
    fprintf(res,"Test for copy from: %s to: %s\n",f1,f2);
    timeDiff(res,time1,time2);
    fclose(res);
    fclose(from);
    fclose(to);
    free(f1);
    free(f2);
    return 0;
}