#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int running = 1;
int sig_counter = 0;
int sig_catched = 0;

void handler(int sig_no){
    if(sig_no == 10 || sig_no == SIGRTMIN+1) sig_counter++;
    if(sig_no == 12 || sig_no == SIGRTMIN+2) running = 0;
}

void siginfo_handler(int sig_no, siginfo_t* sig_info, void* context){
    if(sig_no == 10) sig_counter++;
    else{
        running = 0;
        sig_catched = sig_info->si_value.sival_int;
    }
}

int main(int argc, char** argv){
    int catcher_pid = atoi(argv[1]);
    int amount_sig_to_send = atoi(argv[2]);
    char mode[10];
    strcpy(mode,argv[3]);
    if(amount_sig_to_send<=0){
        kill(catcher_pid,12);       /// w przypadku pomylki zamyka catchera
        perror("Min 1 signal must be sent\n");
        exit(1);
    }
    if(strcmp(mode,"kill")==0){
        for(int i=0;i<amount_sig_to_send;i++){
            kill(catcher_pid, 10);
        }
        kill(catcher_pid, 12);
    }
    else if(strcmp(mode,"sigqueue")==0){
        union sigval sigv;
        sigv.sival_int = 0;
        for(int i=0;i<amount_sig_to_send;i++){
            sigqueue(catcher_pid,10,sigv);
        }
        sigqueue(catcher_pid,12,sigv);
    }
    else if(strcmp(mode,"sigrt")==0){
        for(int i=0;i<amount_sig_to_send;i++){
            kill(catcher_pid, SIGRTMIN+1);
        }
        kill(catcher_pid, SIGRTMIN+2);
    }
    else{
        kill(catcher_pid,12);       /// w przypadku pomylki zamyka catchera
        perror("Mode should be kill, sigqueue or sigrt\n");
        exit(1);
    }
    /// ---------------------------------------------------------------------
    sigset_t newmask;
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
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_mask = newmask;
    act.sa_flags = 0;
    struct sigaction act_info;
    act_info.sa_mask = newmask;
    act_info.sa_flags = SA_SIGINFO;
    act_info.sa_sigaction = siginfo_handler;
    if(strcmp(mode,"sigrt")==0) {
        sigaction(SIGRTMIN+1, &act, NULL);
        sigaction(SIGRTMIN+2, &act, NULL);
    }
    else if(strcmp(mode,"kill")==0){
        sigaction(10, &act, NULL);
        sigaction(12, &act, NULL);
    }
    else{
        sigaction(10, &act_info, NULL);
        sigaction(12, &act_info, NULL);
    }
    while(running){
        sigsuspend(&newmask);
    }
    printf("Sent %s signals to catcher\n",argv[2]);
    if(strcmp(mode,"sigqueue")==0) printf("Catcher catched %d signals\n",sig_catched);
    printf("Received %d signals from catcher\n",sig_counter);
    return 0;
}