#include "GameServer.h"
#include "mypthreads/MyThread.h"

#define DELAY 30000

int socket_desc , client_sock , c , *new_sock, *new_sock2;
struct sockaddr_in server , client;
ini_t *config = 0;


//Inicia el parser de archivos .ini y carga a memoria principal el archivo de configuracion
void loadConfigurationFile(char * configFileName){
  config = ini_load(configFileName);
  if (!config)
  {
    printf("Error al abrir el archivo de configuración. Verifique el nombre de nuevo. \n");
	exit(0);
  }
}

//Maneja la comunicacion entre server-cliente1-cliente2
void connection_handlerPlayerOne(struct sockets * args)
{

    int read_size;
    char * message = "1",  client_message[3];
    write(args->new_sock , message , strlen(message));

    struct send_warrior sendWarrior;
    while( (recv(args->new_sock, (struct send_warrior *)&sendWarrior, sizeof(sendWarrior), 0)) > 0 )
    {
        printf("OK : %s\n",sendWarrior.tower);
        if(strcmp(sendWarrior.tower, "W1B")==0){
            send(args->new_sock2, (void *)&sendWarrior, sizeof(sendWarrior), 0);
        }
        else if(strcmp(sendWarrior.tower, "W2B")==0){
            send(args->new_sock2, (void *)&sendWarrior, sizeof(sendWarrior), 0);
        }
        else if(strcmp(sendWarrior.tower, "WIB")==0){
            send(args->new_sock2, (void *)&sendWarrior, sizeof(sendWarrior), 0);
        }
    }


    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }



}
//Maneja la comunicacion entre server-cliente2-cliente1
void connection_handlerPlayerTwo(struct sockets * args)
{

    int read_size;
    char * message = "2", client_message[3];
    write(args->new_sock2 , message , strlen(message));
    struct send_warrior sendWarrior;
    while( (recv(args->new_sock2, (struct send_warrior *)&sendWarrior, sizeof(sendWarrior), 0)) > 0 )
    {
        //Send the message back to client
        printf("OK : %s\n",sendWarrior.tower);
        if(strcmp(sendWarrior.tower, "W1A")==0){
            send(args->new_sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
        }
        else if(strcmp(sendWarrior.tower, "W2A")==0){
            send(args->new_sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
        }
        else if(strcmp(sendWarrior.tower, "WIA")==0){
            send(args->new_sock, (void *)&sendWarrior, sizeof(sendWarrior), 0);
        }
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }


}

//Encargado de iniciar la conexion TCP y aceptar los clientes 1 y 2 en hilos separados.
void startGameServer(){
    my_thread_init(200);
    socket_desc = createSocket();
    bindSocket(server);
    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for Player1 to connect...");
    c = sizeof(struct sockaddr_in);

    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    puts("Player 1 Connection accepted");
    thread_t t1;
    new_sock = malloc(1);
    *new_sock = client_sock;



    puts("Jugador1 asignado");

    if (client_sock < 0)
    {
        perror("accept failed");
        //exit(0);
    }


    //Accept and incoming connection
    puts("Waiting for Player2 to connect...");
    c = sizeof(struct sockaddr_in);

    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    puts("Player 2 Connection accepted");
    thread_t t2;
    new_sock2 = malloc(1);
    *new_sock2 = client_sock;
    struct sockets *param = malloc(sizeof *param);

    param->new_sock = *new_sock;
    param->new_sock2 = * new_sock2;

    if(my_thread_create(&t1 ,connection_handlerPlayerOne, param,50,"Sort") < 0)
    {
        perror("could not create thread");
        exit(0);
    }

    if(my_thread_create(&t2,connection_handlerPlayerTwo, param,50,"Sort") < 0)
    {
        perror("could not create thread");
        exit(0);
    }
    puts("Jugador2 asignado");

    //Now join the thread , so that we dont terminate before the thread
    my_thread_join( t1 , NULL);
    my_thread_join( t2 , NULL);

    while (1){

    }

    exit(0);
}

//Genera el socket para lograr la conexion entre servidor-clientes.
int createSocket(){
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    int option = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    return socket_desc;
}

//Hace bind al socket con el puerto 8888.
void bindSocket(struct sockaddr_in server){
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
    }
    puts("bind done");
}

