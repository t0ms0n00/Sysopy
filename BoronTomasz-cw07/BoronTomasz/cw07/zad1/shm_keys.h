#ifndef SHM_KEYS_H
#define SHM_KEYS_H

#include <sys/ipc.h>
#include <stdlib.h>

#define table_key ftok(getenv("HOME"), 'T')
#define oven_key ftok(getenv("HOME"), 'O')
#define sem_key ftok(getenv("HOME"), 'S')

#define MAX_OVEN_CAPACITY 5
#define MAX_TABLE_CAPACITY 5

#define MAX_WORKERS 50

struct oven{
    int oven_index;
    int get_from;
    int values[MAX_OVEN_CAPACITY];
};

struct table{
    int table_index;
    int get_from;
    int values[MAX_TABLE_CAPACITY];
};

#endif