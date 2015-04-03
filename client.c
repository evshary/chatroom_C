/* Name: server.c
 * Author: evshary
 * Date: 2015-04-03
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>       //bzero
#include <arpa/inet.h>    //sockaddr_in, inet_pton...
#include <sys/socket.h>   //bind
#include <pthread.h>
#include "chat.h"

#define prompt "talking(q to quit):"

void *send_func(void *arg){
    int fd = *(int *)arg;
    char msg[MAX_MSG];

    while(1){
        printf(prompt);
        fgets(msg, sizeof(msg), stdin);
        msg[strlen(msg)-1] = '\0'; //replace '\n' with '\0'
        if (strncmp(msg, "\\q", 2) == 0) {
            printf("Quit the program now\n");
            goto done;
        }else{
            if ( send(fd, msg, strlen(msg)+1, 0) == -1) {
                perror("send");
                goto done;
            }
        }
    }
    done:
        close(fd);
        exit(1);
}

int main(int argc, char *argv[]){
    pthread_t send_thread;
    struct sockaddr_in sin;  //server address
    int s_fd;
    int bytes;
    char buf[MAX_MSG];
    client me;

    if (argc == 4){
        strncpy(me.name, argv[1], strlen(argv[1])+1); 
        strncpy(me.IP, argv[2], strlen(argv[2])+1);
        me.port = atoi(argv[3]);
        #ifdef DEBUG
        printf("name: %s\n", me.name);
        printf("IP: %s\n", me.IP);
        printf("port: %d\n", me.port);
        #endif
    }else{
        printf("Client Usage: ./client name IP port\n");
        return -1;
    }

    printf("Chatting Room Client start!!!\n");
    printf("Your name is %s\n", me.name);

    /* Set server info */
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;   //using IPv4
    inet_pton(AF_INET, me.IP, &sin.sin_addr); //server IP
    sin.sin_port = htons(me.port);  //server port
    
    /* Communication */
    if ( (s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        perror("socket");
        exit(1);
    }
    if ( connect(s_fd, (struct sockaddr *)&sin, sizeof(sin)) == -1 ) {
        perror("connect");
        exit(1);
    }
    
    bytes = send(s_fd, me.name, strlen(me.name)+1, 0); //include '\0'
    if (bytes == -1) {
        perror("send");
        exit(1);
    }

    //send message
    pthread_create(&send_thread, NULL, (void *)&send_func, (int *)&s_fd);

    //receive message
    while(1){
        int bytes = recv(s_fd, buf, MAX_MSG, 0);
        if (bytes == -1) {
            perror("recv");
            goto done;
        } else if ( bytes == 0 ) { //server-side end
            printf("Server-side disconnect\n");
            goto done;
        } else {
            printf("\33[2K\r");
            printf("%s\n", buf);
            printf(prompt);
            fflush(stdout);
        }
    }

    done:
        pthread_cancel(send_thread);
        close(s_fd);

    return 0;
}
