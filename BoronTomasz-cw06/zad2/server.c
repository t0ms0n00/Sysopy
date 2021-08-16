#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "message.h"

int running = 1;

typedef struct client_data{
    int client_id;
    int client_queue_num;
    int is_availible;
    char queue_name[50];
}client_data;

int server_id = -1;
client_data clients[1000];

void generate_queue_name(char** name){
    srand(time(NULL));
    sprintf(*name, "/se_%d_", getpid());
    char random_letter = 'A' + (random() % 26);
    strcat(*name, &random_letter);
}

void init_response(client_data clients[]){
    char* queue_name = strtok(NULL, " \n");
    struct mq_attr attributes;
    attributes.mq_msgsize = MAX_MSG_LEN;
    attributes.mq_maxmsg = MAX_MSGS;
    printf("Message received: INIT %s\n", queue_name);
    int client_q_id = mq_open(queue_name, O_RDWR, 0666, &attributes);
    if(client_q_id == -1){
        perror("Problem with opening client queue\n");
        exit(1);
    }
    int i;
    for(i = 0; i < 1000; i++){
        if(clients[i].client_id == -1){
            clients[i].client_id = i;
            clients[i].client_queue_num = client_q_id;
            clients[i].is_availible = 1;
            strcpy(clients[i].queue_name, queue_name);
            char* message = calloc(MAX_MSG_LEN, sizeof(char));
            sprintf(message, "Client received id %d from server\n", i);
            if(mq_send(client_q_id, message, MAX_MSG_LEN, 1) == -1){
                perror("Problem during sending message to client\n");
                exit(1);
            }
            free(message);
            break;
        }
    }
}

void close_usr_queues(client_data clients[]){
    for(int i = 0; i < 1000; i++){
        if(clients[i].client_queue_num != -1){
            mq_close(clients[i].client_queue_num);
        }
    }
}

void stop_response(client_data clients[], int client){
    clients[client].is_availible = 0;
    clients[client].client_id = -1;
    strcpy(clients[client].queue_name, "-");
    printf("Closing client queue in server\n");
    char* message = calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(message, " ");
    if(mq_send(clients[client].client_queue_num, message, MAX_MSG_LEN, STOP) == -1){
        perror("Problem during sending STOP to server\n");
        exit(1);
    }
    mq_close(clients[client].client_queue_num);
    clients[client].client_queue_num = -1;
}

void SIGINT_handler(int sig_no){
    printf("Received SIGINT, closing server\n");
    int clients_count = 0;
    for(int i = 0; i < 1000; i++){
        if(clients[i].client_id >= 0){
            char* message = calloc(MAX_MSG_LEN, sizeof(char));
            sprintf(message, "Server will close immiediately, finish your job\n");
            if(mq_send(clients[i].client_queue_num, message, MAX_MSG_LEN, 1) == -1){
                perror("Problem during sending message to client\n");
                exit(1);
            }
            free(message);
            clients_count++;
        }
    }
    while(clients_count > 0){
        char response[MAX_MSG_LEN];
        unsigned int type;
        if(mq_receive(server_id, response, MAX_MSG_LEN, &type) == -1){
            perror("Problem with reading message from client\n");
            exit(1);
        }
        if(type == STOP){
            strtok(response, " \n");
            char* client_pos = strtok(NULL, " \n");
            int pos = atoi(client_pos);
            stop_response(clients, pos);
            clients_count--;
        }
    }
    running = 0;
}

void list_response(client_data clients[], int client){
    char* list = calloc(MAX_MSG_LEN, sizeof(char));
    strcpy(list,"\nUSER_ID \t AVAILIBLE\n");
    int i;
    for(i = 0; i < 1000; i++){
        if(clients[i].client_id != -1){
            char* user_info = calloc(30, sizeof(char));
            char* id = calloc(15, sizeof(char));
            sprintf(id, "%d %s", clients[i].client_id, clients[i].queue_name);
            strcat(user_info, id);
            if(clients[i].is_availible != 0){
                strcat(user_info, "\t TRUE\n");
            }
            else{
                strcat(user_info, "\t FALSE\n");
            }
            strcat(list, user_info);
            free(id);
            free(user_info);
        }
    }
    strcat(list, "\n");
    printf("Sending response for command LIST\n");
    if(mq_send(clients[client].client_queue_num, list, MAX_MSG_LEN, 1) == -1){
        perror("Problem during sending message to client\n");
        exit(1);
    }
    free(list);
}

