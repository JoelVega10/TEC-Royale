#include "MyMutexDataStructure.h"
#include "MyThreadErrors.h"

MutexQueue mutexQueue = NULL;

/* DESCRIPCION:
 * Devuelve 1 si el mutex se encuentra en la cola. */
int isMutexInQueue(thread_mutex_t *pMutex)
{
    if(mutexQueue == NULL || pMutex == NULL)
    {
        return 0;
    }
    else
    {
    	MutexQueue auxQueue = mutexQueue;
	    while((auxQueue != NULL)  && (auxQueue->mutex->lockNumber != pMutex->lockNumber))
	    {
	        auxQueue = auxQueue->nextMutex;
	    }
	    if(auxQueue == NULL)
	    {
	    	return 0;
	    }
	    else
	    {
	    	return 1;
	    }
    }
}

/* DESCRIPCION:
 * Crea una nueva cola de mutex. */
MutexQueue createNewMutexQueue()
{
    MutexQueue auxQueue = (MutexQueue)malloc(sizeof(struct MutexQueueNodes));
    if(auxQueue != NULL)
    {
        auxQueue->mutex = NULL;
        auxQueue->nextMutex = NULL;
        auxQueue->threadNodeList = NULL;
        return auxQueue;
    }
    else
    {
    	return NULL;
    }
}

/* DESCRIPCION:
 * Crea un nuevo nodo de la cola de mutex globales
 * para almacenar un mutex. */
thread_mutex_t* createNewMutexNode()
{
    static long currentMutexValue = 0;
    thread_mutex_t *newMutexNode = (thread_mutex_t*)malloc(sizeof(struct MutexNode));
    newMutexNode->lockNumber = ++currentMutexValue;
    return newMutexNode;
}

/* DESCRIPCION:
 * Agrega un nuevo nodo a la cola de mutex. */
void insertMutexQueue(MutexQueue pMutex)
{
    if(pMutex != NULL)
    {
        pMutex->nextMutex = mutexQueue;
        mutexQueue = pMutex;
    }
}

/* DESCRIPCION:
 * Busca si el mutex ingresado se encuentra en la cola de mutex. */
MutexQueue searchMutexQueue(thread_mutex_t *pMutex)
{
    if(mutexQueue == NULL || pMutex == NULL)
    {
        return NULL;
    }
    else
    {
    	MutexQueue auxQueue = mutexQueue;
	    while((auxQueue != NULL)  && (auxQueue->mutex->lockNumber != pMutex->lockNumber))
	    {
	        auxQueue = auxQueue->nextMutex;
	    }
	    return auxQueue;
    }
}