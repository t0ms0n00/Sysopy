#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "common.h"

/* 
    Z racji że wątki i sygnały razem źle działają,
    program można skończyć 'close'* mu podając.

    *Dodatkowa opcja do wpisania w terminalu gracza, do w miarę ładnej obsługi wyjścia z programu, normalnie powinien mieć admin lub strona serwera tę opcję, 
    natomiast łatwiej było mi przekazać jako komednę do serwera, którą on przechwyci i odpowiednio użyje.

    Słownik innych komend:
    1-9 - wybór pola na planszy
    ping - informacja że gracz odpowiada na zapytanie serwera
    CTRL+C == disconnect - wyłączenie się gracza (automatyczne po skończonej partii lub za pomocą tych komend asynchronicznie)
*/

int running = 1;
int server_socket;
uint16_t port_num;
char board[3][3];
char sign[2];
char opp_sign[2];
char winner[2];
char* name;
char* connection_type;
char* address;
char* port;

void connect_to_server(char* address, char* type){
    if(strcmp(type, "net") == 0){
        if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            perror("Problem with connect to server by network\n");
            exit(3);
        }
        
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port_num);

        if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0) 
        {
            perror("Invalid address, network socket\n");
            exit(4);
        }

        if(connect(server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
            perror("Client - connection failed\n");
            exit(5);
        }
    }
    else if(strcmp(type, "loc") == 0){
        if((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
            perror("Problem with connect in local way\n");
            exit(6);
        }

        struct sockaddr_un serv_addr;
        serv_addr.sun_family = AF_UNIX;
        strcpy(serv_addr.sun_path, address);

        if(connect(server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
            perror("Client local - connection failed\n");
            exit(7);
        }
    }
    else{
        perror("Bad connection type\n");
        exit(2);
    }
}

void register_client(char* name){
    char register_cmd[MAX_TEXT_LENGTH];
    strcpy(register_cmd, "register\n");
    if(send(server_socket, register_cmd, sizeof(register_cmd), 0) == -1){
        perror("Problem with registering client\n");
        exit(8);
    }
    if(send(server_socket, name, sizeof(name), 0) == -1){
        perror("Problem with registering client\n");
        exit(8);
    }
}

void draw(){
    for(int i = 0; i<3; i++){
        printf("%c|%c|%c\n", board[i][0], board[i][1], board[i][2]);
    }
}

void put_sign(char sign, int field){
    board[(field-1)/3][(field-1)%3] = sign;
}

bool check_win(){
    for(int i=0; i<3; i++){
        if(board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' '){
            printf("%c won\n", board[i][0]);
            winner[0] = board[i][0];
            running = 0;
            return true;
        }
        if(board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' '){
            printf("%c won\n", board[0][i]);
            winner[0] = board[0][i];
            running = 0;
            return true;
        }
    }
    if(board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' '){
        printf("%c won\n", board[0][0]);
        winner[0] = board[0][0];
        running = 0;
        return true;
    }
    if(board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' '){
        printf("%c won\n", board[0][2]);
        winner[0] = board[0][2];
        running = 0;
        return true;
    }
    return false;
}

bool all_filled(){
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            if(board[i][j] == ' ') return false;
        }
    }
    return true;
}

void check_draw(){
    if(!check_win() && all_filled()){
        running = 0;
        printf("draw\n");
    }
}

void disconnect(){
    char msg[MAX_TEXT_LENGTH];
    sprintf(msg, "disconnect\n");
    send(server_socket, msg, MAX_TEXT_LENGTH, 0);
    recv(server_socket, NULL, MAX_TEXT_LENGTH, 0);
    exit(0);
}

