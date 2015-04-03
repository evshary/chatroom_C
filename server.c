/* Name: server.c
 * Author: evshary
 * Date: 2015-04-03
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>      //zero
#include <arpa/inet.h>   //sockaddr_in, inet_pton
#include <sys/socket.h>  //bind
#include <pthread.h>
#include "chat.h"

#define MAX_THREAD 20

pthread_t threads[MAX_THREAD];
client clients[MAX_THREAD];
int thread_num = 0;

void *handler(void *arg){
    client *client_i = arg;
    char msg[MAX_MSG];
    int bytes;
    int i;

    while(1){
        bytes = recv(client_i->fd, msg, MAX_MSG, 0);
        if (bytes == -1) {
            perror("receive");
            goto done;
        } else if (bytes == 0) {
            printf("%s offline\n", client_i->name);
            goto done;
        } else {
            for (i=0; i<MAX_THREAD; i++) {
                if ( clients[i].used == true && clients[i].fd != client_i->fd ) {
                    if( send(clients[i].fd, msg, strlen(msg)+1, 0) == -1 ){
                        perror("send");
                        goto done;
                    }
                }
            }
        }
    }
    done:
        close(client_i->fd);
        client_i->used = false;
        thread_num--;

    return NULL;
}

int main(int argc, char *argv[]){
    struct sockaddr_in sin;  //server
    struct sockaddr_in cin;  //client
    int l_fd;    //server socket
    int c_fd;    //client socket
    int port;    //server port

    socklen_t len;   //sock length
    char buf[MAX_MSG];  //buffer to save message
    int bytes;  //bytes from user
    int i;

    if (argc == 2){
        port = atoi(argv[1]);
        #ifdef DEBUG
        printf("Server port is %d\n", port);
        #endif
    }else{
        printf("Server Usage: ./server port\n");
        exit(1);
    }

    printf("Chat Room Server start!!!\n");
    
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;          //using IPv4
    sin.sin_addr.s_addr = INADDR_ANY;  //server can accept any address
    sin.sin_port = htons(port);

    /* Listen */
    if ( (l_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
        perror("socket");
        exit(1);
    }
    if ( bind(l_fd, (struct sockaddr*)&sin, sizeof(sin)) == 1 ) {
        perror("bind");
        exit(1);
    }
    if (listen(l_fd, 10) == -1) { // 10 means the num waitting for connection
        perror("listen");
        exit(1);
    }

    printf("Now we are listening the port %d...\n", port);

    while(1) {
        /* accept connection */
        if ( (c_fd = accept(l_fd, (struct sockaddr*)&cin, &len)) == -1) {
            perror("accept");
            exit(1);
        }

        if (thread_num < MAX_THREAD) {
            /* get a unused client */
            for (i=0; i<MAX_THREAD; i++){
                if (clients[i].used == false) break;
            }
            clients[i].used = true;
            clients[i].fd = c_fd;
            thread_num++;

            /* recv first data from client */
            bytes = recv(c_fd, buf, MAX_MSG, 0);
            if (bytes == -1) {
                clients[i].used = false;
                thread_num--;
                perror("receive");
                goto done;
            }

            /* Get client IP, port */
            inet_ntop(AF_INET, &cin.sin_addr, clients[i].IP, sizeof(clients[i].IP));
            clients[i].port = ntohs(cin.sin_port);
            strncpy(clients[i].name, buf, strlen(buf)+1);
            printf("Now %s connects with %s:%d\n", clients[i].name, clients[i].IP, clients[i].port);

            pthread_create(&threads[i], NULL, handler, (client *)&clients[i]);

        } else {
            strcpy(buf, "Sorry the server is full!\n");
            if( send(c_fd, buf, strlen(buf)+1, 0) == -1 ){
                perror("send");
                goto done;
            }
            close(c_fd);
            inet_ntop(AF_INET, &cin.sin_addr, buf, sizeof(buf));
            printf("%s can't connect to server!!!\n", buf);
            continue;
        }
    }
    done:
        close(c_fd);
        for (i=0; i<MAX_THREAD; i++){
            if (clients[i].used == true){
                pthread_cancel(threads[i]);
            }
        }
        exit(1);
    return 0;
}
