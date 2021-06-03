#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <ncurses.h>
#include "ini.h"
#include "algoritmo_genetico.h"

typedef struct Tower{
    char name[50];
    int power;
}Tower;

typedef struct params{
    int max_x;
    int max_y;
};

typedef struct option_menu_return{
    char *selected_option;
    int warrior;

};

typedef struct send_warrior{
    char tower[10];
    struct warrior_level warrior;

};

void loadConfigurationFile(char * configFileName);
void Inicializar(int canvasX, int canvasY);
void jugadorA();
void jugadorB();
struct option_menu_return *warriorMenuA(void *args);
struct option_menu_return *warriorMenuB(void *args);
char* attackMenu(int max_x, int max_y,void *params);
void startGameServer();
void *menuManagerB(void *args);
void *menuManagerA(void *args);
void *deployWarriorA1(void *params);
void *deployWarriorA2(void *params);
void *deployWarriorB1(void *params);
void *deployWarriorB2(void *params);
void *moveWarriorBTowerA2(void *params);
void *moveWarriorBTowerA1(void *params);
void *moveWarriorATowerB1(void *params);
void *moveWarriorATowerB2(void *params);
void powerTower(Tower *tower);
void *incomeHandler(void *args);
void changeSide(int towerNum,struct warrior_level warrior);
char* concat(int count, ...);
void winner(int winner);