void game(){
    draw();
    int move = 0;
    
    while(running){
        
        char* response = calloc(MAX_TEXT_LENGTH, sizeof(char));
        int field = 0;
        fd_set set;
        struct timeval time;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        time.tv_sec = 0;
        time.tv_usec = 10;

        if(recv(server_socket, response, MAX_TEXT_LENGTH, MSG_DONTWAIT) != -1){
            printf("%s\n", response);
        }

        if(strcmp(response, "Disconnected\n") == 0){
            exit(0);
        }

        if(strcmp(response, "Rival disconnected\n") == 0 || strcmp(response,"Closing server\n") == 0){
            raise(SIGINT);
        }

        if(strcmp(response, "Your move\n")==0){
            move = 1;
        }

        else if(atoi(response) > 0 && atoi(response)<10){       /// digit
            int field_num = atoi(response);
            put_sign(opp_sign[0], field_num);
            draw();
            check_draw();
        }

        if(select(FD_SETSIZE, &set, NULL, NULL, &time) == 1){
            char cmd[10];
            fgets(cmd, 10, stdin);
            field = atoi(cmd);
            if(strcmp(cmd, "ping\n") == 0 || strcmp(cmd, "close\n") == 0){
                send(server_socket, cmd, MAX_TEXT_LENGTH, 0);
            }
            if(field <= 0 || field >9 || board[(field-1)/3][(field-1)%3] != ' '){
                continue;
            }
        }
        else{
            continue;
        }

        if(move){
            char message[MAX_TEXT_LENGTH];
            sprintf(message, "%d", field);
            send(server_socket, message, MAX_TEXT_LENGTH, 0);
            put_sign(sign[0], field);
            draw();
            move = 0;
        }

        check_draw();

        free(response);
    }
    if(sign[0] == winner[0]) disconnect();    /// server will disconnect both so one should disconnect here
    else if(winner[0] != 'X' && winner[0] != 'O' && move == 0){
        disconnect();
    }
    else{
        while(1){
            char* end = calloc(MAX_TEXT_LENGTH, sizeof(char));
            recv(server_socket, end, MAX_TEXT_LENGTH, 0);
            if(strcmp(end, "Rival disconnected\n") == 0){
                raise(SIGINT);
            }
            free(end);
        }
    }

    return;
}

void sig_handler(int signo){
    disconnect();
}

int main(int argc, char** argv){
    if(argc < 4){
        perror("./client [name] [connection type] [server address] ([port num])\n");
        exit(1);
    }
    name = calloc(MAX_TEXT_LENGTH, sizeof(char));
    connection_type = calloc(MAX_TEXT_LENGTH, sizeof(char));
    address = calloc(MAX_TEXT_LENGTH, sizeof(char));
    port = calloc(MAX_TEXT_LENGTH, sizeof(char));

    winner[0] = '-';

    signal(SIGINT, &sig_handler);

    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            board[i][j] = ' ';
        }
    }

    strcpy(name, argv[1]);
    strcpy(connection_type, argv[2]);
    strcpy(address, argv[3]);
    if(argc == 5){
        strcpy(port, argv[4]);
        port_num = (uint16_t) atoi(port);
    }
    
    connect_to_server(address, connection_type);

    register_client(name);

    char response[MAX_TEXT_LENGTH];
    
    if(recv(server_socket, response, MAX_TEXT_LENGTH, 0) == -1){
        perror("Client - problem with receive from server\n");
        exit(9);
    }
    printf("%s\n", response);

    if(strcmp(response, "Waiting for other player to join\n") != 0){
        exit(0);
    }

    int waiting = 1;
    while(waiting){
        char response[MAX_TEXT_LENGTH];
        fd_set set;
        struct timeval time;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        time.tv_sec = 0;
        time.tv_usec = 10;

        if(recv(server_socket, response, MAX_TEXT_LENGTH, MSG_DONTWAIT) != -1){
            printf("%s\n", response);
            if(strcmp(response, "Disconnected\n") == 0){
                exit(0);
            }
            if(strcmp(response,"Closing server\n") == 0){
                raise(SIGINT);
            }
            char* pair = strtok(response, " ");
            if(strcmp(pair, "Paired")==0) break;
        }

        if(select(FD_SETSIZE, &set, NULL, NULL, &time) == 1){
            char cmd[10];
            fgets(cmd, 10, stdin);
            send(server_socket, cmd, MAX_TEXT_LENGTH, 0);
        }
    }

    if(recv(server_socket, sign, MAX_TEXT_LENGTH, 0) == -1){
            perror("Client - problem with receive from server\n");
            exit(9);
    }
    printf("Your sign: %s\n", sign);

    if(sign[0] == 'O'){
        opp_sign[0] = 'X';
    }
    else{
        opp_sign[0] = 'O';
    }
    
    game();

    if(recv(server_socket, response, MAX_TEXT_LENGTH, 0) == -1){
            perror("Client - problem with receive from server\n");
            exit(9);
    }

    free(name);
    free(connection_type);
    free(address);
    free(port);
    return 0;
}