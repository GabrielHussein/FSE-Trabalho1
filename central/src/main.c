#include "socket.h"
#include "def.h"
#include "menu.h"
#include <errno.h>
#include <signal.h>
#include <pthread.h>

room *roomCounter;

int reportSize = 0;
int totalCount = 0;
char *message;
pthread_t listener;
pthread_t central;

bool alarmSystem = false;
bool hallwayLights = false;

void updateCounterLength (void);
void updateCounter (int peopleRoom, int peopleTotal);
void createCentralServer (void);

int main () {

    FILE *fp = fopen("reportLog.csv", "wb");
    
    fprintf(fp, "COMANDO,DATA,STATUS;\n");

    fclose(fp);

    pthread_create(&listener, NULL, funcMenu, NULL);
    pthread_create(&central, NULL, createCentralServer, NULL);

    pthread_join(listener, NULL);
    pthread_join(central, NULL);

    return 0;
}

void updateCounterLength() {
    roomCounter = (roomCounter *) malloc((reportSize + 1) * sizeof(roomCounter));
    reportSize++;
}

void updateCounter (int peopleRoom, int peopleTotal) {
    updateCounterLength();
    roomCounter[reportSize - 1].peopleSingle = peopleRoom;
    totalCount = totalCount + peopleTotal;
}


void createCentralServer(){
    bool opt = true;
    int sockfd, connfd[4], len, i;
    int max_sd, sd, new_socket, valread, addrlen, activity, max_conn = 4;
    struct sockaddr_in address, cli;
    fd_set readfds;
    char buff[MAX];

    for (i = 0; i < max_conn; i++) {  
        connfd[i] = 0;  
    } 

    sockfd = setupSocket();
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  

    bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    bindSocket(sockfd, address);
    listenSocketServer(sockfd);

    addrlen = sizeof(address);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_sd = sockfd; 

        for ( i = 0 ; i < max_conn ; i++) {  
            sd = connfd[i];  
            if(sd > 0){
                FD_SET(sd , &readfds);  
            }
            if(sd > max_sd){
                max_sd = sd;
            }  
        } 

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR)) {  
            printf("select error");  
        }

        if (FD_ISSET(sockfd, &readfds)) {
            if ((new_socket = accept(sockfd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }
            for (i = 0; i < max_conn; i++) {  
                if( connfd[i] == 0 ) {  
                    connfd[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);          
                    break;  
                }
            }  
        }

        for (i = 0; i < max_conn; i++) {  
            sd = connfd[i];  
            if (FD_ISSET(sd , &readfds)) {  
                    int peopleRoom = buff[0];
                    int peopleTotal = buff[1];

                    updateCounter(peopleRoom, peopleTotal);

                    printf("Total de salas monitoradas: %d\n", reportSize);
                    printf("Total de pessoas presentes nas salas: %d\n", totalCount);
                    for(int i = 0; i < reportSize; i++){
                        printf("Numero de pessoas na sala %d: %d\n", i, roomCounter[i].peopleSingle);
                    }
                    bzero(buff, MAX);
            }  
        } 
    }
}