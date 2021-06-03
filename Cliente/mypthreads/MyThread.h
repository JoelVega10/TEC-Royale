#ifndef _MYTHREAD_H
#define	_MYTHREAD_H

#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "DataStructures.h"
#include "MyMutexDataStructure.h"

/*
 *
 * FUNCIONES DE MYPTHREADS
 *
 * */


void my_thread_init(long pTimeInterval);
int my_thread_create(thread_t *pThread, void *(*pStartRoutine)(void *), void *pArgument, int pLimitTime, char *pSchedulerType);
void my_thread_yield(void);
void myThreadExit(void *pReturnValue);
void my_thread_chsched(TCB pThread);
int my_thread_end(thread_t pThread);
int my_thread_join(thread_t pThread, void **pStatus);
int my_thread_detach(thread_t pThread);

//FUNCIONES DE MUTEX

/*
 *
 * FUNCIONES DE MUTEX
 *
 * */

int my_mutex_init(thread_mutex_t *pMutex);
int my_mutex_lock(thread_mutex_t *pMutex);
int my_mutex_trylock(thread_mutex_t *pMutex);
int my_mutex_unlock(thread_mutex_t *pMutex);

#endif