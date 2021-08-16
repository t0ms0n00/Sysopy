#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "message.h"

int server_id = -1;
int client_id = -1;
int client_s_id = -1;
int comrade_id = -1;
int running = 1;

void generate_queue_name(char** name){
    srand(time(NULL));
    sprintf(*name, "/cl_%d_", getpid());
    char random_letter = 'A' + (random() % 26);
    strcat(*name, &random_letter);
}

void stop_action(){
    printf("Sending STOP to server's queue\n");
    char* message = calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(message, "%d %d", STOP, client_s_id);
    if(mq_send(server_id, message, MAX_MSG_LEN, STOP) == -1){
        perror("Problem during sending STOP to server\n");
        exit(1);
    }
    free(message);
    mq_close(server_id);
    mq_close(client_id);
}

void SIGINT_handler(int sig_no){
    printf("Received SIGINT\n");
    stop_action();
    running = 0;
}

int choose_type(char command[]){
    if(strcmp(command, "LIST\n") == 0) return LIST;
    if(strcmp(command, "DISCONNECT\n") == 0) return DISCONNECT;
    if(strcmp(command,"STOP\n") == 0) return STOP;
    if(comrade_id == -1) return CONNECT;
    return TO_PERSON;
}

int main(int argc, char** argv){
    char* name = calloc(40, sizeof(char));
    generate_queue_name(&name);
    printf("Name:%s\n", name);

    /// handle SIGINT

    struct sigaction act;
    act.sa_handler = SIGINT_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    /// creating client queue

    if(argc != 2){
        perror("Format: ./client [server name]");
        exit(1);
    }
    printf("Client start his job\n");
    struct mq_attr attributes;
    attributes.mq_msgsize = MAX_MSG_LEN;
    attributes.mq_maxmsg = MAX_MSGS;
    char server_name[30];
    strcpy(server_name, argv[1]);
    server_id = mq_open(server_name, O_RDWR, 0666, &attributes);
    if(server_id == -1){
        perror("Problem with creating/opening server queue\n");
        exit(1);
    }
    client_id = mq_open(name, O_RDWR | O_CREAT | O_EXCL, 0666, &attributes);
    if(client_id == -1){
        perror("Problem with creating/opening server queue\n");
        exit(1);
    }

    /// init

    printf("Sending INIT to server's queue\n");
    char* message = calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(message, "%d %s", INIT, name);
    if(mq_send(server_id, message, MAX_MSG_LEN, INIT) == -1){
        perror("Problem during sending INIT to server\n");
        exit(1);
    }
    printf("Waiting for response\n");
    char* response = calloc(MAX_MSG_LEN, sizeof(char));
    if(mq_receive(client_id, response, MAX_MSG_LEN, NULL) == -1){
        perror("Problem with catching INIT response\n");
        exit(1);
    }
    else{
        printf("INIT response: %s\n", response);
    }
    strtok(response, " ");
    strtok(NULL, " ");
    strtok(NULL, " ");
    char* client_num = strtok(NULL, " ");
    client_s_id = atoi(client_num);
    free(message);
    free(response);

    /// scanning commands

    struct mq_attr new_attributes;
    new_attributes.mq_msgsize = MAX_MSG_LEN;
    new_attributes.mq_maxmsg = MAX_MSGS;
    new_attributes.mq_flags = O_NONBLOCK;
    mq_setattr(client_id, &new_attributes, NULL);
    char server_msg[MAX_MSG_LEN];
    while(running){
        char command[50];
        fd_set set;
        struct timeval time;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        time.tv_sec = 1;
        time.tv_usec = 0;
        if(mq_receive(client_id, server_msg, MAX_MSG_LEN, NULL) != -1){
            if(server_msg[0] == '/'){
                comrade_id = mq_open(server_msg, O_RDWR, 0666, &new_attributes);
            }
            else{
                printf("%s\n", server_msg);
            }
        }
        if(select(FD_SETSIZE, &set, NULL, NULL, &time) == 1){
            fgets(command, 50 ,stdin);
            int type = choose_type(command);
            if(type == STOP){
                stop_action();
                break;
            }
            else if(type == CONNECT){
                char* msg = calloc(MAX_MSG_LEN, sizeof(char));
                strtok(command," \n");
                char* number = strtok(NULL, " \n");
                int commrade_num = atoi(number);
                sprintf(msg, "%d %d %d", type, client_s_id, commrade_num);
                if(mq_send(server_id, msg, MAX_MSG_LEN, type) == -1){
                    perror("Problem during sending CONNECT to server\n");
                    exit(1);
                }
                free(msg);
            }
            else if(type == DISCONNECT){
                char* msg = calloc(MAX_MSG_LEN, sizeof(char));
                sprintf(msg, "%d %d", type, client_s_id);
                if(mq_send(server_id, msg, MAX_MSG_LEN, type) == -1){
                    perror("Problem during sending DISCONNECT to server\n");
                    exit(1);
                }
                mq_close(comrade_id);
                comrade_id = -1;
                free(msg);
            }
            else if(type == LIST){
                char* msg = calloc(MAX_MSG_LEN, sizeof(char));
                sprintf(msg, "%d %d", type, client_s_id);
                if(mq_send(server_id, msg, MAX_MSG_LEN, type) == -1){
                    perror("Problem during sending LIST to server\n");
                    exit(1);
                }
                free(msg);
            }
            else if(comrade_id >= 0){
                if(mq_send(comrade_id, command, MAX_MSG_LEN, TO_PERSON) == -1){
                    perror("Problem during sending message to person\n");
                    exit(1);
                }
            }
        }
    }

    /// removing client queue

    printf("Removing client queue\n");
    if(mq_unlink(name) == -1){
        perror("Problem with deleting client queue\n");
        exit(1);
    }
    free(name);
    printf("Client finished his job properly\n");
    return 0;
}