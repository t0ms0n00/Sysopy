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

void printRowsWithSign(FILE* file,char* sign){
    fpos_t pos;
    fpos_t prev;
    pos.__pos=-1;
    char* line=calloc(256,sizeof(char));
    char* act_sign=calloc(1,sizeof(char));
    while(!feof(file)){
        bool found = false;
        pos.__pos++;
        prev=pos;
        fread(act_sign,1,1,file);
        while(strcmp(act_sign,"\n")!=0){
            if(strcmp(act_sign,sign)==0) found=true;
            pos.__pos++;
            fsetpos(file,&pos);
            fread(act_sign,1,1,file);
        }
        if(found){
            fsetpos(file,&prev);
            fread(line,1,pos.__pos-prev.__pos+1,file);
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
    FILE* file = fopen(filename,"r");
    if(file==NULL){
        perror(filename);
        exit(1);
    }
    clock_t time1=clock();
    printRowsWithSign(file,sign);
    clock_t time2=clock();
    FILE* res=fopen("pomiar_zad_2.txt","a");
    if(res==NULL){
        perror("pomiar_zad_2.txt");
        exit(1);
    }
    fprintf(res,"Test for file: %s sign: %s\n",filename,sign);
    timeDiff(res,time1,time2);
    fclose(res);
    fclose(file);
    free(filename);
    free(sign);
    return 0;
}