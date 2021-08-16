#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>

#include "common.h"

typedef struct{
    char name[20];
    int id;
    int fd;
    int opponent_fd;
    int pinged;
    int in_game;
}player;

int pairing = 0;
int running = 1;
int local_socket;
int network_socket;
int fd_read;
uint16_t port_num;
player players[MAX_PLAYERS];
pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

void poll_manager(){
    struct pollfd* fds = calloc(2+MAX_PLAYERS, sizeof(struct pollfd));
    fds[0].fd = network_socket;
    fds[0].events = POLLIN;
    fds[1].fd = local_socket;
    fds[1].events = POLLIN;
    
    int i = 0;
    for(i = 0; i < MAX_PLAYERS; i++){
        if(players[i].fd >= 0){        
            fds[i+2].fd = players[i].fd;
            fds[i+2].events = POLLIN;
        }
    }
    
    poll(fds, MAX_PLAYERS + 2, -1);

    for(i = 0; i < MAX_PLAYERS + 2; i++){
        if(fds[i].revents & POLLIN){
            fd_read = fds[i].fd;
            break;
        }
    }
    free(fds);
}

void make_pair(){
    int first_pos = -1;
    int second_pos = -1;
    pthread_mutex_lock(&players_mutex);
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(players[i].id >= 0){
            if(first_pos == -1){
                first_pos = i;
            }
            else{
                second_pos = i;
                break;
            }
        }
    }
    players[first_pos].opponent_fd = players[second_pos].fd;
    players[second_pos].opponent_fd = players[first_pos].fd;
    players[first_pos].in_game = 1;
    players[second_pos].in_game = 1;
    char response[MAX_TEXT_LENGTH];
    sprintf(response, "Paired with %s\n", players[second_pos].name);
    send(players[first_pos].fd, response, MAX_TEXT_LENGTH, 0);
    sprintf(response, "Paired with %s\n", players[first_pos].name);
    send(players[second_pos].fd, response, MAX_TEXT_LENGTH, 0);
    if(rand()%2 == 0){
        sprintf(response, "O");
        send(players[first_pos].fd, response, MAX_TEXT_LENGTH, 0);
        sprintf(response, "X");
        send(players[second_pos].fd, response, MAX_TEXT_LENGTH, 0);
        sprintf(response, "Your move\n");
        send(players[first_pos].fd, response, MAX_TEXT_LENGTH, 0);
        
    }
    else{
        sprintf(response, "X");
        send(players[first_pos].fd, response, MAX_TEXT_LENGTH, 0);
        sprintf(response, "O");
        send(players[second_pos].fd, response, MAX_TEXT_LENGTH, 0);
        sprintf(response, "Your move\n");
        send(players[second_pos].fd, response, MAX_TEXT_LENGTH, 0);
    }
    pthread_mutex_unlock(&players_mutex);
    pairing = 0;
}

void register_player(char* name, int fd){
    int name_taken = 0;
    int free_index = -1;
    pthread_mutex_lock(&players_mutex);
    for(int i=0; i<MAX_PLAYERS; i++){
        if(players[i].id < 0){
            free_index = i;
        }
        if(players[i].id >= 0 && strcmp(name, players[i].name) == 0){
            name_taken = 1;
        }
    }
    pthread_mutex_unlock(&players_mutex);
    char response[MAX_TEXT_LENGTH];
    if(name_taken){
        sprintf(response, "Login occupied\n");
        send(fd, response, MAX_TEXT_LENGTH, 0);
        return;
    }
    if(free_index == -1){
        sprintf(response, "Too many players\n");
        send(fd, response, MAX_TEXT_LENGTH, 0);
        return;
    }
    pthread_mutex_lock(&players_mutex);
    players[free_index].id = free_index;
    players[free_index].fd = fd;
    players[free_index].pinged = 1;
    players[free_index].in_game = 0;
    strcpy(players[free_index].name, name);

    pthread_mutex_unlock(&players_mutex);
    sprintf(response, "Waiting for other player to join\n");
    send(fd, response, MAX_TEXT_LENGTH, 0);
    if(pairing == 1){
        make_pair();
    }
    else{
        pairing = 1;
    }
}

