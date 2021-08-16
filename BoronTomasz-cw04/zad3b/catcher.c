#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int sig_counter = 0;
int running = 1;
pid_t sender_pid;
char mode[10];

void siginfo_handler(int sig_no, siginfo_t* sig_info, void* context){
    union sigval sval;
    sender_pid = sig_info->si_pid;
    if(sig_no == 10){
        sig_counter++;
        if(strcmp(mode,"sigqueue")==0) sigqueue(sender_pid, 10, sval);
        else kill(sender_pid, 10);
    }
    else if(sig_no == SIGRTMIN+1){
        sig_counter++;
        kill(sender_pid, SIGRTMIN+1);
    }
    else{
        running = 0;
    }
}

void handler_wait(int sig_no){}

int main(int argc, char** argv){
    if(argc<2){
        perror("You should set mode: kill, sigqueue or sigrt\n");
        exit(1);
    }
    if(strcmp(argv[1],"kill") != 0 && strcmp(argv[1],"sigqueue") != 0 && strcmp(argv[1],"sigrt") != 0){
        perror("Mode should be kill, sigqueue or sigrt\n");
        exit(1);
    }
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
    struct sigaction act_info;
    act_info.sa_mask = newmask;
    act_info.sa_flags = SA_SIGINFO;
    act_info.sa_sigaction = siginfo_handler;
    if(strcmp(mode,"sigrt")==0){
        sigaction(SIGRTMIN+1, &act_info, NULL);
        sigaction(SIGRTMIN+2, &act_info, NULL);
    }
    else{
        sigaction(10, &act_info, NULL);
        sigaction(12, &act_info, NULL);
    }
    while(running){
        sigsuspend(&newmask);
    }
    printf("Received %d SIGUSR1 signals from sender\n",sig_counter);
    /// ----------------------------------------------------------------------
    printf("Sending back received signals to sender after 5s\n");
    sleep(5);
    sigset_t wait;
    sigfillset(&wait);
    if(strcmp(mode,"sigrt")==0){
        sigdelset(&wait, SIGRTMIN+1);
    }
    else{
        sigdelset(&wait, 10);
    }
    struct sigaction act_wait;
    act_wait.sa_handler = handler_wait;
    act_wait.sa_mask = wait;
    act_wait.sa_flags = 0;
    if(strcmp(mode,"sigrt")==0){
        sigaction(SIGRTMIN+1, &act_wait, NULL);
    }
    else{
        sigaction(10, &act_wait, NULL);
    }
    if(strcmp(mode,"kill")==0){
        for(int i=0;i<sig_counter;i++){
            kill(sender_pid, 10);
            sigsuspend(&wait);
        }
        kill(sender_pid, 12);
    }
    else if(strcmp(mode,"sigqueue")==0){
        union sigval sigv;
        sigv.sival_int = 0;
        for(int i=0;i<sig_counter;i++){
            sigqueue(sender_pid,10,sigv);
            sigsuspend(&wait);
        }
        sigv.sival_int = sig_counter;
        sigqueue(sender_pid,12,sigv);
    }
    else{
        for(int i=0;i<sig_counter;i++){
            kill(sender_pid, SIGRTMIN+1);
            sigsuspend(&wait);
        }
        kill(sender_pid, SIGRTMIN+2);
    }
    return 0;
}