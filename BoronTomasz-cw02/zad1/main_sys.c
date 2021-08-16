#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

void timeDiff(int f,clock_t start,clock_t end){
    double seconds=(double)((end-start)/CLOCKS_PER_SEC);
    char* time=calloc(10,sizeof(char));
    write(f,"Time for sys: ",14);
    snprintf(time,10,"%f",seconds);
    write(f,time,8);
    write(f,"\n",1);
    free(time);
}

void showRoundRobin(int f1, int f2){
    char* line=calloc(256,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    int counter;
    while(read(f1,sign,1)>0 && read(f2,sign,1)>0){
        counter=0;
        lseek(f1,-1,SEEK_CUR);
        lseek(f2,-1,SEEK_CUR);
        while(read(f1,sign,1)>0 && strcmp(sign,"\n")!=0){
            counter++;
        }
        lseek(f1,-counter-1,SEEK_CUR);
        read(f1,line,counter+1);
        printf("%s",line);
        free(line);
        line=calloc(256,sizeof(char));
        counter=0;
        while(read(f2,sign,1)>0 && strcmp(sign,"\n")!=0){
            counter++;
        }
        lseek(f2,-counter-1,SEEK_CUR);
        read(f2,line,counter+1);
        printf("%s",line);
        free(line);
        line=calloc(256,sizeof(char));
    }
    while(read(f1,sign,1)!=0){
        counter=0;
        lseek(f1,-1,SEEK_CUR);
        while(read(f1,sign,1)>0 && strcmp(sign,"\n")!=0){
            counter++;
        }
        lseek(f1,-counter-1,SEEK_CUR);
        read(f1,line,counter+1);
        printf("%s",line);
        free(line);
        line=calloc(256,sizeof(char));
    }
    while(read(f2,sign,1)!=0){
        counter=0;
        lseek(f2,-1,SEEK_CUR);
        while(read(f2,sign,1)>0 && strcmp(sign,"\n")!=0){
            counter++;
        }
        lseek(f2,-counter-1,SEEK_CUR);
        read(f2,line,counter+1);
        printf("%s",line);
        free(line);
        line=calloc(256,sizeof(char));
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
    int f1 = open(name1,O_RDONLY);
    int f2 = open(name2,O_RDONLY);
    if(f1==-1){
        perror(name1);
        exit(1);
    }
    if(f2==-1){
        perror(name2);
        exit(1);
    }
    clock_t time1=clock();
    showRoundRobin(f1,f2);
    clock_t time2=clock();
    int res=open("pomiar_zad_1.txt",O_WRONLY);
    if(res==-1){
        perror("pomiar_zad_1.txt");
        exit(1);
    }
    lseek(res,0,SEEK_END);
    write(res,"Test for files: ",16);
    write(res,name1,strlen(name1));
    write(res," : ",3);
    write(res,name2,strlen(name2));
    write(res,"\n",1);
    timeDiff(res,time1,time2);
    close(res);
    close(f1);
    close(f2);
    free(name1);
    free(name2);
    return 0;
}