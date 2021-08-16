#ifndef message_h
#define message_h

#define MAX_MSG_LEN 1000

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define TYPES_COUNTER 6

typedef struct my_msgbuf{
    long mtype;
    char mtext[MAX_MSG_LEN];
    int sender_id;
    int comrade_id;
}my_msgbuf;

const int MSGSIZE = sizeof(my_msgbuf) - sizeof(long);

#endif
