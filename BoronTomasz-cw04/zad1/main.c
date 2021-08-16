#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/// polecam zaczac od raportu, tam umiescilem sprawy organizacyjne dot. zadania

int signum = 10;

void handler(int sig_no){
    printf("Signal number %d was received\n",sig_no);
}

int main(int argc,char** argv){
    pid_t child_pid;
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
        printf("Parent process, PID=%d\n",getpid());
        strcat(command,"/status | grep SigIgn");
        system(command);
        raise(signum);
        child_pid = fork();
        wait(NULL);
        if(child_pid==0){
            char command[255];
            char pid_num[5];
            printf("Child process, PID=%d\n",getpid());
            strcat(command,"cat /proc/");
            sprintf(pid_num,"%d",getpid());
            strcat(command,pid_num);
            strcat(command,"/status | grep SigIgn");
            system(command);
            raise(signum);
            exit(0);
        }
    }
    else if(strcmp(argv[1],"handler")==0){
        signal(signum,handler);
        printf("Parent process, PID=%d\n",getpid());
        raise(signum);
        child_pid = fork();
        wait(NULL);
        if(child_pid==0){
            printf("Child process, PID=%d\n",getpid());
            raise(signum);
            exit(0);
        }
    }
    else if(strcmp(argv[1],"mask")==0){
        sigset_t newmask;
        sigset_t oldmask;
        sigemptyset(&newmask);
        sigaddset(&newmask,signum);
        sigprocmask(SIG_BLOCK,&newmask,&oldmask);
        printf("Parent process, PID=%d\n",getpid());
        strcat(command,"/status | grep SigBlk");
        system(command);
        raise(signum);
        child_pid = fork();
        wait(NULL);
        if(child_pid==0){
            char command[255];
            char pid_num[5];
            printf("Child process, PID=%d\n",getpid());
            strcat(command,"cat /proc/");
            sprintf(pid_num,"%d",getpid());
            strcat(command,pid_num);
            strcat(command,"/status | grep SigBlk");
            system(command);
            raise(signum);
            exit(0);
        }
    }
    else if(strcmp(argv[1],"pending")==0){
        sigset_t pending;
        sigset_t newmask;
        sigset_t oldmask;
        sigemptyset(&newmask);
        sigaddset(&newmask,signum);
        sigprocmask(SIG_BLOCK,&newmask,&oldmask);
        printf("Parent process, PID=%d\n",getpid());
        raise(signum);
        sigpending(&pending);
        if(sigismember(&pending,signum)) printf("Signal number %d is pending\n",signum);
        else printf("Signal number %d is NOT pending\n",signum);
        child_pid = fork();
        wait(NULL);
        if(child_pid==0){
            printf("Child process, PID=%d\n",getpid());
            sigset_t pending;
            sigset_t newmask;
            sigset_t oldmask;
            sigemptyset(&newmask);
            sigaddset(&newmask,signum);
            sigprocmask(SIG_BLOCK,&newmask,&oldmask);
            sigpending(&pending);
            if(sigismember(&pending,signum)) printf("Signal number %d is pending\n",signum);
            else printf("Signal number %d is NOT pending\n",signum);            
            exit(0);
        }
    }
    else{
        perror("Argument doesn't fit into any category\n");
        exit(1);
    }
    printf("\n");
    return 0;
}