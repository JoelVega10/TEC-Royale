#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <ncurses.h>
#include "ini.h"

void loadConfigurationFile(char * configFileName);
void startGameServer();
int createSocket();
void bindSocket();

typedef struct thread_args{
    int start_row;
    int start_column;
    int central_tower_column;
};

typedef struct params{
    int max_x;
    int max_y;
};
typedef struct sockets {
    int *new_sock;
    int *new_sock2;
} sockets;

typedef struct warrior_level{
    int health;
    int attack;
    int attack_speed;
    int level;
}warrior_level;

typedef struct send_warrior{
    char tower[10];
    struct warrior_level warrior;

};