void move(int fd, char* msg){
    pthread_mutex_lock(&players_mutex);
    for(int i=0; i<MAX_PLAYERS; i++){
        if (fd == players[i].fd){
            char response[MAX_TEXT_LENGTH];
            sprintf(response, "%s", msg);
            send(players[i].opponent_fd, response, MAX_TEXT_LENGTH, 0);
            sprintf(response, "Your move\n");
            send(players[i].opponent_fd, response, MAX_TEXT_LENGTH, 0);
            break;
        }
    }
    pthread_mutex_unlock(&players_mutex);
}

void disconnect(int fd){
    int opp_fd = -1;
    pthread_mutex_lock(&players_mutex);
    for(int i=0; i<MAX_PLAYERS; i++){
        if(players[i].fd == fd){
            char ban[MAX_TEXT_LENGTH];
            sprintf(ban, "Disconnected\n");
            send(players[i].fd, ban, MAX_TEXT_LENGTH, 0);
            opp_fd = players[i].opponent_fd;
            if(!players[i].in_game && pairing == 1){
                pairing = 0;    /// when waiting player disconected
            }
            if(opp_fd != -1){
                for(int j=0; j<MAX_PLAYERS; j++){
                    if(players[j].fd == opp_fd){
                        players[j].opponent_fd = -1;
                    }
                }
            }
            close(players[i].fd);
            players[i].fd = -1;
            players[i].id = -1;
            players[i].opponent_fd = -1;
            players[i].pinged = 1;
            players[i].in_game = 0;
            strcpy(players[i].name, "");
            
            break;
        }
    }
    pthread_mutex_unlock(&players_mutex);
    if(opp_fd >= 0){
        char response[MAX_TEXT_LENGTH];
        sprintf(response, "Rival disconnected\n");
        send(opp_fd, response, MAX_TEXT_LENGTH, 0);
    }
}

void ping_response(int fd){
    pthread_mutex_lock(&players_mutex);
    for(int i=0; i<MAX_PLAYERS; i++){
        if(players[i].fd == fd){
            players[i].pinged = 1;
            break;
        }
    }
    pthread_mutex_unlock(&players_mutex);
}

void pinging(){
    int act_index = 0;
    while(running){
        sleep(3);
        pthread_mutex_lock(&players_mutex);
        if(players[act_index].id >= 0){
            if(players[act_index].pinged == 0){
                /*pthread_mutex_unlock(&players_mutex);
                disconnect(players[act_index].fd);
                pthread_mutex_lock(&players_mutex);*/
                int opp_fd = -1;
                char ban[MAX_TEXT_LENGTH];
                sprintf(ban, "Disconnected\n");
                send(players[act_index].fd, ban, MAX_TEXT_LENGTH, 0);
                opp_fd = players[act_index].opponent_fd;
                if(!players[act_index].in_game && pairing == 1){
                    pairing = 0;    /// when waiting player disconected
                }
                close(players[act_index].fd);
                players[act_index].fd = -1;
                players[act_index].id = -1;
                players[act_index].opponent_fd = -1;
                players[act_index].pinged = 1;
                players[act_index].in_game = 0;
                strcpy(players[act_index].name, "");
                if(opp_fd >= 0){
                    char response[MAX_TEXT_LENGTH];
                    sprintf(response, "Rival disconnected\n");
                    send(opp_fd, response, MAX_TEXT_LENGTH, 0);
                }
            }
            else{
                players[act_index].pinged = 0;
                char ping_msg[MAX_TEXT_LENGTH];
                sprintf(ping_msg, "Write ping\n");
                send(players[act_index].fd, ping_msg, MAX_TEXT_LENGTH, 0);
            }
        }
        else{
            players[act_index].pinged = 1;
        }
        pthread_mutex_unlock(&players_mutex);
        act_index = (act_index+1)%MAX_PLAYERS;
    }
}

