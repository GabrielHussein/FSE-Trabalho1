#include "menu.h"
#include "socket.h"
#include "def.h"
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

room roomStatus;

void* funcMenu() {
    signal(SIGINT, handleSignal);
    signal(SIGTSTP, handleSignal);

    while(1) {
        mainMenu();
        sleep(20);
        __fpurge(stdin);
    }
}


void handleSignal(int sig) {
    if(sig==20){
        activateMenuOptions();
    } else {
        exit(0);
    }
}

void mainMenu() {
    system("clear");
    printf(" || ======= Estado atual dos componentes de saida da sala (1 = Ativo, 0 = Inativo) ====== ||\n\n");
    printf(" || 1 - Lampada 01:      %d                                                               ||\n", roomStatus.lamp01);
    printf(" || 2 - Lampada 02:      %d                                                               ||\n", roomStatus.lamp02);
    printf(" || 3 - Ar condicionado: %d                                                               ||\n", roomStatus.airConditioner);
    printf(" || 4 - Projetor:        %d                                                               ||\n", roomStatus.multimediaProjector);
    printf(" || 5 - Alarme:          %d                                                               ||\n\n", roomStatus.buzzer);
    printf(" || ===== Estado atual dos componentes de entrada da sala (1 = Ativo, 0 = Inativo) ====== ||\n\n");
    printf(" || Sensor de presenca:  %d                                                               ||\n", roomStatus.presenceS);
    printf(" || Sensor de fumaca:    %d                                                               ||\n", roomStatus.smokeS);
    printf(" || Sensor de janela:    %d                                                               ||\n", roomStatus.windowS);
    printf(" || Sensor de porta:     %d                                                               ||\n", roomStatus.doorS);
    printf(" || Sensor de entrada:   %d                                                               ||\n", roomStatus.peopleInS);
    printf(" || Sensor de saida:     %d                                                               ||\n\n", roomStatus.peopleOutS);
    printf(" ||       PARA HABILITAR OPCOES DE CONTROLE DOS COMPONENTES DIGITE CTRL+Z (SIGTSTP)       ||\n\n");
}

void activateMenuOptions() {
    printf(" ||   PARA ALTERAR O ESTADO DE UM COMPONENTE DE SAIDA DIGITE SEU CODIGO ||\n");
    printf(" ||   PARA DESLIGAR TODOS OS COMPONENTES DE SAIDA DIGITE 6              ||\n");
    printf(" ||   PARA LIGAR TODOS OS COMPONENTES DE SAIDA DIGITE 7                 ||\n");

    __fpurge(stdin);
    char userInputChar = getchar();
    int userInput = (int)userInputChar;
    system("clear");
    printf("userinput : %d\n",userInput);

    switch(userInput){
        case 1: 
            printf("Alterando o estado da lampada 01\n");
            roomStatus.lamp01 = !roomStatus.lamp01;
            sendMessageSignal(userInput);
        break;
            case 2: 
            printf("Alterando o estado da lampada 02\n");
            roomStatus.lamp02 = !roomStatus.lamp02;
            sendMessageSignal(userInput);
        break;
            case 3: 
            printf("Alterando o estado do ar condicionado\n");
            roomStatus.airConditioner = !roomStatus.airConditioner;
            sendMessageSignal(userInput);
        break;
            case 4: 
            printf("Alterando o estado do projetor\n");
            roomStatus.multimediaProjector = !roomStatus.multimediaProjector;
            sendMessageSignal(userInput);
            break;
        case 5: 
            printf("Alterando o estado do alarme\n");
            roomStatus.buzzer = !roomStatus.buzzer;
            sendMessageSignal(userInput);
            break;
        case 6: 
            printf("Desligando todos componentes\n");
            changeAll(0);
            sendMessageSignal(userInput);
            break;
        case 7: 
            printf("Ligando todos os componentes\n");
            changeAll(1);
            sendMessageSignal(userInput);
            break;
        default: 
            printf("Comando incorreto\n");
            break;
        
}
}

void changeAll(int onOff){
    if(onOff==1){
        roomStatus.lamp01 = 1;
        roomStatus.lamp02 = 1;
        roomStatus.airConditioner = 1;
        roomStatus.multimediaProjector = 1;
        roomStatus.buzzer = 1;
    } else if(onOff==0){
        roomStatus.lamp01 = 0;
        roomStatus.lamp02 = 0;
        roomStatus.airConditioner = 0;
        roomStatus.multimediaProjector = 0;
        roomStatus.buzzer = 0;
    }
}

void sendMessageSignal(int commandNumber) {
    printf("entrei na mensagem\n");
  
    struct sockaddr_in clientAddress;

    int sockfd = setupSocket();
    
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(DISTRIBUTED_IP);
    clientAddress.sin_port = htons(PORT);

    connectSocketServer(sockfd, clientAddress);

    char sendBuffer[10];
    snprintf(sendBuffer, 10, "%d", commandNumber);
    int size = strlen(sendBuffer);
    printf("enviando mensagem\n");
    if (send(sockfd, sendBuffer, size, 0) != size) {
        exit(0);
    }

    char responseBuffer[10];
    int size_rec = recv(sockfd, responseBuffer, 10, 0);
    if (size_rec < 0) {
        exit(0);
    }

    responseBuffer[10] = '\0';

    int responseMessage;
    sscanf(responseBuffer, "%d", &responseMessage);

    close(sockfd);
    writeLog(commandNumber, responseMessage);
}

void writeLog(int commandNumber, int responseMessage){
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    FILE *fp = fopen("reportLog.csv", "a");

    fprintf(fp, "%s,%s,%s", checkCommandNumber(commandNumber), asctime(timeinfo), (responseMessage == 1 ? "Sucesso" : "Falha"));

    fclose(fp);
}

char *checkCommandNumber(int commandNumber){
    switch (commandNumber){
        case 1:
            return "Aciounou Lampada 01";
        case 2:
            return "Acionou Lampada 02";
        case 3:
            return "Acionou Ar condicionado";
        case 4:
            return "Acionou Projetor";
        case 5:
            return "Acionou Buzzer";
        case 6:
            return "Desligou todos componentes";
        case 7:
            return "Ligou todos componentes";
        case 8:
            return "Acionou alarme";
        default:
            return "Comando nao registrado";
    }
}