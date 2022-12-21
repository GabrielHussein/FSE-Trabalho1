#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <pthread.h>
#include "socket.h"
#include "def.h"

int L_01;
int L_02;
int AC;
int PR;
int AL_BZ;
int SPres;
int SFum;
int SJan;
int SPor;
int SC_IN;
int SC_OUT;
int DHT22;

room roomCounter;

pthread_t reportSender;
pthread_t listener;

long lastMovement = 0;
long millisHallway = 0;
int counterLength = 0;
int millisIn = 0;
int millisOut = 0;
int sockfd, connfd;
unsigned int reportSize;
unsigned int messageSize;
char *message;

bool alarmSystem = false;
bool hallwayLights = false;

void func(int sockfd);
void *threadSender (void *arg);
void *threadListener (void *arg);
void alertSensorInit(int pin);
void updateCounter(int movementSignal);
void doorSensor (void);
void windowSensor (void);
void smokeSensor (void);
void presenceSensor (void);
void inSensor (void);
void outSensor (void);
void changeState (int pin);
void sendReport (void);
void setupPins (void);
void checkFile (char *fileName);
void sendMessage (char *message);
void hallwayCheck (void);

int main (int argc, char * argv[]) {
    if(wiringPiSetup() == -1) {
        printf("Falha ao realizar set up da wiringpi.\n");
        return;
    }
    
    checkFile(argv[1]);
    setupPins();

    //pthread_create(&reportSender, NULL, threadSender, NULL);
    while(1){
        hallwayCheck();
        delay(3000);
    }
    //pthread_join(reportSender, NULL);
    return 0;
}

void hallwayCheck (void){
    long millisCheck = millis();
    if (hallwayLights == true){
	    printf("Tempo com luzes ligadas em milisegundos: %ld\n", millisCheck - millisHallway);
    }
    if ((millisCheck - millisHallway >= 15000) && hallwayLights == true){
        changeState(L_01);
        changeState(L_02);
        hallwayLights = false;
	printf("Desligando luzes apos o termino dos 15 segundos\n");
	millisHallway = millisCheck;
    }
}

void sendMessage(char *message){
    write(sockfd, message, sizeof(message));
    bzero(message, sizeof(message));
}

void func(int sockfd) {
    char reportBuffer[MAX];
    int n;  

    while(1) {
        bzero(reportBuffer, sizeof(reportBuffer));
        reportBuffer[0] = roomCounter.peopleSingle;
        write(sockfd, reportBuffer, sizeof(reportBuffer));
        bzero(reportBuffer, sizeof(reportBuffer));
        delay(2000);        
    }
}

void *threadSender (void *arg) {
    struct sockaddr_in serverAddress, cli;
   
    sockfd = setupSocket();
    bzero(&serverAddress, sizeof(serverAddress));
   
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
    serverAddress.sin_port = htons(PORT);
   
    connectSocketServer(sockfd, serverAddress);
    pthread_create(&listener, NULL, threadListener, (void*)sockfd);
    func(sockfd);   
    close(sockfd);
}

void *threadListener (void *arg){
    char receiveBuffer[10];
    int receiveSocket  = (int) arg;

    printf("Ouvindo do central\n");
    while(1){
        read(receiveSocket, receiveBuffer, 10);
	printf("Mensagem recebida: %d\n", receiveBuffer);
    }
}

void updateCounter(int movementSignal){
    if(movementSignal == SC_IN){
        roomCounter.peopleSingle++;
        roomCounter.peopleTotal++;
	printf("Entrou uma pessoa\n");
	sendReport();
    } else if(movementSignal == SC_OUT){
        roomCounter.peopleSingle--;
        roomCounter.peopleTotal--;
	printf("Saiu uma pessoa\n");
	sendReport();
    }
}

void changeState(int pin){
    if (digitalRead(pin) == 1){
        digitalWrite(pin, 0);
    }
    else{
        digitalWrite(pin, 1);
    }
}

void doorSensor(void) {
    alertSensorInit(SPor);
}

void windowSensor(void) {
    alertSensorInit(SJan);
}

void smokeSensor(void) {
    alertSensorInit(SFum);
}

void presenceSensor(void) {
    alertSensorInit(SPres);
}

void inSensor(void) {
    long millisIn = millis();
    if(millisIn - lastMovement > 100){
        updateCounter(SC_IN);
    }
    lastMovement = millisIn;
}

