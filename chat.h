#ifndef _CHAT_H
#define _CHAT_H

#define MAX_NAME 25
#define MAX_IP 20
#define MAX_MSG 100

#define DEBUG

#include <stdbool.h>     //bool

typedef struct {
    int fd;
    bool used;
    char IP[MAX_IP];
    char name[MAX_NAME];
    int port;
} client; 

#endif
