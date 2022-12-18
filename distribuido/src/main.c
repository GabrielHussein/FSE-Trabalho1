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

pthread_t threadA;
pthread_t threadB;

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
void *thread_func (void *arg);
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
void checkFile (void);
void sendMessage (char *message);
void hallwayCheck (void);

int main (int argc, char * argv[]) {
    pthread_create(&threadA, NULL, thread_func, NULL);
    if(wiringPiSetup() == -1) {
        printf("Falha ao realizar set up da wiringpi.\n");
        return;
    }
    
    checkFile();
    setupPins();

    while(1){
        hallwayCheck();
        delay(500);
    }

    return 0;
}

void hallwayCheck (void){
    long millisCheck = millis();
    if ((millisCheck - millisHallway >= 1500) && hallwayLights == true){
        changeState(L_01);
        changeState(L_02);
        hallwayLights = false;
    }
    millisHallway = millisCheck;
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
        buff[0] = roomCounter[n].peopleSingle;
        buff[1] = roomCounter[n].peopleTotal;
        write(sockfd, reportBuffer, sizeof(reportBuffer));
        bzero(reportBuffer, sizeof(reportBuffer));
        delay(2000);        
    }
}

void *thread_func (void *arg) {
    struct sockaddr_in serverAddress, cli;
   
    sockfd = setupSocket();
    bzero(&serverAddress, sizeof(serverAddress));
   
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(CENTRAL_IP);
    serverAddress.sin_port = htons(PORT);
   
    connectSocketServer(sockfd, serverAddress);
   
    func(sockfd);   
    close(sockfd);
}

void updateCounter(int movementSignal){
    if(movementSignal == SC_IN){
        roomCounter.peopleSingle++;
        roomCounter.peopleTotal++;
    } else if(movementSignal == SC_OUT){
        roomCounter.peopleSingle--;
        roomCounter.peopleTotal--;
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
    if(millisIn - lastMovement > 200){
        updateCounter(SC_IN);
    }
    lastMovement = millisIn;
}

void outSensor(void) {
    long millisOut = millis();
    if(millisOut - lastMovement > 200){
        updateCounter(SC_OUT);
    }
    lastMovement = millisOut;
}

void alertSensorInit(int pin){
    switch(pin) {
      case SFum :
         message = "Sensor de fumaca acionado, iniciando alarme de incendio";
         alarmSystem = true;
         changeState(AL_BZ);
         sendMessage(message);
         break;
      case SJan :
         message = "Sensor de janela acionado";
         sendMessage(message);
         break;
      case SPor :
         message = "Sensor de porta acionado";
         sendMessage(message);
         break;
      case SPres :
         message = "Sensor de presenca acionado";
         if(alarmSystem == true && digitalRead(AL_BZ)==0){
            message = "Sensor de presenca acionado, alarme acionado";
            changeState(AL_BZ);
         } else if(alarmSystem = false){
            digitalWrite(L_01, 1);
            digitalWrite(L_02, 1);
            millisHallway = millis();
            hallwayLights = true;
            message = "Sensor de presenca acionado, lampadas da sala acionadas";
         }
         sendMessage(message);
         break;
      default :
         message = "Nao houveram sensores acionados";
         sendMessage(message);
   }
}

void sendReport(void){
    char *reportNumber;
    char *reportMessage = "O numero de pessoas na sala e de: ";
    asprintf(&reportNumber, "%d", roomCounter.peopleTotal);
    strcat(reportMessage, reportNumber);

    reportSize = strlen(reportMessage);

    if (send(clienteSocket, reportMessage, reportSize, 0) != reportSize)    {
        printf("Erro no envio.\n");
    }
}

void checkFile(void){
    FILE* roomFile;

    roomFile = fopen(argv[1], "r");
    
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
}
