#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>

#include <dlfcn.h>

#include "array_blocks.h"

double realTimeDiff(clock_t t1,clock_t t2){
    return ((double)(t2 - t1)/CLOCKS_PER_SEC);
}

double sysTimeDiff(clock_t t1, clock_t t2){
    return ((double)(t2 - t1)/sysconf(_SC_CLK_TCK));
}

void writeResult(FILE* f,clock_t r_start, clock_t r_end, struct tms* s_start, struct tms* s_end){
    fprintf(f, "Real time: %f\n", realTimeDiff(r_start, r_end));
    fprintf(f, "User time: %f\n", sysTimeDiff(s_start->tms_utime, s_end->tms_utime));
    fprintf(f, "System time: %f\n\n", sysTimeDiff(s_start->tms_stime, s_end->tms_stime));

    printf("Real time: %f\n", realTimeDiff(r_start, r_end));
    printf("User time: %f\n", sysTimeDiff(s_start->tms_utime, s_end->tms_utime));
    printf("System time: %f\n", sysTimeDiff(s_start->tms_stime, s_end->tms_stime));
}

int main(int argc, char** argv) {
    FILE* result;
    result=fopen("results3b.txt","a");
    struct ArrayOfBlocks arr;
    void* handle =dlopen("./libarray_blocks.so",RTLD_LAZY);
    struct ArrayOfBlocks (*createMainArray)(int) = dlsym(handle,"createMainArray");
    struct Block (*mergeFiles)(char*,char*)= dlsym(handle,"mergeFiles");
    int (*addNewBlockWithNoFile)(struct ArrayOfBlocks,struct Block)=dlsym(handle,"addNewBlockWithNoFile");
    void (*removeBlock)(struct ArrayOfBlocks,int)=dlsym(handle,"removeBlock");
    void (*removeRow)(struct ArrayOfBlocks,int,int)=dlsym(handle,"removeRow");
    /// void (*printMerged)(struct ArrayOfBlocks)=dlsym(handle,"printMerged"); // odkomentowac jesli chce sie wypisac strukture na konsole
    struct tms** sysTimes=(struct tms**) calloc(argc,sizeof(struct tms*));
    for(int i=0;i<argc;i++){
        sysTimes[i]=(struct tms*) calloc(1,sizeof(struct tms));
    }
    clock_t* time=(clock_t*) calloc(argc,sizeof(clock_t));
    int currClockIndex=0;
    fprintf(result, "------------------------------------------------\n");
    fprintf(result, "Test %s\n",argv[1]);
    for(int i=2;i<argc;){
        time[currClockIndex]=clock();
        times(sysTimes[currClockIndex]);
        currClockIndex+=1;
        if(strcmp(argv[i],"create_table")==0){
            int arr_size=atoi(argv[i+1]);
            arr=(*createMainArray)(arr_size);
            printf("Array of blocks of size %d was created\n",arr_size);
            fprintf(result, "Array of blocks of size %d was created\n",arr_size);
            i+=2;
        }
        else if(strcmp(argv[i],"merge_files")==0){
            int counter=i;
            int pairCounter=0;
            char* file1=(char*) calloc(50,sizeof(char));
            char* file2=(char*) calloc(50,sizeof(char));
            while(counter+1<argc && strcmp(argv[counter+1],"create_table")!=0
                    && strcmp(argv[counter+1],"remove_block")!=0 && strcmp(argv[counter+1],"remove_row")!=0){
                    
                    counter++;
                    strcpy(file1,argv[counter]);
                    counter++;
                    strcpy(file2,argv[counter]);
                    pairCounter++;
                    struct Block block=(*mergeFiles)(file1,file2);
                    arr.lastPos=(*addNewBlockWithNoFile)(arr,block);
                    printf("New block created from %s and %s added\n",file1,file2);
                    ///fprintf(result, "New block created from %s and %s added\n",file1,file2); /// dla czytelnosci wrzucam do pliku tylko ogolna informacje
                }
            fprintf(result, "Merged %d pairs of files\n",pairCounter);
            i=counter+1;
        }
        else if(strcmp(argv[i],"remove_block")==0){
            int index=atoi(argv[i+1]);
            (*removeBlock)(arr,index);
            printf("Block %d deleted\n",index);
            fprintf(result, "Block %d deleted\n",index);
            i+=2;
        }
        else if(strcmp(argv[i],"remove_row")==0){
            int blockIndex=atoi(argv[i+1]);
            int rowIndex=atoi(argv[i+2]);
            (*removeRow)(arr,blockIndex,rowIndex);
            printf("Row %d from block %d deleted\n",rowIndex,blockIndex);
            fprintf(result, "Row %d from block %d deleted\n",rowIndex,blockIndex);
            i+=3;
        }
        else{
            printf("Bad number %d argument %s\n",i,argv[i]);
            exit(1);
        }
        time[currClockIndex]=clock();
        times(sysTimes[currClockIndex]);
        writeResult(result,time[currClockIndex-1],time[currClockIndex],sysTimes[currClockIndex-1],sysTimes[currClockIndex]);
        currClockIndex+=1;
    }
    fclose(result);
    dlclose(handle);
    ///(*)printMerged(arr);    ///odkomentowac aby sprawdzic wynik wszystkich operacji
    return 0;
}