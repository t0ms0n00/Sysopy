#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/// dla siginfo najlepszy efekt daje wysylanie sygnalow z konsoli, specjalnie wypisuje pid procesu na poczatku dla wygody, 
/// reszta testow flag dziala automatycznie

int breaker = 1;
int sigusr1_count=0;
int fib_2_back=0;
int fib_1_back=1;

void increment_sigusr1(){
    sigusr1_count++;
}

int get_next_fib(){
    int now = fib_2_back+fib_1_back;
    fib_2_back = fib_1_back;
    fib_1_back = now;
    return now;
}

void handler(int sig_no, siginfo_t* sig_info, void* context){
    if(sig_no == 2){
        printf("Break the loop\n");
        breaker=0;
    }
    else if(sig_no == 10){
        increment_sigusr1();
        printf("Sigusr1 sended %d times\n",sigusr1_count);
    }
    else{
        printf("Next Fibonacci number is %d\n",get_next_fib());
    }
    printf("PID=%ld, signal number %ld \n",(long)sig_info->si_pid,(long)sig_info->si_signo);
}

void handler_sigchld(int sig_no){
    if(sig_no == 17) printf("Process pid=%d received signal %d\n",getpid(),sig_no);
}

int main(int argc,char** argv){
    printf("Process has pid %d\n",getpid());
    if(argc<2){
        perror("Missing argument\n");
        exit(1);
    }
    if(strcmp(argv[1],"siginfo")==0){
        printf("10 and 12 for execute some functions, 2 for break the loop\n");
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask,2);
        sigdelset(&mask,10);
        sigdelset(&mask,12);
        sigprocmask(SIG_SETMASK,&mask,NULL);
        struct sigaction* sig_act = calloc(1,sizeof(struct sigaction));
        sig_act->sa_sigaction = &handler;
        sig_act->sa_flags = SA_SIGINFO;
        sigaction(12,sig_act,NULL);
        sigaction(10,sig_act,NULL);
        sigaction(2,sig_act,NULL);
        while(breaker){
            pause();
        }
        free(sig_act);
    }
    else if(strcmp(argv[1],"nocldstop")==0){
        printf("Waiting 5s before next signal\n");
        pid_t child_pid;
        struct sigaction* sig_act = calloc(1,sizeof(struct sigaction));
        sig_act->sa_handler = &handler_sigchld;
        sigaction(17,sig_act,NULL);
        child_pid = fork();
        if(child_pid == 0){
            while(1){};
        }
        else{
            sleep(5);
            printf("Sending SIGSTOP to child\n");
            kill(child_pid, 19);
            wait(NULL);
        }
        printf("Setting flag SA_NOCLDSTOP\n");
        sig_act->sa_flags = SA_NOCLDSTOP;
        sigaction(17,sig_act,NULL);
        if(child_pid == 0){
            while(1){};
        }
        else{
            sleep(5);
            printf("Sending SIGSTOP to child\n");
            kill(child_pid, 19);
            sleep(5);
            printf("Sending SIGKILL to child\n");
            kill(child_pid, 9);
            wait(NULL);
        }
        free(sig_act);
    }
    else if(strcmp(argv[1],"nocldwait")==0){
        pid_t child_pid;
        struct sigaction* sig_act = calloc(1,sizeof(struct sigaction));
        sig_act->sa_handler = &handler_sigchld;
        sigaction(17,sig_act,NULL);
        child_pid = fork();
        if (child_pid == 0) return 0;
        else{
            sleep(5);
            char command[256];
            char chpid[5];
            printf("Status for child process pid=%d\n",child_pid);
            strcpy(command,"ps -eo pid,stat | grep ");
            sprintf(chpid,"%d",child_pid);
            strcat(command, chpid);
            system(command);
        }
        wait(NULL);
        printf("Setting flag SA_NOCLDWAIT\n");
        sig_act->sa_flags = SA_NOCLDWAIT;
        sigaction(17,sig_act,NULL);
        child_pid = fork();
        if (child_pid == 0) return 0;
        else{
            sleep(5);
            char command[256];
            char chpid[5];
            printf("Status for child process pid=%d\n",child_pid);
            strcpy(command,"ps -eo pid,stat | grep ");
            sprintf(chpid,"%d",child_pid);
            strcat(command, chpid);
            int err = system(command);
            if(err == -1) printf("Child process was terminated, not transformed to zombie\n");
            else printf("Child process transformed to zombie\n");
        }
        wait(NULL);
        free(sig_act);
    }
    else{
        perror("Bad argument\n");
        exit(1);
    }
    printf("\n");
    return 0;
}