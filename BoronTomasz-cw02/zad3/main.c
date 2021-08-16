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

void countEven(FILE* data){
    FILE* score=fopen("a.txt","a");
    fpos_t pos;
    pos.__pos=-1;
    int counter=0;
    char* sign=calloc(1,sizeof(char));
    while(!feof(data)){
        pos.__pos++;
        fsetpos(data,&pos);
        fread(sign,1,1,data);
        while(!feof(data) && strcmp(sign,"\n")!=0){
            pos.__pos++;
            fread(sign,1,1,data);
        }
        if(!feof(data)){
            pos.__pos--;
            fsetpos(data,&pos);
            fread(sign,1,1,data);
            int digit=atoi(sign);
            if(digit%2==0) counter++;
            pos.__pos++;
        }
    }
    fprintf(score,"Liczb parzystych jest %d",counter);
    fclose(score);
    free(sign);
}

void lastThreeBinDigitsEq(FILE* data){
    FILE* score=fopen("b.txt","a");
    fpos_t pos;
    fpos_t prev;
    pos.__pos=-1;
    char* number=calloc(11,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    while(!feof(data)){
        pos.__pos++;
        prev=pos;
        fsetpos(data,&pos);
        fread(sign,1,1,data);
        while(!feof(data) && strcmp(sign,"\n")!=0){
            pos.__pos++;
            fread(sign,1,1,data);
        }
        bool found=false;
        if(!feof(data)){
            pos.__pos-=2;
            fsetpos(data,&pos);
            fread(sign,1,1,data);
            int digit=atoi(sign);
            if(strcmp(sign,"\n")==0) digit=1;
            if(digit==0 || digit==7) found=true;
            pos.__pos+=2;
            if(found){
                fsetpos(data,&prev);
                fread(number,1,pos.__pos-prev.__pos+1,data);
                fprintf(score,"%s",number);
                free(number);
                number=calloc(11,sizeof(char));
            }
        }
    }
    fclose(score);
    free(sign);
    free(number);
}

bool isSquare(int a){
    int i=0;
    while(i*i<a) i++;
    if(i*i==a) return true;
    return false;
}

void squares(FILE* data){
    FILE* score=fopen("c.txt","a");
    fpos_t pos;
    fpos_t prev;
    pos.__pos=-1;
    char* number=calloc(11,sizeof(char));
    char* sign=calloc(1,sizeof(char));
    while(!feof(data)){
        pos.__pos++;
        prev=pos;
        fsetpos(data,&pos);
        fread(sign,1,1,data);
        while(!feof(data) && strcmp(sign,"\n")!=0){
            pos.__pos++;
            fread(sign,1,1,data);
        }
        if(!feof(data)){
            fsetpos(data,&prev);
            fread(number,1,pos.__pos-prev.__pos+1,data);
            int num=atoi(number);
            if(isSquare(num)) fprintf(score,"%s",number);
            free(number);
            number=calloc(11,sizeof(char));
        }
    }
    fclose(score);
    free(sign);
    free(number);
}

int main(){
    FILE* data = fopen("dane.txt","r");
    if(data==NULL){
        perror("dane.txt");
        exit(1);
    }
    FILE* msg=fopen("a.txt","w");
    fprintf(msg,"LIB Version\n");
    fclose(msg);
    msg=fopen("b.txt","w");
    fprintf(msg,"LIB Version\n");
    fclose(msg);
    msg=fopen("c.txt","w");
    fprintf(msg,"LIB Version\n");
    fclose(msg);
    clock_t time1=clock();
    countEven(data);
    fseek(data,0,0);
    lastThreeBinDigitsEq(data);
    fseek(data,0,0);
    squares(data);
    fseek(data,0,0);
    clock_t time2=clock();
    FILE* res=fopen("pomiar_zad_3.txt","a");
    if(res==NULL){
        perror("pomiar_zad_3.txt");
        exit(1);
    }
    fprintf(res,"Test\n");
    timeDiff(res,time1,time2);
    fclose(res);
    fclose(data);
    return 0;
}