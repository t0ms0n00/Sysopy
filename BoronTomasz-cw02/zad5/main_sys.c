#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

void timeDiff(int f,clock_t start,clock_t end){
    double seconds=(double)((end-start)/CLOCKS_PER_SEC);
    char* time=calloc(10,sizeof(char));
    write(f,"Time for sys: ",14);
    snprintf(time,10,"%f",seconds);
    write(f,time,8);
    write(f,"\n",1);
    free(time);
}

void copyFiles(int from, int to){
    char* line=calloc(50,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    int counter;
    while(read(from,sign,1)>0){
        counter=0;
        lseek(from,-1,SEEK_CUR);
        while(read(from,sign,1)>0 && strcmp(sign,"\n")!=0){
            counter++;
            if(counter==50){
                lseek(from,-50,SEEK_CUR);
                read(from,line,50);
                write(to,line,50);
                write(to,"\n",1);
                counter=0;
                free(line);
                line=calloc(50,sizeof(char));
            }
        }
        lseek(from,-counter,SEEK_CUR);
        read(from,line,counter);
        write(to,line,counter);
        free(line);
        line=calloc(50,sizeof(char));
    }
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
    int from = open(f1,O_RDONLY);
    int to = open(f2,O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(from==-1){
        perror(f1);
        exit(1);
    }
    clock_t time1=clock();
    copyFiles(from,to);
    clock_t time2=clock();
    int res=open("pomiar_zad_5.txt",O_WRONLY);
    if(res==-1){
        perror("pomiar_zad_5.txt");
        exit(1);
    }
    lseek(res,0,SEEK_END);
    write(res,"Test for copy from: ",20);
    write(res,f1,strlen(f1));
    write(res," to: ",5);
    write(res,f2,strlen(f2));
    write(res,"\n",1);
    timeDiff(res,time1,time2);
    close(res);
    close(from);
    close(to);
    free(f1);
    free(f2);
    return 0;
}