#ifndef SHM_KEYS_H
#define SHM_KEYS_H

#define TABLE "/table"
#define OVEN "/oven"

#define S_TABLE_FREE_SPACE "/table_free_space"
#define S_OVEN_FREE_SPACE "/oven_free_space"
#define S_TABLE_LOCK "/table_lock"
#define S_OVEN_LOCK "/oven_lock"

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