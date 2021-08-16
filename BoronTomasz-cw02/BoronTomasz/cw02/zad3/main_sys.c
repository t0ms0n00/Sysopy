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

void countEven(int data){
    int score=open("a.txt",O_WRONLY, 0666);
    write(score,"SYS Version\n",12);
    int counter=0;
    char* sign=calloc(1,sizeof(char));
    while(read(data,sign,1)>0){
        lseek(data,-1,SEEK_CUR);
        while(read(data,sign,1)>0 && strcmp(sign,"\n")!=0){
            // do nothing
        }
        lseek(data,-2,SEEK_CUR);
        read(data,sign,1);
        int digit=atoi(sign);
        if(digit%2==0) counter++;
        lseek(data,1,SEEK_CUR);
    }
    write(score,"Liczb parzystych jest ",22);
    char* result=calloc(11,sizeof(char));
    snprintf(result,11,"%d",counter);
    write(score,result,strlen(result));
    free(result);
    close(score);
    free(sign);
}

void lastThreeBinDigitsEq(int data){
    int score=open("b.txt",O_WRONLY, 0666);
    write(score,"SYS Version\n",12);
    char* number=calloc(11,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    int num_len;
    while(read(data,sign,1)>0){
        lseek(data,-1,SEEK_CUR);
        num_len=0;
        while(read(data,sign,1)>0 && strcmp(sign,"\n")!=0){
            num_len++;
        }
        bool found=false;
        lseek(data,-3,SEEK_CUR);
        read(data,sign,1);
        lseek(data,2,SEEK_CUR);
        int digit=atoi(sign);
        if(strcmp(sign,"\n")==0) digit=1;
        if(digit==0 || digit==7) found=true;
        if(found){
            lseek(data,-num_len-1,SEEK_CUR);
            read(data,number,num_len+1);
            write(data,number,strlen(number));
            free(number);
            number=calloc(11,sizeof(char));
        }
    }
    close(score);
    free(sign);
    free(number);
}

bool isSquare(int a){
    int i=0;
    while(i*i<a) i++;
    if(i*i==a) return true;
    return false;
}

void squares(int data){
    int score=open("c.txt",O_WRONLY, 0666);
    write(score,"SYS Version\n",12);
    char* number=calloc(11,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    int num_len;
    while(read(data,sign,1)>0){
        lseek(data,-1,SEEK_CUR);
        num_len=0;
        while(read(data,sign,1)>0 && strcmp(sign,"\n")!=0){
            num_len++;
        }
        lseek(data,-num_len-1,SEEK_CUR);
        read(data,number,num_len+1);
        int num=atoi(number);
        if(isSquare(num)) write(score,number,strlen(number));
        free(number);
        number=calloc(11,sizeof(char));
    }
    close(score);
    free(sign);
    free(number);
}

int main(int argc,char** argv){
    int data = open("dane.txt",O_RDONLY);
    if(data==-1){
        perror("dane.txt");
        exit(1);
    }
    clock_t time1=clock();
    countEven(data);
    lseek(data,0,SEEK_SET);
    lastThreeBinDigitsEq(data);      
    lseek(data,0,SEEK_SET);
    squares(data);
    lseek(data,0,SEEK_SET);
    clock_t time2=clock();
    int res=open("pomiar_zad_3.txt",O_WRONLY);
    if(res==-1){
        perror("pomiar_zad_3.txt");
        exit(1);
    }
    lseek(res,0,SEEK_END);
    write(res,"Test\n",5);
    timeDiff(res,time1,time2);
    close(res);
    close(data);
    return 0;
}