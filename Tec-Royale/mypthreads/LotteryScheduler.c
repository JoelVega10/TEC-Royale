#include "LotteryScheduler.h"
#include "MyThread.h"

extern TCBQueue threadsQueue;
extern struct itimerval timeQuantum;
extern long timeInterval;
extern sigset_t sigProcMask;


static TCB getLotteryHead(TCBQueue pQueue);
static void deleteValueThreadGenerates(int *pGenerateThreads, int pValue, int pInitialLotteryThreads);
static void insertValueThreadGenerates(int *pGenerateThreads, int pValue, int pInitialLotteryThreads);
static int existInThreadGenerates(int *pGenerateThreads, int pValue, int pInitialLotteryThreads);
static int generateNewTicket(TCBQueue pQueue, int *pGenerateThreads, int pInitialLotteryThreads);
static int getThreadGeneratesCount(int *pGenerateThreads, int pInitialLotteryThreads);
static int detectAllThreadsBlocked(TCBQueue pQueue);

int randomTicket = 0;


/* DESCRIPCION:
 * Algoritmo de sorteo */
void lotteryScheduler()
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    TCB currentThread = getLotteryHead(threadsQueue);
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
 * Itera sobre los hilos generados y elimina aquellos hilos
 * que coincidan con pValue*/
static void deleteValueThreadGenerates(int *pGenerateThreads, int pValue, int pInitialLotteryThreads)
{
	int count = 0;
	while(count < pInitialLotteryThreads)
	{
		if(pGenerateThreads[count] == pValue)
		{
			pGenerateThreads[count] = 0;
			break;
		}
		else
		{
			count++;
		}
	}

}

/* DESCRIPCION:
 * Itera sobre los hilos generados e inserta el pValue en los que son 0. */
static void insertValueThreadGenerates(int *pGenerateThreads, int pValue, int pInitialLotteryThreads)
{
	int count = 0;
	while(count < pInitialLotteryThreads)
	{
		if(pGenerateThreads[count] == 0 && !existInThreadGenerates(pGenerateThreads, pValue, pInitialLotteryThreads))
		{
			pGenerateThreads[count] = pValue;
			break;
		}
		else
		{
			count++;
		}
	}
}

/* DESCRIPCION:
 * Devuelve la cantidad de hilos generados que sean distintos de 0. */
static int getThreadGeneratesCount(int *pGenerateThreads, int pInitialLotteryThreads)
{
	int countArray = 0;
	int count = 0;
	while(countArray < pInitialLotteryThreads)
	{
		if(pGenerateThreads[countArray] != 0)
		{
			count++;
			countArray++;
		}
		else
		{
			countArray++;
		}
	}
	return count;
}

/* DESCRIPCION:
 * Devuelve 1 si el pValue se encuentra entre los hilos generados.*/
static int existInThreadGenerates(int *pGenerateThreads, int pValue, int pInitialLotteryThreads)
{
	int count = 0;
	int result = 0;
	while(count < pInitialLotteryThreads)
	{
		if(pGenerateThreads[count] == pValue)
		{
			result = 1;
			break;
		}
		else
		{
			count++;
		}
	}
	return result;
}

/* DESCRIPCION:
 * Itera sobre la cola de hilos y devuelve los que se encuentran
 * bloqueados. */
static int detectAllThreadsBlocked(TCBQueue pQueue)
{
	int result = 1;
	TCB headThread = getHeadQueue(pQueue);
    TCB next_thread = headThread->next_thread;
    if(headThread->lotteryScheduler && !headThread->threadBlocked)
    {
    	result = 0;
    }
    else
    {
    	while(next_thread != headThread)
	    {
	        if(next_thread->lotteryScheduler && !next_thread->threadBlocked)
	        {
	            result = 0;
	            break;
	        }
	        else
	        {
                next_thread = next_thread->next_thread;
	        }
	    }
    }
    return result;
}

/* DESCRIPCION:
 * Se encarga de generar un nuevo ticket para un hilo disponible. */
static int generateNewTicket(TCBQueue pQueue, int *pGenerateThreads, int pInitialLotteryThreads)
{
	int randomTicket = 0;
	do
	{
		randomTicket = ((rand() % (searchLastTicket(pQueue) - 1)) + 1);
	}while(existInThreadGenerates(pGenerateThreads, (int)searchThreadTicket(randomTicket, pQueue)->threadID, pInitialLotteryThreads));
	return randomTicket;
}

/* DESCRIPCION:
 * Devuelve la cabeza del queue de tiquetes que corresponde al queue que le esta entrando. */
static TCB getLotteryHead(TCBQueue pQueue)
{
	TCB result = NULL;
	int initialSortThreads = getNodeLotteryCount(pQueue);
	int *generateThreads = (int*)calloc(initialSortThreads, sizeof(int));
	randomTicket = generateNewTicket(pQueue, generateThreads, initialSortThreads);
	TCB headThread = searchThreadTicket(randomTicket, pQueue);
    if(detectAllThreadsBlocked(pQueue))
    {
        randomTicket = 0;
        result = NULL;
    }
    else
    {
    	while(getThreadGeneratesCount(generateThreads, initialSortThreads) <= getNodeLotteryCount(pQueue))
	    {
	        if(headThread->threadCompleted && headThread->lotteryScheduler)
	        {
	            if(pQueue->currentThread == headThread)
	            {
	                pQueue->currentThread = NULL;
	            }
	            deleteValueThreadGenerates(generateThreads, (int)headThread->threadID, initialSortThreads);
	            deleteLotteryThread(headThread->threadID, pQueue);
	            if(getNodeLotteryCount(pQueue) == 0 || detectAllThreadsBlocked(pQueue))
		        {
		            randomTicket = 0;
		            result = NULL;
		            break;
		        }
		        else
		        {
		        	randomTicket = generateNewTicket(pQueue, generateThreads, initialSortThreads);
		        	headThread = searchThreadTicket(randomTicket, pQueue);
		        	insertValueThreadGenerates(generateThreads, (int)headThread->threadID, initialSortThreads);
		        }
	        }
	        else if(headThread->threadBlocked || headThread->roundRobinScheduler)
	        {
	            randomTicket = generateNewTicket(pQueue, generateThreads, initialSortThreads);
	        	headThread = searchThreadTicket(randomTicket, pQueue);
	        	insertValueThreadGenerates(generateThreads, (int)headThread->threadID, initialSortThreads);
	        }
	        else
	        {
	            result = headThread;
	            break;
	        }
	    }
    }
    free(generateThreads);
    return result; 
}