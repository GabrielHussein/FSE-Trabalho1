#include "socket.h"

int setupSocket () {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created...\n");
        return sock;
    }
}

void bindSocket (int sockfd, struct sockaddr_in servaddr) {
    int b = bind(sockfd, (SA*)&servaddr, sizeof(servaddr));

    if(b != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded...\n");
    }
}

void listenSocketServer (int sockfd) {
    int l = listen(sockfd, 5);

    if(l != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening...\n");
    }
}

int acceptDataClient (int sockfd, struct sockaddr_in cli, int len) {
    int acc = accept(sockfd, (SA*)&cli, &len);

    if(acc < 0) {
        printf("Server accept failed...\n");
        exit(0);
    } else {
        printf("Server accept client...\n");
        return acc;
    }
}

void connectSocketServer (int sockfd, struct sockaddr_in servaddr) {
    int c = connect(sockfd, (SA*) &servaddr, sizeof(servaddr));
    if(c != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }
}