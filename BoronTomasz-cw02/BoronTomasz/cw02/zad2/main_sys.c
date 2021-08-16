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

void printRowsWithSign(int file, char* sign){
    char* line=calloc(256,sizeof(char));
    char* act_sign=calloc(1,sizeof(char));
    int counter;
    while(read(file,act_sign,1)>0){
        counter=0;
        lseek(file,-1,SEEK_CUR);
        bool found=false;
        while(read(file,act_sign,1)>0 && strcmp(act_sign,"\n")!=0){
            if(strcmp(act_sign,sign)==0) found=true;
            counter++;
        }
        if(found){
            lseek(file,-counter-1,SEEK_CUR);
            read(file,line,counter+1);
            printf("%s",line);
            free(line);
            line=calloc(256,sizeof(char));
        }
    }
    free(line);
    free(act_sign);
}

int main(int argc,char** argv){
    char* filename=calloc(256,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    if(argc==1){
        printf("Sign=");
        scanf("%s",sign);
        printf("File=");
        scanf("%s",filename);
    }
    else if (argc==2){
        strcpy(sign,argv[1]);
        printf("File=");
        scanf("%s",filename);
    }
    else{
        strcpy(sign,argv[1]);
        strcpy(filename,argv[2]);
    }
    int f = open(filename,O_RDONLY);
    if(f==-1){
        perror(filename);
        exit(1);
    }
    clock_t time1=clock();
    printRowsWithSign(f,sign);
    clock_t time2=clock();
    int res=open("pomiar_zad_2.txt",O_WRONLY);
    if(res==-1){
        perror("pomiar_zad_2.txt");
        exit(1);
    }
    lseek(res,0,SEEK_END);
    write(res,"Test for file: ",15);
    write(res,filename,strlen(filename));
    write(res," sign: ",7);
    write(res,sign,strlen(sign));
    write(res,"\n",1);
    timeDiff(res,time1,time2);
    close(res);
    close(f);
    free(filename);
    free(sign);
    return 0;
}