void outSensor(void) {
    long millisOut = millis();
    if(millisOut - lastMovement > 100){
        updateCounter(SC_OUT);
    }
    lastMovement = millisOut;
}

void alertSensorInit(int pin){
    if(pin==SFum){
        message = "Sensor de fumaca acionado, iniciando alarme de incendio";
        alarmSystem = true;
        printf("Sensor de fumaca acionado, iniciando alarme de incendio\n");
        changeState(AL_BZ);
    //    sendMessage(message);
    }else if(pin==SJan){
         message = "Sensor de janela acionado";
         printf("Sensor de janela acionado\n");
    //     sendMessage(message);
    }else if(pin==SPor){
         message = "Sensor de porta acionado";
         printf("Sensor de porta acionado\n");
    //     sendMessage(message);
    }else if(pin==SPres){
         message = "Sensor de presenca acionado";
         printf("Sensor de presenca acionado\n");
         if(alarmSystem == true && digitalRead(AL_BZ)==0){
            message = "Sensor de presenca acionado, alarme acionado";
            printf("Alarme acionado\n");
            changeState(AL_BZ);
         } else if(alarmSystem == false){
            digitalWrite(L_01, 1);
            digitalWrite(L_02, 1);
            millisHallway = millis();
            hallwayLights = true;
            message = "Sensor de presenca acionado, lampadas da sala acionadas";
            printf("Lampadas da sala acionadas\n");
         }
    //     sendMessage(message);
    }
}

void sendReport(void){
    char *reportNumber;
    char reportMessage[100];
    strcpy(reportMessage, "O numero de pessoas na sala e de: ");
    printf("O numero de pessoas na sala e de: %d\n", roomCounter.peopleTotal);
    asprintf(&reportNumber, "%d", roomCounter.peopleTotal);
    strcat(reportMessage, reportNumber);

    reportSize = strlen(reportMessage);

}

void checkFile(char *fileName){
    FILE* roomFile;

    roomFile = fopen(fileName, "r");
    
    if(!roomFile) {
        printf("Arquivo de configuração de sala não existente ou incorreto\n");
    } else {
        fscanf(roomFile, "%d", &L_01);
        fscanf(roomFile, "%d", &L_02);
        fscanf(roomFile, "%d", &AC);
        fscanf(roomFile, "%d", &PR);
        fscanf(roomFile, "%d", &AL_BZ);
        fscanf(roomFile, "%d", &SPres);
        fscanf(roomFile, "%d", &SFum);
        fscanf(roomFile, "%d", &SJan);
        fscanf(roomFile, "%d", &SPor);
        fscanf(roomFile, "%d", &SC_IN);
        fscanf(roomFile, "%d", &SC_OUT);
        fscanf(roomFile, "%d", &DHT22);
    }
    printf("Leitura do arquivo de configuração feita com sucesso\n");
    fclose(roomFile);
}

void setupPins(void){
    pinMode(L_01, OUTPUT);
    pinMode(L_02, OUTPUT);
    pinMode(AC, OUTPUT);
    pinMode(PR, OUTPUT);
    pinMode(AL_BZ, OUTPUT);
    pinMode(SPres, INPUT);
    pinMode(SFum, INPUT);
    pinMode(SPor, INPUT);
    pinMode(SJan, INPUT);
    pinMode(SC_IN, INPUT);
    pinMode(SC_OUT, INPUT);

    pullUpDnControl(L_01, PUD_UP);
    pullUpDnControl(L_02, PUD_UP);
    pullUpDnControl(AC, PUD_UP);
    pullUpDnControl(AL_BZ, PUD_UP);
    pullUpDnControl(SPres, PUD_UP);
    pullUpDnControl(SFum, PUD_UP);
    pullUpDnControl(SPor, PUD_UP);
    pullUpDnControl(SJan, PUD_UP);
    pullUpDnControl(SC_IN, PUD_UP);
    pullUpDnControl(SC_OUT, PUD_UP);

    wiringPiISR(SPor, INT_EDGE_RISING, &doorSensor);
    wiringPiISR(SJan, INT_EDGE_RISING, &windowSensor);
    wiringPiISR(SFum, INT_EDGE_RISING, &smokeSensor);
    wiringPiISR(SPres, INT_EDGE_RISING, &presenceSensor);
    wiringPiISR(SC_IN, INT_EDGE_RISING, &inSensor);
    wiringPiISR(SC_OUT, INT_EDGE_RISING, &outSensor);

    printf("Configuracao de pinos feita com sucesso\n");
}
