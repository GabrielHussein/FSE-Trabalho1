#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX 100
#define PORT 10151
#define SA struct sockaddr

int setupSocket ();
void bindSocket (int sockfd, struct sockaddr_in servaddr);
void listenSocketServer (int sockfd);
int acceptDataClient (int sockfd, struct sockaddr_in cli, int len);
void connectSocketServer (int sockfd, struct sockaddr_in servaddr);

#endif