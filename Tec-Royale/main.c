#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ini.h"
#include "GameServer.h"
#include "mypthreads/MyThread.h"


char *configFileName = NULL;

int main(int argc, char **argv)
{

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
    startGameServer();

  return 0;
}
