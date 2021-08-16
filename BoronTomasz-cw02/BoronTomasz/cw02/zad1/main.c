#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

void timeDiff(FILE *f,clock_t start,clock_t end){
    double seconds=(double)((end-start)/CLOCKS_PER_SEC);
    fprintf(f,"Time for lib: %f\n",seconds);
}

void showRoundRobin(FILE* f1, FILE* f2){
    fpos_t pos1;
    fpos_t pos2;
    fpos_t prev1;
    fpos_t prev2;
    pos1.__pos=-1;
    pos2.__pos=-1;
    char* line=calloc(256,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    while(!feof(f1) && !feof(f2)){
        pos1.__pos++;
        pos2.__pos++;
        prev1=pos1;
        fread(sign,1,1,f1);
        while(strcmp(sign,"\n")!=0 && !feof(f1)){
            pos1.__pos++;
            fsetpos(f1,&pos1);
            fread(sign,1,1,f1);
        }
        fsetpos(f1,&prev1);
        fread(line,1,pos1.__pos-prev1.__pos+1,f1);
        printf("%s",line);
        free(line);
        line=calloc(256,sizeof(char));
        prev2=pos2;
        fread(sign,1,1,f2);
        while(strcmp(sign,"\n")!=0 && !feof(f2)) {
            pos2.__pos++;
            fsetpos(f2,&pos2);
            fread(sign,1,1,f2);
        }
        fsetpos(f2,&prev2);
        fread(line,1,pos2.__pos-prev2.__pos+1,f2);
        printf("%s",line);
        free(line);
        line=calloc(256,sizeof(char));
    }
    if(feof(f1)){
        while(!feof(f2)){
            pos2.__pos++;
            prev2=pos2;
            fread(sign,1,1,f2);
            while(strcmp(sign,"\n")!=0 && !feof(f2)) {
                pos2.__pos++;
                fsetpos(f2,&pos2);
                fread(sign,1,1,f2);
            }
            fsetpos(f2,&prev2);
            fread(line,1,pos2.__pos-prev2.__pos+1,f2);
            printf("%s",line);
            free(line);
            line=calloc(256,sizeof(char));
        }
    }
    else{
        while(!feof(f1)){
            pos1.__pos++;
            prev1=pos1;
            fread(sign,1,1,f1);
            while(strcmp(sign,"\n")!=0 && !feof(f1)) {
                pos1.__pos++;
                fsetpos(f1,&pos1);
                fread(sign,1,1,f1);
            }
            fsetpos(f1,&prev1);
            fread(line,1,pos1.__pos-prev1.__pos+1,f1);
            printf("%s",line);
            free(line);
            line=calloc(256,sizeof(char));
        }
    }
    free(line);
    free(sign);
}

int main(int argc,char** argv){
    char* name1=calloc(256,sizeof(char));
    char* name2=calloc(256,sizeof(char));
    if(argc==1){
        printf("File1=");
        scanf("%s",name1);
        printf("File2=");
        scanf("%s",name2);
    }
    else if (argc==2){
        strcpy(name1,argv[1]);
        printf("File2=");
        scanf("%s",name2);
    }
    else{
        strcpy(name1,argv[1]);
        strcpy(name2,argv[2]);
    }
    FILE* f1 = fopen(name1,"r");
    FILE* f2 = fopen(name2,"r");
    if(f1==NULL){
        perror(name1);
        exit(1);
    }
    if(f2==NULL){
        perror(name2);
        exit(1);
    }
    clock_t time1=clock();
    showRoundRobin(f1,f2);
    clock_t time2=clock();
    FILE* res=fopen("pomiar_zad_1.txt","a");
    if(res==NULL){
        perror("pomiar_zad_1.txt");
        exit(1);
    }
    fprintf(res,"Test for files: %s : %s\n",name1,name2);
    timeDiff(res,time1,time2);
    fclose(res);
    fclose(f1);
    fclose(f2);
    free(name1);
    free(name2);
    return 0;
}