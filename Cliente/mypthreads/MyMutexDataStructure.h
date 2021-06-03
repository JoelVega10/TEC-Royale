#ifndef _MYMUTEXDATASTRUCTURE_H
#define _MYMUTEXDATASTRUCTURE_H

#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "DataStructures.h"

/*
 *  ESTRUCTURAS DE DATOS PARA LOS MUTEX
 * */

typedef struct MutexNode
{
   long lockNumber;
} thread_mutex_t;

typedef struct MutexQueueNodes
{
    thread_mutex_t *mutex;
    struct MutexQueueNodes *nextMutex;
    WaitingThreadsQueue threadNodeList;
} *MutexQueue;

/*
 *
 * FUNCIONES DE LAS ESTRUCTURAS DE DATOS DE MUTEX
 *
 * */

MutexQueue searchMutexQueue(thread_mutex_t *pMutex);
MutexQueue createNewMutexQueue();
thread_mutex_t *createNewMutexNode();
void insertMutexQueue(MutexQueue pMutex);
int isMutexInQueue(thread_mutex_t *pMutex);

#endif