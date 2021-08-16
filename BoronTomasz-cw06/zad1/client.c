#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "message.h"

int running = 1;
int server_id;
int client_id;
int client_s_id;
int comrade_id;

void stop_action(){
    my_msgbuf quit;
    quit.mtype = STOP;
    strcpy(quit.mtext, "STOP");
    quit.sender_id = client_s_id;
    if(msgsnd(server_id, &quit, MSGSIZE, IPC_NOWAIT) == -1){
        perror("Problem during sending command to server\n");
        exit(1);
    }
}

void SIGINT_handler(int sig_no){
    printf("Received SIGINT\n");
    stop_action();
    running = 0;
}

void choose_type(my_msgbuf* msg, char command[]){
    if (strcmp(command,"LIST\n") == 0){
        msg->mtype = LIST;
    }
    else if(strcmp(command,"DISCONNECT\n") == 0){
        msg->mtype = DISCONNECT;
    }
    else{
        msg->mtype = CONNECT;
    }
}

int main(int argc, char** argv){

    comrade_id = -1;

    /// handle SIGINT

    struct sigaction act;
    act.sa_handler = SIGINT_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    /// creating client ipc queue

    if(argc != 2){
        perror("Format: ./client [server queue id]");
        exit(1);
    }
    server_id = atoi(argv[1]);
    printf("Client start his job\n");
    client_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if(client_id == -1){
        perror("Error during creating new msg queue\n");
        exit(1);
    }
    printf("Client queue id %d\n", client_id);
    
    /// init

    printf("Sending INIT to server's queue\n");
    char* message = calloc(10, sizeof(char));
    sprintf(message, "INIT %d", client_id);
    my_msgbuf msg;
    msg.mtype = INIT;
    msg.sender_id = client_id;
    strcpy(msg.mtext, message);
    if(msgsnd(server_id, &msg, MSGSIZE, 0) == -1){
        perror("Problem during sending init to server\n");
        exit(1);
    }
    my_msgbuf msg_received;
    if (msgrcv(client_id, &msg_received, MSGSIZE, 0, 0) != -1){
        printf("%s", msg_received.mtext);
    }
    strtok(msg_received.mtext, " ");
    strtok(NULL, " ");
    strtok(NULL, " ");
    char* client_num = strtok(NULL, " ");
    client_s_id = atoi(client_num);

    /// scanning commands

    while(running){
        char command[50];
        fd_set set;
        struct timeval time;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        time.tv_sec = 1;
        time.tv_usec = 0;
        my_msgbuf response;
        if (msgrcv(client_id, &response, MSGSIZE, 0, IPC_NOWAIT) != -1){
            printf("%s", response.mtext);
            if(comrade_id == -1) comrade_id = response.comrade_id;
        }
        if(select(FD_SETSIZE, &set, NULL, NULL, &time) == 1){
            fgets(command, 50, stdin);
            if(strcmp(command, "DISCONNECT\n") == 0){
                my_msgbuf disconnect;
                choose_type(&disconnect, command);
                msg.sender_id = client_s_id;
                strcpy(msg.mtext, command);
                if(msgsnd(server_id, &msg, MSGSIZE, IPC_NOWAIT) == -1){
                    perror("Problem during sending command to server\n");
                    exit(1);
                }
                comrade_id = -1;
                continue;
            }
            if(strcmp(command, "STOP\n") == 0){
                stop_action();
                break;
            }
            my_msgbuf msg;
            choose_type(&msg, command);
            msg.sender_id = client_s_id;
            msg.comrade_id = client_id;
            strcpy(msg.mtext, command);
            if(comrade_id == -1){
                if(msgsnd(server_id, &msg, MSGSIZE, IPC_NOWAIT) == -1){
                    perror("Problem during sending command to server\n");
                    exit(1);
                }
            }
            else{
                if(msgsnd(comrade_id, &msg, MSGSIZE, IPC_NOWAIT) == -1){
                    perror("Problem during sending command to chatter\n");
                    exit(1);
                }
            }
        }
    }

    /// removing client queue

    if(msgctl(client_id, IPC_RMID, NULL) == -1){
        perror("Error during removing client's queue\n");
        exit(1);
    }
    free(message);
    printf("Client finished his job properly\n");
    return 0;
}