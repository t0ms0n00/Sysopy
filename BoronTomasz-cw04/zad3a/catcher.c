#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int sig_counter = 0;
int running = 1;
pid_t sender_pid;

void handler(int sig_no){
    sig_counter++;
}

void siginfo_handler(int sig_no, siginfo_t* sig_info, void* context){
    running = 0;
    sender_pid = sig_info->si_pid;
}

int main(int argc, char** argv){
    if(argc<2){
        perror("You should set mode: kill, sigqueue or sigrt\n");
        exit(1);
    }
    if(strcmp(argv[1],"kill") != 0 && strcmp(argv[1],"sigqueue") != 0 && strcmp(argv[1],"sigrt") != 0){
        perror("Mode should be kill, sigqueue or sigrt\n");
        exit(1);
    }
    char mode[10];
    strcpy(mode,argv[1]);
    sigset_t newmask;
    pid_t pid = getpid();
    sigfillset(&newmask);
    if(strcmp(mode,"sigrt")==0){
        sigdelset(&newmask,SIGRTMIN+1);
        sigdelset(&newmask,SIGRTMIN+2);
    }
    else{
        sigdelset(&newmask,10);
        sigdelset(&newmask,12);
    }
    sigprocmask(SIG_SETMASK,&newmask,NULL);
    printf("Process PID %d\n",pid);
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_mask = newmask;
    act.sa_flags = 0;
    if(strcmp(mode,"sigrt")==0) sigaction(SIGRTMIN+1, &act, NULL);
    else sigaction(10, &act, NULL);
    struct sigaction act_info;
    act_info.sa_mask = newmask;
    act_info.sa_flags = SA_SIGINFO;
    act_info.sa_sigaction = siginfo_handler;
    if(strcmp(mode,"sigrt")==0) sigaction(SIGRTMIN+2, &act_info, NULL);
    else sigaction(12, &act_info, NULL);
    while(running){
        sigsuspend(&newmask);
    }
    printf("Received %d SIGUSR1 signals from sender\n",sig_counter);
    /// ----------------------------------------------------------------------
    printf("Sending back received signals to sender after 5s\n");
    sleep(5);
    if(strcmp(mode,"kill")==0){
        for(int i=0;i<sig_counter;i++){
            kill(sender_pid, 10);
        }
        kill(sender_pid, 12);
    }
    else if(strcmp(mode,"sigqueue")==0){
        union sigval sigv;
        sigv.sival_int = 0;
        for(int i=0;i<sig_counter;i++){
            sigqueue(sender_pid,10,sigv);
        }
        sigv.sival_int = sig_counter;
        sigqueue(sender_pid,12,sigv);
    }
    else{
        for(int i=0;i<sig_counter;i++){
            kill(sender_pid, SIGRTMIN+1);
        }
        kill(sender_pid, SIGRTMIN+2);
    }
    return 0;
}