#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
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

long lastMovement = 0;
int counterLength = 0;
int millisIn = 0;
int millisOut = 0;

void alertSensorInit(int sensor);
void updateCounter(int movementSignal);
void setupDevices();
void doorSensor (void);
void windowSensor (void);
void smokeSensor (void);
void presenceSensor (void);

int main (int argc, char * argv[]) {
    if(wiringPiSetup() == -1) return 1;

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
    wiringPiISR(SC_IN, INT_EDGE_RISING, &callback_speed_out);
    wiringPiISR(SC_OUT, INT_EDGE_RISING, &callback_speed_in);

    return 0;
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

void setupDevices() {
    digitalWrite(L_01, 1);
    digitalWrite(L_02, 1);
    digitalWrite(AC, 1);
    digitalWrite(PR, 1);
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

void alertSensorInit(int sensor){
    switch(sensor) {
      case SFum :
         printf("Sensor de fumaca acionado\n" );
         break;
      case SJan :
         printf("Sensor de janela acionado\n" );
         break;
      case SPor :
         printf("Sensor de porta acionado\n" );
         break;
      case SPres :
         printf("Sensor de presenca acionado\n" );
         break;
      default :
         printf("Nao houveram sensores acionados\n" );
   }
}