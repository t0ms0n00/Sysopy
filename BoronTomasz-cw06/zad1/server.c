#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "message.h"

int running = 1;

typedef struct client_data{
    int client_id;
    int client_queue_num;
    int is_availible;
}client_data;

int server_id;
client_data clients[1000];

void init_response(client_data clients[]){
    char* queue_num = strtok(NULL, " \n");
    printf("Message received: INIT %s\n", queue_num);
    int client_q_id = atoi(queue_num);
    int i;
    for(i = 0; i < 1000; i++){
        if (clients[i].client_id == -1){
            clients[i].client_id = i;
            clients[i].client_queue_num = client_q_id;
            clients[i].is_availible = 1;
            char* message = calloc(100, sizeof(char));
            sprintf(message, "Client received id %d from server\n", i);
            my_msgbuf msg;
            msg.mtype = 1;
            strcpy(msg.mtext, message);
            msg.comrade_id = -1;
            printf("Sending response for command INIT\n");
            if(msgsnd(client_q_id, &msg, MSGSIZE, 0) == -1){
                perror("Problem during sending message to client\n");
                exit(1);
            }
            free(message);
            break;
        }
    }
}

void list_response(client_data clients[], int client_id){
    char* list = calloc(10000, sizeof(char));
    strcpy(list,"\nUSER_ID \t AVAILIBLE\n");
    printf("Message received: LIST\n");
    int i;
    for(i = 0; i < 1000; i++){
        if (clients[i].client_id != -1){
            char* user_info = calloc(30, sizeof(char));
            char* id = calloc(15, sizeof(char));
            sprintf(id, "%d ", clients[i].client_id);
            strcat(user_info, id);
            if(clients[i].is_availible != 0){
                strcat(user_info, "\t\t TRUE\n");
            }
            else{
                strcat(user_info, "\t\t FALSE\n");
            }
            strcat(list, user_info);
            free(id);
            free(user_info);
        }
    }
    strcat(list, "\n");
    my_msgbuf msg;
    msg.mtype = 1;
    strcpy(msg.mtext, list);
    msg.comrade_id = -1;
    printf("Sending response for command LIST\n");
    if(msgsnd(clients[client_id].client_queue_num, &msg, MSGSIZE, 0) == -1){
        perror("Problem during sending message to client\n");
        exit(1);
    }
    free(list);
}

void connect_response(client_data clients[], int client_1, int client_2){
    if (clients[client_2].is_availible == 0){
        char* message = calloc(30, sizeof(char));
        sprintf(message, "Cannot connect with client %d\n", client_2);
        my_msgbuf msg;
        msg.mtype = 1;
        strcpy(msg.mtext, message);
        msg.comrade_id = -1;
        printf("Sending response for command CONNECT _\n");
        if(msgsnd(clients[client_1].client_queue_num, &msg, MSGSIZE, 0) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        free(message);
    }
    else{
        my_msgbuf msg;
        msg.mtype = 1;
        msg.comrade_id = clients[client_2].client_queue_num;
        strcpy(msg.mtext,"=========== CHAT =========\n");
        if(msgsnd(clients[client_1].client_queue_num, &msg, MSGSIZE, 0) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        msg.mtype = 1;
        msg.comrade_id = clients[client_1].client_queue_num;
        strcpy(msg.mtext,"=========== CHAT =========\n");
        if(msgsnd(clients[client_2].client_queue_num, &msg, MSGSIZE, 0) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        clients[client_1].is_availible = 0;
        clients[client_2].is_availible = 0;
    }
}

void disconnect_response(client_data clients[], int client){
    my_msgbuf msg;
    msg.mtype = 1;
    strcpy(msg.mtext,"========== SERVER ==========\n");
    msg.comrade_id = -1;
    printf("Sending response to DISCONNECT command\n");
    if(msgsnd(clients[client].client_queue_num, &msg, MSGSIZE, 0) == -1){
        perror("Problem during sending message to client\n");
        exit(1);
    }
    clients[client].is_availible = 1;
}

void stop_response(client_data clients[], int client){
    clients[client].is_availible = 0;
    clients[client].client_id = -1;
    clients[client].client_queue_num = 0;
}

void SIGINT_handler(int sig_no){
    printf("Received SIGINT, closing server\n");
    int clients_count = 0;
    for(int i = 0; i < 1000; i++){
        if(clients[i].client_id >= 0){
            my_msgbuf msg;
            msg.mtype = 1;
            strcpy(msg.mtext, "Server will close immiediately, finish your job\n");
            msg.comrade_id = -1;
            if(msgsnd(clients[i].client_queue_num, &msg, MSGSIZE, 0) == -1){
                perror("Problem during sending message to client\n");
                exit(1);
            }
            clients_count++;
        }
    }
    while(clients_count > 0){
        my_msgbuf reaction;
        if(msgrcv(server_id, &reaction, MSGSIZE, 0, 0) == -1){
            perror("Problem with reading message from client\n");
            exit(1);
        }
        if(strcmp(reaction.mtext, "STOP") == 0){
            int client_num = reaction.sender_id;
            stop_response(clients, client_num);
            clients_count--;
        }
    }
    running = 0;
}

int main(int argc, char** argv){

    for(int i = 0; i < 1000; i++){
        clients[i].client_id = -1;
        clients[i].client_queue_num = -1;
        clients[i].is_availible = 0;
    }

    /// sigint handle

    struct sigaction act;
    act.sa_handler = SIGINT_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    /// creating server ipc queue

    printf("Server start his job\n");
    server_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if(server_id == -1){
        perror("Error during creating new msg queue\n");
        exit(1);
    }
    printf("Server queue id = %d\n", server_id);

    /// waiting for messages

    my_msgbuf msg_received;
    while(running){
        if (msgrcv(server_id, &msg_received, MSGSIZE, -TYPES_COUNTER, 0) != -1){
            char* command = strtok(msg_received.mtext, " \n");
            if(command == NULL){
                continue;
            }
            if(strcmp(command, "INIT") == 0){
                init_response(clients);
            }
            else if(strcmp(command, "LIST") == 0){
                int client_id = msg_received.sender_id;
                list_response(clients, client_id);
            }
            else if(strcmp(command, "CONNECT") == 0){
                char* pair = strtok(NULL, " \n");
                int pair_id = atoi(pair);
                int client_id = msg_received.sender_id;
                connect_response(clients, client_id, pair_id);
            }
            else if(strcmp(command, "DISCONNECT") == 0){
                int client_id = msg_received.sender_id;
                disconnect_response(clients, client_id);
            }
            else if(strcmp(command, "STOP") == 0){
                int client_id = msg_received.sender_id;
                stop_response(clients, client_id);
            }
        }
    };

    /// closing and removing server queue

    if(msgctl(server_id, IPC_RMID, NULL) == -1){
        perror("Error during removing server's queue\n");
        exit(1);
    }
    printf("Server finished his job properly\n");
    return 0;
}