void program_exit(){
    for(int i=0; i<MAX_PLAYERS; i++){
        if(players[i].id >= 0){
            char msg[MAX_TEXT_LENGTH];
            sprintf(msg,"Closing server\n");
            send(players[i].fd, msg, MAX_TEXT_LENGTH, 0);
        }
    }
    running = 0;
}

void checker(int signo){
    printf("Received SIGPIPE\n");
    exit(1);
}

int main(int argc, char** argv){
    if(argc != 3){
        perror("./server [port num] [path]\n");
        exit(1);
    }

    signal(SIGPIPE, &checker);

    for(int i=0; i<MAX_PLAYERS; i++){
        player fake;
        fake.id = -1;
        fake.opponent_fd = -1;
        fake.fd = -1;
        fake.pinged = 1;
        fake.in_game = 0;
        players[i] = fake;
    }

    srand(time(NULL));
    char* port = calloc(MAX_TEXT_LENGTH, sizeof(char));
    char* path = calloc(MAX_TEXT_LENGTH, sizeof(char));
    strcpy(port, argv[1]);
    strcpy(path, argv[2]);
    port_num = (uint16_t) atoi(port);
    
    /// local socket

    unlink(path);

    if((local_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Problem with creating local socket\n");
        exit(2);
    }

    struct sockaddr_un local_addr;
    local_addr.sun_family = AF_UNIX;
    strcpy(local_addr.sun_path, path);

    if(bind(local_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
        perror("Problem with binding local socket\n");
        exit(3);
    }

    if(listen(local_socket, BACKLOG) == -1){
        perror("Problem with listen function local socket\n");
        exit(4);
    }

    /// network socket

    if((network_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Problem with creating network socket\n");
        exit(5);
    }

    struct sockaddr_in network_addr;
    network_addr.sin_family = AF_INET;
    network_addr.sin_addr.s_addr = INADDR_ANY;
    network_addr.sin_port = htobe16(port_num);

    if(bind(network_socket,(const struct sockaddr*) &network_addr, sizeof(network_addr)) == -1){
        perror("Problem with binding network socket\n");
        exit(6);
    }

    if(listen(network_socket, BACKLOG) == -1){
        perror("Problem with listen function network socket\n");
        exit(7);
    }

    /// ping init

    pthread_t tid;
    pthread_create(&tid, NULL, (void*) &pinging, NULL);

    /// loop

    printf("Server starts job\n");
    while (running){
        printf("-----------------------\n");
        poll_manager();
        if(fd_read == network_socket || fd_read == local_socket){
            fd_read = accept(fd_read, NULL, NULL);
        }
        
        char msg_received[MAX_TEXT_LENGTH];
        memset(msg_received, 0, MAX_TEXT_LENGTH);
        recv(fd_read, msg_received, MAX_TEXT_LENGTH, 0);
        printf("MSG RECEIVED %s\n", msg_received);

        if(strcmp(msg_received, "ping\n") == 0){
            ping_response(fd_read);
        }

        else if(atoi(msg_received) > 0 && atoi(msg_received) <= 9){
            move(fd_read, msg_received);
        }

        else if(strcmp(msg_received, "disconnect\n") == 0){
            disconnect(fd_read);
        }

        else if(strcmp(msg_received, "register\n") == 0){
            char name[MAX_TEXT_LENGTH];
            recv(fd_read, name, MAX_TEXT_LENGTH, 0);
            register_player(name, fd_read);
        }

        else if(strcmp(msg_received, "close\n") == 0){
            program_exit();
        }

        else{
            printf("NO ACTION\n");
        }
    }

    printf("Server ends job\n");

    unlink(path);
    free(port);
    free(path);
    if(close(local_socket) == -1){
        perror("Problem with closing local socket\n");
        exit(8);
    }
    if(close(network_socket) == -1){
        perror("Problem with closing network socket\n");
        exit(9);
    }
    return 0;
}