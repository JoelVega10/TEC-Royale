#include "RoundRobinScheduler.h"
#include "MyThread.h"

extern TCBQueue threadsQueue;
extern struct itimerval timeQuantum;
extern long timeInterval;
extern sigset_t sigProcMask;

static TCB getRoundRobinHead(TCBQueue pQueue);

/* DESCRIPCION:
 * Algoritmo de round robin. */
void roundRobin()
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    TCB currentThread = getRoundRobinHead(threadsQueue);
    TCB previousThread = threadsQueue->currentThread;
    if(previousThread == NULL && currentThread == NULL)
    {
        //Deadlock
        exit(1);
    }
    else if(currentThread == NULL)
    {
        threadsQueue->currentThreadCopy = threadsQueue->currentThread;
        threadsQueue->currentThread = NULL;
        sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
        my_thread_yield();
    }
    else if(previousThread == NULL && currentThread != threadsQueue->currentThreadCopy)
    {
        threadsQueue->currentThread = currentThread;
        timeQuantum.it_value.tv_usec = timeInterval;
        if(threadsQueue->currentThreadCopy != NULL)
        {

            TCB auxTCB = threadsQueue->currentThreadCopy;
            threadsQueue->currentThreadCopy = NULL;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            swapcontext(&(auxTCB->t_Context), &(currentThread->t_Context));
        }
        else
        {
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            setcontext(&(currentThread->t_Context));
        }
    }
    else
    {
        if(previousThread != NULL && previousThread != currentThread)
        {
            threadsQueue->currentThread = currentThread;
            threadsQueue->currentThreadCopy = NULL;
            timeQuantum.it_value.tv_usec = timeInterval;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            swapcontext(&(previousThread->t_Context), &(currentThread->t_Context));
        }
        else
        {
            threadsQueue->currentThread = currentThread;
        }
        sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    }
}

/* DESCRIPCION:
 * Obtiene el proximo hilo a ejecutar de la cola de hilos pendientes. */
static TCB getRoundRobinHead(TCBQueue pQueue)
{
    TCB result = NULL;
    int count = 0;
    if(getHeadQueue(pQueue)->threadCompleted == 0)
    {
        moveForward(pQueue);
    }

    TCB headThread = getHeadQueue(pQueue);
    while(count < getNodeCountQueue(pQueue))
    {

        if(getNodeRoundRobinCount(pQueue) == 0)
        {
            result = NULL;
            break;
        }
        else if(headThread->threadCompleted && headThread->roundRobinScheduler)
        {
            if(pQueue->currentThread == headThread)
            {
                pQueue->currentThread = NULL;
            }
            deleteHeadThread(pQueue);
            headThread = getHeadQueue(pQueue);
        }
        else if(headThread->threadBlocked || headThread->lotteryScheduler)
        {
            moveForward(pQueue);
            headThread = getHeadQueue(pQueue);
            count++;
        }
        else
        {
            result = headThread;
            break;
        }
    }
    return result; 
}