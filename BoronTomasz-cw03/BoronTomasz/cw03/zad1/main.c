#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv){
    int n = 0;
    if(argc == 1){
        printf("Nie podano liczby procesow do utworzenia\n");
    }
    else{
        n = atoi(argv[1]);
    }
    while(n<=0){
        printf("Podaj liczbe procesow: ");
        scanf("%d",&n);
        if(n<=0) printf("Liczba procesow musi byc wartoscia dodatnia\n");
    }
    pid_t child_pid;
    for(int i=0;i<n;i++){
        child_pid = fork();
        if(child_pid == 0){
            printf("Napis pochodzi z procesu o PID=%d\n",(int) getpid());
            break;
        }
    }
    return 0;
}