#ifndef MENU_H_
#define MENU_H_

typedef struct Room {
    int lamp01;
    int lamp02;
    int airConditioner;
    int multimediaProjector;
    int buzzer;
    int presenceS;
    int smokeS;
    int windowS;
    int doorS;
    int peopleInS;
    int peopleOutS;
} room;

void* funcMenu(void);
void handleSignal(int sig);
void mainMenu(void);
void activateMenuOptions(void);
void changeAll(int onOff);
void sendMessageSignal(int commandNumber);
void writeLog(int commandNumber, int responseMessage);
char *checkCommandNumber (int commandNumber);

#endif /* MENU_H_ */