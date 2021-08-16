#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>

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

/*

1. Powtórzona tabela z laboratorium pierwszego:

    | static   |   shared   |  dynamic
-------------------------------------------
-O0 |  0.711   |   1.197    |    0.620
-------------------------------------------
-O2 |  0.949   |   0.496    |    0.548
-------------------------------------------
-Os |  0.704   |   0.583    |    0.615

2. Tabela pomiarów dla tego laboratorium (skupiam się na tym samym teście z make long_time)

    | static   |   shared   |  dynamic
-------------------------------------------
-O0 |  0.662   |   0.663    |    0.650
-------------------------------------------
-O2 |  0.642   |   0.645    |    0.671
-------------------------------------------
-Os |  0.680   |   0.718    |    0.699

Można zauważyć, że czasy się bardziej ustabilizowały, jednak nie można jednoznacznie stwierdzić,
czy to podejście daje lepsze wyniki (patrząc np na komórki shared -O0 oraz shared -O2).

W pozostałych testach zaobserwowałem nieco lepsze wyniki dla biblioteki dołączanej dynamicznie.

Ciekawie było w przypadku testów na poziomie optymalizacji -O2, gdzie to biblioteka łączona statycznie spisała się lepiej
(szczególnie w dłuższych testach).

Dodatkowo mogę stwierdzić, że poziom -O0 dla większej liczby testów spisał się trochę gorzej niż pozostałe (z powyższej tabelki 
to nie wynika, natomiast opieram się na zebranych danych w pliku results2.txt)

*/

int main(int argc, char** argv){
    FILE* result;
    result=fopen("results2.txt","a");
    struct tms** sysTimes=(struct tms**) calloc(2,sizeof(struct tms*));
    for(int i=0;i<2;i++){
        sysTimes[i]=(struct tms*) calloc(1,sizeof(struct tms));
    }
    clock_t* time=(clock_t*) calloc(2,sizeof(clock_t));
    int pairs = (argc-1)/2;
    struct ArrayOfBlocks arr = createMainArray(pairs);
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
            struct Block block = mergeFiles(argv[2*i+1],argv[2*i+2]);
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
    /// printMerged(arr);
    for(int i=arr.lastPos;i>=0;i--){
        removeBlock(arr,i);
    }
    free(arr.blocks);
    free(time);
    free(sysTimes[0]);
    free(sysTimes[1]);
    free(sysTimes);
    return 0;
}