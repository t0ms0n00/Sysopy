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

void switchText(int from,int to, char* n1,char* n2){
    int patternLen=strlen(n1);
    int stepBack=patternLen-1;
    char* cut=calloc(patternLen,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    while(read(from,sign,1)>0){
        lseek(from,-1,SEEK_CUR);
        int check=read(from,cut,patternLen);
        if(check<patternLen){
            lseek(from,-strlen(cut),SEEK_CUR);
            write(to,cut,strlen(cut));
            break;
        }
        if(strcmp(cut,n1)==0){
            write(to,n2,strlen(n2));
        }
        else{
            write(to,&cut[0],1);
            lseek(from,-stepBack,SEEK_CUR);
        }
        free(cut);
        cut=calloc(patternLen,sizeof(char));
    }
    free(sign);
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
    int from = open(fileFrom,O_RDONLY);
    int to = open(fileTo,O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(from==-1){
        perror(fileFrom);
        exit(1);
    }
    if(to==-1){
        perror(fileTo);
        exit(1);
    }
    clock_t time1=clock();
    switchText(from,to,n1,n2);
    clock_t time2=clock();
    int res=open("pomiar_zad_4.txt",O_WRONLY);
    if(res==-1){
        perror("pomiar_zad_4.txt");
        exit(1);
    }
    lseek(res,0,SEEK_END);
    write(res,"Test copy from ",15);
    write(res,fileFrom,strlen(fileFrom));
    write(res," to ",4);
    write(res,fileTo,strlen(fileTo));
    write(res,", switch ",9);
    write(res,n1,strlen(n1));
    write(res," to ",4);
    write(res,n2,strlen(n2));
    write(res,"\n",1);
    timeDiff(res,time1,time2);
    close(res);
    close(from);
    close(to);
    free(fileFrom);
    free(fileTo);
    free(n1);
    free(n2);
    return 0;
}