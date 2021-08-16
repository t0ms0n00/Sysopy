#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/// polecam zaczac od raportu, tam umiescilem sprawy organizacyjne dot. zadania

int signum = 10;

int main(int argc,char** argv){
    char command[255];
    char pid_num[5];
    strcat(command,"cat /proc/");
    sprintf(pid_num,"%d",getpid());
    strcat(command,pid_num);
    if(argc<2){
        perror("Missing argument\n");
        exit(1);
    }
    if(strcmp(argv[1],"ignore")==0){
        signal(signum,SIG_IGN);
        if(strcmp(argv[2],"false")==0) printf("Before exec, PID=%d\n",getpid());
        else printf("After exec, PID=%d\n",getpid());
        strcat(command,"/status | grep SigIgn");
        system(command);
        raise(signum);
    }
    else if(strcmp(argv[1],"mask")==0){
        sigset_t newmask;
        sigset_t oldmask;
        sigemptyset(&newmask);
        sigaddset(&newmask,signum);
        sigprocmask(SIG_BLOCK,&newmask,&oldmask);
        if(strcmp(argv[2],"false")==0) printf("Before exec, PID=%d\n",getpid());
        else printf("After exec, PID=%d\n",getpid());
        strcat(command,"/status | grep SigBlk");
        system(command);
        raise(signum);
    }
    else if(strcmp(argv[1],"pending")==0){
        sigset_t pending;
        sigset_t newmask;
        sigset_t oldmask;
        sigemptyset(&newmask);
        sigaddset(&newmask,signum);
        sigprocmask(SIG_BLOCK,&newmask,&oldmask);
        if(strcmp(argv[2],"false")==0) printf("Before exec, PID=%d\n",getpid());
        else printf("After exec, PID=%d\n",getpid());
        raise(signum);
        sigpending(&pending);
        if(sigismember(&pending,signum)) printf("Signal number %d is pending\n",signum);
        else printf("Signal number %d is NOT pending\n",signum);
    }
    else{
        perror("Argument doesn't fit into any category\n");
        exit(1);
    }
    printf("\n");
    if (strcmp(argv[2],"false")==0) execlp("./main_exec","./main_exec",argv[1],"true",NULL);
    return 0;
}