void connect_response(client_data clients[], int client_1, int client_2){
    if(clients[client_2].is_availible == 0){
        char* message = calloc(30, sizeof(char));
        sprintf(message, "Cannot connect with client %d\n", client_2);
        printf("Sending response for command CONNECT _\n");
        if(mq_send(clients[client_1].client_queue_num, message, MAX_MSG_LEN, 1) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        free(message);
    }
    else{
        char comrade_name[50];
        char msg[50];
        sprintf(comrade_name,"%s", clients[client_2].queue_name);
        strcpy(msg, "=========== CHAT =========\n");
        if(mq_send(clients[client_1].client_queue_num, comrade_name, MAX_MSG_LEN, 1) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        if(mq_send(clients[client_1].client_queue_num, msg, MAX_MSG_LEN, 1) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        char customer_name[50];
        sprintf(customer_name,"%s", clients[client_1].queue_name);
        if(mq_send(clients[client_2].client_queue_num, customer_name, MAX_MSG_LEN, 1) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        if(mq_send(clients[client_2].client_queue_num, msg, MAX_MSG_LEN, 1) == -1){
            perror("Problem during sending message to client\n");
            exit(1);
        }
        clients[client_1].is_availible = 0;
        clients[client_2].is_availible = 0;
    }
}

void disconnect_response(client_data clients[], int client){
    printf("Sending response to DISCONNECT command\n");
    char* msg = calloc(30, sizeof(char));
    strcpy(msg, "========== SERVER ==========\n");
    if(mq_send(clients[client].client_queue_num, msg, MAX_MSG_LEN, 1) == -1){
        perror("Problem during sending message to client\n");
        exit(1);
    }
    clients[client].is_availible=1;
    free(msg);
}

int main(int argc, char** argv){

    /// sigint handle

    struct sigaction act;
    act.sa_handler = SIGINT_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    printf("Server starts his job\n");
    char* name = calloc(40, sizeof(char));
    generate_queue_name(&name);
    printf("Name:%s\n", name);
    for(int i = 0; i < 1000; i++){
        clients[i].client_id = -1;
        clients[i].client_queue_num = -1;
        clients[i].is_availible = 0;
        strcpy(clients[i].queue_name, "-");
    }

    /// creating server queue

    struct mq_attr attributes;
    attributes.mq_msgsize = MAX_MSG_LEN;
    attributes.mq_maxmsg = MAX_MSGS;
    server_id = mq_open(name, O_RDWR | O_CREAT | O_EXCL, 0666, &attributes);
    if(server_id == -1){
        perror("Problem with creating/opening server queue\n");
        exit(1);
    }

    /// waiting for messages

    char message_received[MAX_MSG_LEN];
    unsigned int priop;
    while(running){
        printf(".\n");
        if(mq_receive(server_id, message_received, MAX_MSG_LEN, &priop) != -1){
            printf("Get message\n");
            if(priop == INIT){
                strtok(message_received, " \n");
                init_response(clients);
            }
            else if(priop == STOP){
                strtok(message_received, " \n");
                char* client_pos = strtok(NULL, " \n");
                int pos = atoi(client_pos);
                stop_response(clients, pos);
            }
            else if(priop == LIST){
                strtok(message_received, " \n");
                char* client_pos = strtok(NULL, " \n");
                int pos = atoi(client_pos);
                list_response(clients, pos);
            }
            else if(priop == CONNECT){
                strtok(message_received, " \n");
                char* client_1 = strtok(NULL, " \n");
                int pos_1 = atoi(client_1);
                char* client_2 = strtok(NULL, " \n");
                int pos_2 = atoi(client_2);
                connect_response(clients, pos_1, pos_2);
            }
            else if(priop == DISCONNECT){
                strtok(message_received, " \n");
                char* client_pos = strtok(NULL, " \n");
                int pos = atoi(client_pos);
                disconnect_response(clients, pos);
            }
        }
        else{
            printf("Receiving error\n");
        }
    }
    /// delete server queue

    printf("Deleting server queue\n");
    close_usr_queues(clients);
    if(mq_unlink(name) == -1){
        perror("Problem with deleting server queue\n");
        exit(1);
    }
    free(name);
    printf("Server finished his job properly\n");
    return 0;
}