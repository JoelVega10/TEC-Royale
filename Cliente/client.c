/*
	C ECHO client example using sockets
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ini.h"
#include "client.h"
#include "mypthreads/MyThread.h"
#include "queue.h"
#include <math.h>
#include <stdlib.h>

#define DELAY 30000
thread_t *threads;
int loserA1 = 0,loserA2 = 0,loserB1 = 0,loserB2 = 0, winnerA = 0,winnerB = 0,torreB1 = 1,torreB2 = 1,
torreA2 = 1,torreA1 = 1,lock,sock, i = 0;;
char *configFileName = NULL;
char *warriorsA[4],*warriorsB[4];
ini_t *config = 0;
struct warrior_level *populationA;
struct warrior_level *populationB;
struct sockaddr_in server;
char server_reply[3];
struct option_menu_return *returnValue;
Queue A1Q,A2Q,B1Q,B2Q;
struct warrior_level *towerA1,*towerA2,*towerB1,*towerB2,*towerAC,*towerBC;


//Funcion para que un guerrero ataque a una torre, esta imprime la vida actual de estas.
char* fight_tower(struct warrior_level*warriorA,struct warrior_level *warriorB, char* tower){
    double dps_warriorA = warriorA->attack*((warriorA->attack_speed/500.0)*1);
    double dps_warriorB = warriorB->attack*((warriorB->attack_speed/500.0)*1);
    move(11,15);
    printw("  ");
    if(strcmp(tower,"TA1") == 0) {
        move(5, 12);
        printw("%d", warriorA->health);
    }
    else if(strcmp(tower,"TA2") == 0) {
        move(18, 12);
        printw("%d", warriorA->health);
    }
    else if(strcmp(tower,"TB1") == 0) {
        move(5, 41);
        printw("%d", warriorA->health);
    }
    else if(strcmp(tower,"TB2") == 0) {
        move(18, 41);
        printw("%d", warriorA->health);
    }
    else if(strcmp(tower,"TBC") == 0) {
        move(11, 48);
        printw("%d", warriorA->health);
    }
    else if(strcmp(tower,"TAC") == 0) {
        move(11, 3);
        printw("%d", warriorA->health);
    }
    if(warriorB->health/dps_warriorA > warriorA->health/dps_warriorB){
        warriorB->health = warriorB->health - ((int) dps_warriorA * warriorA->health/dps_warriorB);
        return "B";
    }
    else if (warriorB->health/dps_warriorA == warriorA->health/dps_warriorB){
        return "tie";
    }
    else{
        warriorA->health = warriorA->health - ((int) dps_warriorB * warriorB->health/dps_warriorA);
        return "A";
    }
}



int main(int argc , char *argv[])
{
     srand(time(0));
     threads = (thread_t*)malloc(sizeof(thread_t) * 2);
     int index;
     int c;

     opterr = 0;

     while ((c = getopt(argc, argv, "c:")) != -1)
         switch (c)
         {
             case 'c':
                 configFileName = optarg;
                 break;
             case '?':
                 if (optopt == 'c')
                     fprintf(stderr, "La opción -%c requiere un argumento.\n", optopt);
                 else if (isprint(optopt))
                     fprintf(stderr, "Opción inválida `-%c'.\n", optopt);
                 else
                     fprintf(stderr,
                             "Caracter desconocido `\\x%x'.\n",
                             optopt);
                 return 1;
             default:
                 abort();
         }

     loadConfigurationFile(configFileName);


     //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

     server.sin_addr.s_addr = inet_addr("127.0.0.1");
     server.sin_family = AF_INET;
     server.sin_port = htons( 8888 );

     //Connect to remote server
     if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
     {
         perror("connect failed. Error");
         return 1;
     }

     puts("Connected\n");

     //keep communicating with server
     while(1)
     {
         //Receive a reply from the server
         if( recv(sock , server_reply , 2 , 0) < 0)
         {
             puts("recv failed");
             break;
         }
         printf("El valor recibido: %s\n", server_reply);

         if( server_reply[0]=='1'){
             jugadorA();
         }
         else if( server_reply[0]=='2'){
             jugadorB();
         }else{
             printf("nada\n");
             break;
         }
     }
     close(sock);
     return 0;
}


void loadConfigurationFile(char * configFileName){
    config = ini_load(configFileName);
    if (!config)
    {
        printf("Error al abrir el archivo de configuración. Verifique el nombre de nuevo. \n");
        exit(0);
    }
}


void Inicializar(canvasX, canvasY)
{
    initscr();
    curs_set(0);
    wresize(stdscr, canvasX, canvasY);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    noecho();
}

//Inicializa el display y funcionamiento del jugador A.
void jugadorA()
{
    const char *torreA1 = ini_get(config, "game", "torreA1");
    const char *torreA2 = ini_get(config, "game", "torreA2");
    const char *torreAC = ini_get(config, "game", "torreAC");
    const char *torreA1x = ini_get(config, "game", "torreA1x");
    const char *torreA1y = ini_get(config, "game", "torreA1y");
    const char *torreACx = ini_get(config, "game", "torreACx");
    const char *torreACy = ini_get(config, "game", "torreACy");
    const char *torreA2x = ini_get(config, "game", "torreA2x");
    const char *torreA2y = ini_get(config, "game", "torreA2y");
    const char *puente1A1 = ini_get(config, "game", "puente1A1");
    const char *puente1Ax1 = ini_get(config, "game", "puente1Ax1");
    const char *puente1Ay1 = ini_get(config, "game", "puente1Ay1");
    const char *puente1A2 = ini_get(config, "game", "puente1A2");
    const char *puente1Ax2 = ini_get(config, "game", "puente1Ax2");
    const char *puente1Ay2 = ini_get(config, "game", "puente1Ay2");
    const char *puente2A1 = ini_get(config, "game", "puente2A1");
    const char *puente2Ax1 = ini_get(config, "game", "puente2Ax1");
    const char *puente2Ay1 = ini_get(config, "game", "puente2Ay1");
    const char *puente2A2 = ini_get(config, "game", "puente2A2");
    const char *puente2Ax2 = ini_get(config, "game", "puente2Ax2");
    const char *puente2Ay2 = ini_get(config, "game", "puente2Ay2");
    const int canvasX = atoi(ini_get(config, "game", "canvasX"));
    const int canvasY = atoi(ini_get(config, "game", "canvasY"));
    populationA = generatePopulation(4);
    queueInit(&A1Q, sizeof(struct warrior_level));
    queueInit(&A2Q, sizeof(struct warrior_level));
    towerA1 = (struct warrior_level*)malloc(sizeof(struct warrior_level));
    towerA2 = (struct warrior_level*)malloc(sizeof(struct warrior_level));
    towerAC = (struct warrior_level*)malloc(sizeof(struct warrior_level));
    towerA1->attack = 500;
    towerA1->attack_speed  = 500;
    towerA1->health = 250;
    towerA2->attack = 500;
    towerA2->attack_speed  = 500;
    towerA2->health = 250;
    towerAC->attack = 500;
    towerAC->attack_speed  = 500;
    towerAC->health = 500;
    int max_y = 0, max_x = 0;
    Inicializar(canvasX, canvasY);
    bkgd(COLOR_PAIR(1));

    getmaxyx(stdscr, max_y, max_x);
    //Jugador a
    move(atoi(torreA1x), atoi(torreA1y));
    printw(torreA1);
    move(atoi(puente1Ax1), atoi(puente1Ay1));
    printw(puente1A1); //====----------------");
    move(atoi(puente1Ax2), atoi(puente1Ay2));
    printw(puente1A2); //====----------------");
    move(atoi(torreACx), atoi(torreACy));
    printw(torreAC);
    move(atoi(torreA2x), atoi(torreA2y));
    printw(torreA2);
    move(atoi(puente2Ax1), atoi(puente2Ay1));
    printw(puente2A1); //====----------------");
    move(atoi(puente2Ax2), atoi(puente2Ay2));
    printw(puente2A2); //====----------------");
    refresh();

    struct params *param = malloc(sizeof *param);
    param->max_x = max_x;
    param->max_y = max_y;

    thread_t t1,t2;
    my_thread_init(200);
    my_mutex_init(&lock);

    my_thread_create(&t1,incomeHandler, param,50,"Sort");
    my_thread_create(&t2,menuManagerA, param,50,"Sort");
    my_thread_join(t1, NULL);
    my_thread_join(t2, NULL);

    getch();
    endwin();

}

//Inicializa el display y funcionamiento del jugador B.
void jugadorB()
{
    const char *torreB1 = ini_get(config, "game", "torreB1");
    const char *torreB2 = ini_get(config, "game", "torreB2");
    const char *torreBC = ini_get(config, "game", "torreBC");
    const char *torreB1x = ini_get(config, "game", "torreB1x");
    const char *torreB1y = ini_get(config, "game", "torreB1y");
    const char *torreBCx = ini_get(config, "game", "torreBCx");
    const char *torreBCy = ini_get(config, "game", "torreBCy");
    const char *torreB2x = ini_get(config, "game", "torreB2x");
    const char *torreB2y = ini_get(config, "game", "torreB2y");
    const char *puente1B1 = ini_get(config, "game", "puente1B1");
    const char *puente1Bx1 = ini_get(config, "game", "puente1Bx1");
    const char *puente1By1 = ini_get(config, "game", "puente1By1");
    const char *puente1B2 = ini_get(config, "game", "puente1B2");
    const char *puente1Bx2 = ini_get(config, "game", "puente1Bx2");
    const char *puente1By2 = ini_get(config, "game", "puente1By2");
    const char *puente2B1 = ini_get(config, "game", "puente2B1");
    const char *puente2Bx1 = ini_get(config, "game", "puente2Bx1");
    const char *puente2By1 = ini_get(config, "game", "puente2By1");
    const char *puente2B2 = ini_get(config, "game", "puente2B2");
    const char *puente2Bx2 = ini_get(config, "game", "puente2Bx2");
    const char *puente2By2 = ini_get(config, "game", "puente2By2");
    const int canvasX = atoi(ini_get(config, "game", "canvasX"));
    const int canvasY = atoi(ini_get(config, "game", "canvasY"));
    populationB = generatePopulation(4);
    queueInit(&B1Q, sizeof(struct warrior_level));
    queueInit(&B2Q, sizeof(struct warrior_level));
    towerB1 = (struct warrior_level*)malloc(sizeof(struct warrior_level));
    towerB2 = (struct warrior_level*)malloc(sizeof(struct warrior_level));
    towerBC = (struct warrior_level*)malloc(sizeof(struct warrior_level));
    towerB1->attack = 500;
    towerB1->attack_speed  = 500;
    towerB1->health = 250;
    towerB2->attack = 500;
    towerB2->attack_speed  = 500;
    towerB2->health = 250;
    towerBC->attack = 500;
    towerBC->attack_speed  = 500;
    towerBC->health = 500;
    int max_y = 0, max_x = 0;
    Inicializar(canvasX, canvasY);
    bkgd(COLOR_PAIR(1));

    getmaxyx(stdscr, max_y, max_x);
    move(atoi(torreB1x), atoi(torreB1y));
    printw(torreB1);
    move(atoi(puente1Bx1), atoi(puente1By1));
    printw(puente1B1); //====----------------");
    move(atoi(puente1Bx2), atoi(puente1By2));
    printw(puente1B2);
    move(atoi(torreBCx), atoi(torreBCy));
    printw(torreBC);
    move(atoi(torreB2x), atoi(torreB2y));
    printw(torreB2);
    move(atoi(puente2Bx1), atoi(puente2By1));
    printw(puente2B1); //====----------------");
    move(atoi(puente2Bx2), atoi(puente2By2));
    printw(puente2B2);
    refresh();

    struct params *param = malloc(sizeof *param);
    param->max_x = max_x;
    param->max_y = max_y;

    thread_t t1,t2;
    my_thread_init(200);
    my_mutex_init(&lock);

    my_thread_create(&t1,incomeHandler, param,50,"Sort");
    my_thread_create(&t2,menuManagerB, param,50,"Sort");
    my_thread_join(t1, NULL);
    my_thread_join(t2, NULL);

    getch();
    endwin();

}


//Despliega un guerrero en el mismo cliente solicitado.
void *menuManagerA(void *args){
    while (1){
        returnValue = warriorMenuA(args);
        if(strcmp(returnValue->selected_option,"Lanzar hacia torre 2") == 0){
            enqueue(&A2Q,&populationA[returnValue->warrior]);
            my_thread_create(&threads[i],deployWarriorA2,&populationA[returnValue->warrior],50,"Sort");
            my_thread_join(threads[i],NULL);
            i++;
            if(!loserA2) {
                changeSide(4, populationA[returnValue->warrior]);
            }
            loserA2 = 0;
            int warrior;
            warrior = returnValue->warrior;
            int random = getRandomNumber(0,3);
            while(random==warrior){
                random = getRandomNumber(0,3);
            }
            crossover(populationA,warrior,random);
            mutate(populationA,warrior);



        }else if(strcmp(returnValue->selected_option,"Lanzar hacia torre 1") == 0) {
            enqueue(&A1Q,&populationA[returnValue->warrior]);
            my_thread_create(&threads[i],deployWarriorA1,&populationA[returnValue->warrior],50,"Sort");
            my_thread_join(threads[i],NULL);
            i++;
            if(!loserA1) {
                changeSide(3, populationA[returnValue->warrior]);
            }
            loserA1 = 0;
            int warrior;
            warrior = returnValue->warrior;
            int random = getRandomNumber(0,3);
            while(random==warrior){
                random = getRandomNumber(0,3);
            }
            crossover(populationA,warrior,random);
            mutate(populationA,warrior);

        }

    }
    getch();
    endwin();
}

//Despliega un guerrero en el mismo cliente solicitado.
void *menuManagerB(void *args){
    while (1){
        returnValue = warriorMenuB(args);
        if(strcmp(returnValue->selected_option,"Lanzar hacia torre 2") == 0){
            enqueue(&B2Q,&populationB[returnValue->warrior]);
            my_thread_create(&threads[i],deployWarriorB2,&populationB[returnValue->warrior],50,"Sort");
            my_thread_join(threads[i],NULL);
            i++;
            if(!loserB2) {
                changeSide(2, populationB[returnValue->warrior]);
            }
            loserB2 = 0;
            int warrior = returnValue->warrior;
            int random = getRandomNumber(0,3);
            while(random==warrior){
                random = getRandomNumber(0,3);
            }
            crossover(populationB,warrior,random);
            mutate(populationB,warrior);


        }else if(strcmp(returnValue->selected_option,"Lanzar hacia torre 1") == 0) {

            enqueue(&B1Q,&populationB[returnValue->warrior]);
            my_thread_create(&threads[i],deployWarriorB1,&populationB[returnValue->warrior],50,"Sort");
            my_thread_join(threads[i],NULL);
            i++;
            if(!loserB1) {
                changeSide(1, populationB[returnValue->warrior]);
            }
            loserB1 = 0;
            int warrior = returnValue->warrior;
            int random = getRandomNumber(0,3);
            while(random==warrior){
                random = getRandomNumber(0,3);
            }
            crossover(populationB,warrior,random);
            mutate(populationB,warrior);

        }

    }
    getch();
    endwin();
}
//Recibe la respuesta del servidor y despliega el guerrero en la posicion indicada.
void *incomeHandler(void *args){
    while (1){
        struct send_warrior server_reply;
        if( recv(sock , (struct send_warrior *)&server_reply, sizeof(server_reply), 0) > 0)
        {
            if(strcmp(server_reply.tower, "W1A")==0){
                enqueue(&A1Q, &(server_reply.warrior));
                my_thread_create(&threads[i] ,moveWarriorBTowerA1,&(server_reply.warrior), 50, "Sort");
                my_thread_join(threads[i],NULL);
                i++;

            }else if(strcmp(server_reply.tower, "W2A")==0){
                enqueue(&A2Q, &(server_reply.warrior));
                my_thread_create(&threads[i],moveWarriorBTowerA2, &(server_reply.warrior), 50, "Sort");
                my_thread_join(threads[i],NULL);
                i++;

            }else if(strcmp(server_reply.tower, "W1B")==0){
                enqueue(&B1Q, &(server_reply.warrior));
                my_thread_create(&threads[i] ,moveWarriorATowerB1,&(server_reply.warrior), 50, "Sort");
                my_thread_join(threads[i],NULL);
                i++;

            }else if(strcmp(server_reply.tower, "W2B")==0){
                enqueue(&B2Q, &(server_reply.warrior));
                my_thread_create(&threads[i] ,moveWarriorATowerB2,&(server_reply.warrior), 50, "Sort");
                my_thread_join(threads[i],NULL);
                i++;

            }
            else if(strcmp(server_reply.tower, "WIA")==0){
                move(13,15);
                printw("Gano el jugador A");
                usleep(5000000);
                endwin();

            }
            else if(strcmp(server_reply.tower, "WIB")==0){
                move(13,15);
                printw("Gano el jugador B");
                usleep(5000000);
                endwin();
            }
        }
        refresh();
    }
    getch();
}

struct option_menu_return *warriorMenuA(void *args)
{
    getCardDeck(populationA,warriorsA,4);
    struct params *param = args;
    int max_x,max_y;
    max_x = param->max_x;
    max_y = param->max_y;


    WINDOW *menuwin = newwin(6, max_x, max_y - 10, 0);

    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);
    keypad(menuwin, true);


    int choice;
    int highlight = 0;

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == highlight)
            {

                wattron(menuwin, A_REVERSE);
            }


            mvwprintw(menuwin, i + 1, 1, warriorsA[i]);
            wattroff(menuwin, A_REVERSE);

        }
        choice = wgetch(menuwin);

        switch (choice)
        {
            case KEY_UP:
                highlight--;
                if (highlight == -1)
                    highlight = 0;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight == 4)
                    highlight = 3;
                break;
            default:
                break;
        }
        if (choice == 10)
            break;
    }
    struct option_menu_return *response = malloc(sizeof *response);
    response->selected_option = attackMenu(max_x, max_y, param);
    response->warrior = highlight;
    return response;
}

struct option_menu_return *warriorMenuB(void *args)
{

    getCardDeck(populationB,warriorsB,4);
    struct params *param = args;
    int max_x,max_y;
    max_x = param->max_x;
    max_y = param->max_y;


    WINDOW *menuwin = newwin(6, max_x, max_y - 10, 0);

    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);


    keypad(menuwin, true);

    int choice;
    int highlight = 0;

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == highlight)
            {

                wattron(menuwin, A_REVERSE);
            }

            mvwprintw(menuwin, i + 1, 1, warriorsB[i]);
            wattroff(menuwin, A_REVERSE);

        }
        choice = wgetch(menuwin);

        switch (choice)
        {
            case KEY_UP:
                highlight--;
                if (highlight == -1)
                    highlight = 0;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight == 4)
                    highlight = 3;
                break;
            default:
                break;
        }
        if (choice == 10)
            break;
    }

    struct option_menu_return *response = malloc(sizeof *response);
    response->selected_option = attackMenu(max_x, max_y, param);
    response->warrior = highlight;
    return response;
}

char * attackMenu(int max_x, int max_y, void *params)
{
    WINDOW *menuwin = newwin(6, max_x, max_y - 10, 0);

    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin, true);

    char *attacks[2] = {"Lanzar hacia torre 1",
                        "Lanzar hacia torre 2"};
    int choice;
    int highlight = 0;

    while (1)
    {
        for (int i = 0; i < 2; i++)
        {
            if (i == highlight)
            {

                wattron(menuwin, A_REVERSE);

            }

            mvwprintw(menuwin, i + 1, 1, attacks[i]);
            wattroff(menuwin, A_REVERSE);

        }
        choice = wgetch(menuwin);

        switch (choice)
        {
            case KEY_UP:
                highlight--;
                if (highlight == -1)
                    highlight = 0;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight == 2)
                    highlight = 1;
                break;
            default:
                break;
        }
        if (choice == 10)
            break;
    }
    return attacks[highlight];
}

//----------------------------------------------------------------------------------
//Realiza la animacion para mover al guerrero hacia abajo.
void move_down(int startRow,int startCol,int level,char *warrior_char)
{
    int x=startRow;
    int y = startCol;

    char char_level[2];
    char warrior[10];
    sprintf(char_level, "%d", level);
    sprintf(warrior,"|%s|%s|",warrior_char,char_level);
    move(x,y);

    printw("%s",warrior);

    x++;
    move(x,y);

    printw("|\\|/|");




}


//----------------------------------------------------------------------------------
//Realiza la animacion para mover al guerrero hacia la derecha.
void move_right(int startRow,int startCol,int level,char *warrior_char)
{
    int x=startRow;
    int y = startCol;

    char char_level[2];
    char warrior1[10];
    char warrior2[10];
    sprintf(char_level, "%d", level);
    sprintf(warrior1,"|%s|>|",warrior_char);
    sprintf(warrior2,"|%s|>|",char_level);
    move(x,y);
    printw("%s",warrior1);
    x++;
    move(x,y);
    printw("%s",warrior2);


}

//----------------------------------------------------------------------------------
//Realiza la animacion para mover al guerrero hacia arriba.
void move_up(int startRow,int startCol,int level,char *warrior_char)
{
    int x=startRow;
    int y = startCol;

    char char_level[2];
    char warrior[10];
    sprintf(char_level, "%d", level);
    sprintf(warrior,"|%s|%s|",warrior_char,char_level);


    move(x,y);

    printw("|/|\\|");

    x++;
    move(x,y);

    printw("%s",warrior);




}


//----------------------------------------------------------------------------------
//Realiza la animacion para mover al guerrero hacia la izquierda.
void move_left(int startRow,int startCol,int level,char *warrior_char)
{
    init_pair(2, COLOR_RED, COLOR_GREEN); //Creamos unos pares de colores

    int x=startRow;
    int y = startCol;
    attron (COLOR_PAIR (2));


    char char_level[2];
    char warrior1[10];
    char warrior2[10];
    sprintf(char_level, "%d", level);
    sprintf(warrior1,"|<|%s|",warrior_char);
    sprintf(warrior2,"|<|%s|",char_level);

    move(x,y);

    printw("%s",warrior1);

    x++;
    move(x,y);

   printw("%s",warrior2);

    attroff(COLOR_PAIR (2));



}

//----------------------------------------------------------------------------------
//Borra un guerrero.
void erase_warrior(int startRow,int startCol)
{
    int x=startRow;
    int y = startCol;
    move(x,y);

    printw("     ");

    x++;
    move(x,y);

    printw("     ");


}
//----------------------------------------------------------------------------------
void *moveWarriorBTowerA2(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float) (warrior->attack + warrior->health + warrior->attack_speed) / 1500.0 * 9);
    int start_row = 17;
    int start_column = 51;
    int central_tower_column = 3;
    int ch;
    int loser = 0;
        //Se mueve hacia la torre A2.
        while (ch != 50)
        {
            ch = mvinch(start_row, start_column - 1) & A_CHARTEXT;
            if (!torreA2) {
                break;
            }
            //Se verifica si se debe peleaer con algun otro guerrero.
            if (ch != 32 && ch != 50) {
                move(15,15);
                printw("%s"," ");
                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&A2Q, &enemy);
                if (strcmp(fight(&enemy, warrior), "A") == 0) {
                    loser = 1;
                    break;
                } else if (strcmp(fight(&enemy, warrior), "tie") == 0) {
                    loser = 1;
                    break;
                }
            }
            //Se continua moviendo hacia la torre A2.
                move_left(start_row, start_column, level, "p");
                refresh();
                for (int s = 1; s <= 50000000; s++);
                erase_warrior(start_row, start_column);
                refresh();
                start_column--;

            }

        //Se ataca a la torreA2.
        if (!loser && towerA2) {
            if (strcmp(fight_tower(towerA2, warrior, "TA2"), "A") == 0) {
                move(10, 15);
                printw("%s", " ");
                loser = 1;
            } else if (strcmp(fight_tower(towerA2, warrior, "TA2"), "tie") == 0) {
                move(10, 15);
                printw("%s", " ");
                loser = 1;
            } else {
                move(17, 13);
                printw("    ");
                move(18, 13);
                printw("    ");
                torreA2 = 0;
            }
        }
        if(!loser){
            while (start_column != central_tower_column) {
                //Se verifica si se debe pelear con algun otro guerrero.
                ch = mvinch(start_row, start_column - 1) & A_CHARTEXT;
                if (ch != 32) {
                    move(15,15);
                    printw("%s"," ");
                    struct warrior_level *enemy = malloc(sizeof *enemy);
                    dequeue(&A2Q, &enemy);
                    if (strcmp(fight(&enemy, warrior), "A") == 0) {
                        loser = 1;
                        break;
                    } else if (strcmp(fight(&enemy, warrior), "tie") == 0) {
                        loser = 1;
                        break;
                    }
                }
                //Se camina hacia la columna de la torre central.
                move_left(start_row, start_column, level, "p");
                refresh();
                for (int s = 1; s <= 50000000; s++);
                erase_warrior(start_row, start_column);
                refresh();
                start_column--;
            }
        }
        if(!loser) {
        //Se camina hasta encontrar la torre central.
            while (ch != 65)
            {
                ch = mvinch(start_row - 1, start_column + 1) & A_CHARTEXT;
                move_up(start_row, start_column, level, "p");
                refresh();
                for (int s = 1; s <= 50000000; s++);
                erase_warrior(start_row, start_column);
                refresh();
                start_row--;

            }
            //Se ataca a la torre central.
            if (strcmp(fight_tower(towerAC, warrior, "TAC"), "A") == 0) {
                move(10, 15);
                printw("%s", " ");
                loser = 1;
            } else if (strcmp(fight_tower(towerAC, warrior, "TAC"), "tie") == 0) {
                move(10, 15);
                printw("%s", " ");
                loser = 1;
            } else {
                move(10, 3);
                printw("    ");
                move(11, 3);
                printw("    ");
                winnerB = 1;
                move(13,15);
                printw("Ha ganado el jugador B");
                winner(2);
            }
        }
    struct warrior_level delete;
    dequeue(&A2Q,&delete);
    }


void *moveWarriorBTowerA1(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 4;
    int start_column = 51;
    int central_tower_column = 3;
    int ch;
    int loser = 0;
        //Se camina hasta encontrar la torre A1
        while (ch != 49)
        {
            ch = mvinch(start_row, start_column - 1) & A_CHARTEXT;
            if(!torreA1){
                break;
            }
            //Se ataca a la torre A1.
            if (ch != 32 && ch!=49) {
                move(15,15);
                printw("%s"," ");
                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&A1Q,&enemy);
                if(strcmp(fight(&enemy,warrior),"A")==0){
                    loser = 1;
                    break;
                }
                else if(strcmp(fight(&enemy,warrior),"tie")==0){
                    loser = 1;
                    break;
                }
            }
            //Se mueve continua moviendo hacia la torre a1.
            move_left(start_row, start_column,level,"p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_column--;

        }
    //Se ataca a la torre A1.
    if(!loser && towerA1) {
        if (strcmp(fight_tower(towerA1, warrior, "TA1"), "A") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else if (strcmp(fight_tower(towerA1, warrior, "TA1"), "tie") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else {
            move(4, 13);
            printw("    ");
            move(5, 13);
            printw("    ");
            torreA1 = 0;
        }
    }
    if(!loser){

        while (start_column != central_tower_column) {
            ch = mvinch(start_row, start_column - 1) & A_CHARTEXT;
           //Se verifica si se debe pelear con los guerreros.
            if (ch != 32) {
                move(15,15);
                printw("%s"," ");
                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&A1Q,&enemy);
                if(strcmp(fight(&enemy,warrior),"A")==0){
                    loser = 1;
                    break;
                }
                else if(strcmp(fight(&enemy,warrior),"tie")==0){
                    loser = 1;
                    break;
                }
            }
            //Se continua moviendo hasta la columna de la torre.
            move_left(start_row, start_column, level, "p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_column--;

        }
    }
    if(!loser) {
        //Se camina hasta encontrar la torre central.
        while (ch != 65)
        {
            ch = mvinch(start_row + 2, start_column + 1) & A_CHARTEXT;
            move_down(start_row, start_column, level, "p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_row++;
        }

        //Se ataca a la torre central.
        if (strcmp(fight_tower(towerAC, warrior, "TAC"), "A") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else if (strcmp(fight_tower(towerAC, warrior, "TAC"), "tie") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else {
            move(10, 3);
            printw("    ");
            move(11, 3);
            printw("    ");
            winnerB = 1;
            move(13,15);
            printw("Ha ganado el jugador B");
            winner(2);

        }
    }
    struct warrior_level delete;
    dequeue(&A1Q,&delete);
}


void *moveWarriorATowerB1(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 4;
    int start_column = 0;
    int central_tower_column = 48;
    int ch;
    int loser = 0;
        //Se camina hasta encontrar la torre b1.
        while (ch != 84)
        {
            ch = mvinch(start_row, start_column + 5) & A_CHARTEXT;
            if(!torreB1){
                break;
            }
            //Se verifica si se debe pelear con algun enemigo.
            if (ch != 32 && ch != 84) {
                move(15,15);
                printw("%s"," ");


                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&B1Q,&enemy);
                if(strcmp(fight(warrior,&enemy),"B")==0){
                    loser = 1;
                    break;
                }
                else if(strcmp(fight(warrior,&enemy),"tie")==0){
                    loser = 1;
                    break;
                }
            }
            //Se continua moviendo hasta la torre b1
            move_right(start_row, start_column,level,"p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_column++;

        }


    if(!loser && towerB1) {
        //Se pelea con la torre b1
        if (strcmp(fight_tower(towerB1, warrior, "TB1"), "A") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else if (strcmp(fight_tower(towerB1, warrior, "TB1"), "tie") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else {
            move(4, 41);
            printw("    ");
            move(5, 41);
            printw("    ");
            torreB1 = 0;
        }
    }
    if(!loser){
        while (start_column != central_tower_column) {
            //Se verifica si hay lucha de guerreros.
            ch = mvinch(start_row, start_column + 5) & A_CHARTEXT;
            if (ch != 32) {
                move(15,15);
                printw("%s"," ");
                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&B1Q,&enemy);
                if(strcmp(fight(warrior,&enemy),"B")==0){
                    loser = 1;
                    break;
                }
                else if(strcmp(fight(warrior,&enemy),"tie")==0){
                    loser = 1;
                    break;
                }
            }
            //Se continua caminando hacia la columna de la torre central.
            move_right(start_row, start_column, level, "p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_column++;

        }
    }
    if(!loser) {
        //Se camina hacia la torre central.
        while (ch != 66)
        {
            ch = mvinch(start_row + 2, start_column + 1) & A_CHARTEXT;
            move_down(start_row, start_column, level, "p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_row++;
        }
        //Se ataca a la torre central.
        if (strcmp(fight_tower(towerBC, warrior, "TBC"), "A") == 0) {
            move(15, 10);
            printw("%s", " ");
            loser = 1;
        } else if (strcmp(fight_tower(towerBC, warrior, "TBC"), "tie") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else {
            move(10, 48);
            printw("    ");
            move(11, 48);
            printw("    ");
            winnerA = 1;
            move(13,15);
            printw("Ha ganado el jugador A");
            winner(1);
        }
    }
    struct warrior_level delete;
    dequeue(&B1Q,&delete);
}


void *moveWarriorATowerB2(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 17;
    int start_column = 0;
    int central_tower_column = 48;
    int ch;
    int loser = 0;
    //Se camina hasta la torre b2
    while (ch != 84)
        {
            // Se verifica si se debe pelear con algun guerrero.
            ch = mvinch(start_row, start_column + 5) & A_CHARTEXT;
            if(!torreB2){
                break;
            }
            if (ch != 32 && ch != 84) {
                move(15,15);
                printw("%s"," ");
                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&B2Q,&enemy);
                if(strcmp(fight(warrior,&enemy),"B")==0){
                    loser = 1;
                    break;
                }
                else if(strcmp(fight(warrior,&enemy),"tie")==0){
                    loser = 1;
                    break;
                }
            }
            //Se continua moviendo hacia la derecha.
            move_right(start_row, start_column,level,"p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_column++;

        }

    //Se ataca a la torre defensora b2
    if(!loser && towerB2) {
        if (strcmp(fight_tower(towerB2, warrior, "TB2"), "A") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else if (strcmp(fight_tower(towerB2, warrior, "TB2"), "tie") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else {
            move(17, 41);
            printw("    ");
            move(18, 41);
            printw("    ");
            torreB2 = 0;
        }
    }
    if(!loser){
        //Se mueve hasta la misma columna de la torre central.
        while (start_column != central_tower_column) {

            //Se verifica si debe pelear con algun guerrero
            ch = mvinch(start_row, start_column + 5) & A_CHARTEXT;
            if (ch != 32) {
                move(15,15);
                printw("%s"," ");
                struct warrior_level *enemy = malloc(sizeof *enemy);
                dequeue(&B2Q,&enemy);
                if(strcmp(fight(warrior,&enemy),"B")==0){
                    loser = 1;
                    break;
                }
                else if(strcmp(fight(warrior,&enemy),"tie")==0){
                    loser = 1;
                    break;
                }
            }
            //Se mueve hacia la derecha hasta la columna de la torre central
            move_right(start_row, start_column, level, "p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_column++;
        }
    }
    if(!loser) {

        //Se mueve hasta encontrar la torre central.
        while (ch != 66)
        {
            ch = mvinch(start_row - 1, start_column + 1) & A_CHARTEXT;
            move_up(start_row, start_column, level, "p");
            refresh();
            for (int s = 1; s <= 50000000; s++);
            erase_warrior(start_row, start_column);
            refresh();
            start_row--;

        }

        //Se ataca a la torre central.
        if (strcmp(fight_tower(towerBC, warrior, "TBC"), "A") == 0) {
            move(15, 10);
            printw("%s", " ");
            loser = 1;
        } else if (strcmp(fight_tower(towerBC, warrior, "TBC"), "tie") == 0) {
            move(10, 15);
            printw("%s", " ");
            loser = 1;
        } else {
            move(10, 48);
            printw("    ");
            move(11, 48);
            printw("    ");
            winnerA = 1;
            move(13,15);
            printw("Ha ganado el jugador A");
            winner(1);
        }
    }
    struct warrior_level delete;
    dequeue(&B2Q,&delete);

}

void *deployWarriorA1(void *params) {
    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 4;
    int start_column = 15;

    int ch;
    // Ciclo que mueve el guerrero hasta el limite de la pantalla para que se despliegue
    // en la siguiente pantalla
    while (start_column != 51)
    {
        // Se comprueba si existe un guerrero enfrente.
        ch = mvinch(start_row, start_column + 5) & A_CHARTEXT;
        if (ch != 32) {
            move(16,15);
            printw("%s"," ");
            struct warrior_level *enemy = malloc(sizeof *enemy);
            dequeue(&A1Q,&enemy);
            if(strcmp(fight(warrior,&enemy),"B")==0){
                loserA1 = 1;
                break;
            }
            else if(strcmp(fight(warrior,&enemy),"tie")==0){
                loserA1 = 1;
                break;
            }
        }
        //Se mueve hasta encontrar el limite de la pantalla.
        move_right(start_row, start_column,level,"p");
        refresh();
        for (int s = 1; s <= 50000000; s++);
        erase_warrior(start_row, start_column);
        refresh();
        start_column++;
    }
    struct warrior_level delete;
    dequeue(&A1Q,&delete);

}


void *deployWarriorA2(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 17;
    int start_column = 15;

    int ch;
    // Ciclo que mueve el guerrero hasta el limite de la pantalla para que se despliegue
    // en la siguiente pantalla
    while (start_column != 51)
    {
        // Se comprueba si existe un guerrero enfrente.
        ch = mvinch(start_row, start_column + 5) & A_CHARTEXT;
        if (ch != 32) {
            move(16,15);
            printw("%s"," ");
            struct warrior_level *enemy = malloc(sizeof *enemy);
            dequeue(&A2Q,&enemy);
            if(strcmp(fight(warrior,&enemy),"B")==0){
                loserA2 = 1;
                break;
            }
            else if(strcmp(fight(warrior,&enemy),"tie")==0){
                loserA2 = 1;
                break;
            }
        }
        //Se mueve hasta encontrar el limite de la pantalla.
        move_right(start_row, start_column,level,"p");
        refresh();
        for (int s = 1; s <= 50000000; s++);
        erase_warrior(start_row, start_column);
        refresh();
        start_column++;
    }

    struct warrior_level delete;
    dequeue(&A2Q,&delete);
}


void *deployWarriorB1(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 4;
    int start_column = 36;

    int ch;
    // Ciclo que mueve el guerrero hasta el limite de la pantalla para que se despliegue
    // en la siguiente pantalla

    while (start_column != 0)
    {
        // Se comprueba si existe un guerrero enfrente.
        ch = mvinch(start_row, start_column - 1) & A_CHARTEXT;
        if (ch != 32) {
            move(16,15);
            printw("%s"," ");
            struct warrior_level *enemy = malloc(sizeof *enemy);
            dequeue(&B1Q,&enemy);
            if(strcmp(fight(&enemy,warrior),"A")==0){
                loserB1 = 1;
                break;
            }
            else if(strcmp(fight(&enemy,warrior),"tie")==0){
                loserB1 = 1;
                break;
            }
        }
        //Se mueve hasta encontrar el limite de la pantalla.
        move_left(start_row, start_column,level,"p");
        refresh();
        for (int s = 1; s <= 50000000; s++);
        erase_warrior(start_row, start_column);
        refresh();
        start_column--;
    }

    struct warrior_level delete;
    dequeue(&B1Q,&delete);
}


void *deployWarriorB2(void *params) {

    struct warrior_level *warrior = params;
    int level = (int) round((float)(warrior->attack+warrior->health+warrior->attack_speed)/1500.0*9);
    int start_row = 17;
    int start_column = 36;

    int ch;

    // Ciclo que mueve el guerrero hasta el limite de la pantalla para que se despliegue
    // en la siguiente pantalla

    while (start_column != 0)
    {

        // Se comprueba si existe un guerrero enfrente.
        ch = mvinch(start_row, start_column - 1) & A_CHARTEXT;
        if (ch != 32) {
            move(16,15);
            printw("%s"," ");
            struct warrior_level *enemy = malloc(sizeof *enemy);
            dequeue(&B2Q,&enemy);

            if(strcmp(fight(&enemy,warrior),"A")==0){
                loserB2 = 1;
                break;
            }
            else if(strcmp(fight(&enemy,warrior),"tie")==0){
                loserB2 = 1;
                break;
            }
        }

        //Se mueve hasta encontrar el limite de la pantalla.
        move_left(start_row, start_column,level,"p");
        refresh();
        for (int s = 1; s <= 50000000; s++);
        erase_warrior(start_row, start_column);
        refresh();
        start_column--;
    }

    struct warrior_level delete;
    dequeue(&B2Q,&delete);
}



void winner(int winner) {
    if (winner == 1) {
        struct send_warrior sendWarrior;
        strcpy(sendWarrior.tower, "WIA");
        send(sock, (void *) &sendWarrior, sizeof(sendWarrior), 0);
    } else {
        struct send_warrior sendWarrior;
        strcpy(sendWarrior.tower, "WIB");
        send(sock, (void *) &sendWarrior, sizeof(sendWarrior), 0);
    }
}

    void changeSide(int towerNum,struct warrior_level warrior) {
    if (towerNum == 1) {
        struct send_warrior sendWarrior;
        strcpy(sendWarrior.tower, "W1A");
        sendWarrior.warrior = warrior;
        send(sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
    } else if (towerNum == 2) {
        struct send_warrior sendWarrior;
        strcpy(sendWarrior.tower, "W2A");
        sendWarrior.warrior = warrior;
        send(sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
    } else if (towerNum == 3) {
        struct send_warrior sendWarrior;
        strcpy(sendWarrior.tower, "W1B");
        sendWarrior.warrior = warrior;
        send(sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
    }
    else{
        struct send_warrior sendWarrior;
        strcpy(sendWarrior.tower, "W2B");
        sendWarrior.warrior = warrior;
        send(sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
    }
}





