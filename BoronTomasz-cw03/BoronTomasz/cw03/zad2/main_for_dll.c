#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>
#include <dlfcn.h>

#include "array_blocks.h"

double realTimeDiff(clock_t t1,clock_t t2){
    return ((double)(t2 - t1)/CLOCKS_PER_SEC);
}

double sysTimeDiff(clock_t t1, clock_t t2){
    return ((double)(t2 - t1)/sysconf(_SC_CLK_TCK));
}

void writeResult(FILE* f,clock_t r_start, clock_t r_end, struct tms* s_start, struct tms* s_end){
    fprintf(f, "------------------------------------------------\n");
    fprintf(f, "Test\n");

    fprintf(f, "Real time: %f\n", realTimeDiff(r_start, r_end));
    fprintf(f, "User time: %f\n", sysTimeDiff(s_start->tms_utime, s_end->tms_utime));
    fprintf(f, "System time: %f\n\n", sysTimeDiff(s_start->tms_stime, s_end->tms_stime));

    printf("Real time: %f\n", realTimeDiff(r_start, r_end));
    printf("User time: %f\n", sysTimeDiff(s_start->tms_utime, s_end->tms_utime));
    printf("System time: %f\n", sysTimeDiff(s_start->tms_stime, s_end->tms_stime));
}

int main(int argc, char** argv){
    FILE* result;
    result=fopen("results2.txt","a");
    int pairs = (argc-1)/2;
    void* handle =dlopen("./libarray_blocks.so",RTLD_LAZY);
    struct ArrayOfBlocks (*createMainArray)(int) = dlsym(handle,"createMainArray");
    struct Block (*mergeFiles)(char*,char*)= dlsym(handle,"mergeFiles");
    void (*removeBlock)(struct ArrayOfBlocks,int)=dlsym(handle,"removeBlock");
    int (*addNewBlock)(struct ArrayOfBlocks array,char* tmpFileName)=dlsym(handle,"addNewBlock");
    struct tms** sysTimes=(struct tms**) calloc(2,sizeof(struct tms*));
    for(int i=0;i<2;i++){
        sysTimes[i]=(struct tms*) calloc(1,sizeof(struct tms));
    }
    clock_t* time=(clock_t*) calloc(2,sizeof(clock_t));
    struct ArrayOfBlocks arr = (*createMainArray)(pairs);
    pid_t child_pid;
    time[0]=clock();
    times(sysTimes[0]);
    for(int i=0;i<pairs;i++){
        child_pid = fork();
        wait(NULL);
        if(child_pid == 0){
            /// printf("Teraz dziala proces potomny PID=%d\n",getpid());
            FILE* tmp = fopen("tmp.txt","w");
            if(tmp == NULL){
                perror("tmp.txt");
                exit(1);
            }
            struct Block block = (*mergeFiles)(argv[2*i+1],argv[2*i+2]);
            for(int j=0;j<block.last;j++){
                fprintf(tmp,"%s",block.rows[j]);
            }
            fclose(tmp);
            exit(0);
        }
        /// printf("Teraz dziala proces macierzysty PID=%d\n",getpid());
        arr.lastPos=addNewBlock(arr,"tmp.txt");
    }
    time[1]=clock();
    times(sysTimes[1]);
    writeResult(result,time[0],time[1],sysTimes[0],sysTimes[1]);
    fclose(result);
    /// (*)printMerged(arr);
    for(int i=arr.lastPos;i>=0;i--){
        removeBlock(arr,i);
    }
    free(arr.blocks);
    free(time);
    free(sysTimes[0]);
    free(sysTimes[1]);
    free(sysTimes);
    dlclose(handle);
    return 0;
}