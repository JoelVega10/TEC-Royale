#include "RealTimeScheduler.h"
#include "LotteryScheduler.h"
#include "RoundRobinScheduler.h"
#include "MyThread.h"

extern TCBQueue threadsQueue;
extern sigset_t sigProcMask;

int roundRobinControl = 0;
int sortControl = 0;

static void changeScheduler();
static void checkThreads();
static void checkThread(TCB pThread);

/* DESCRIPCION:
 * Algoritmo de real time*/
void realTime()
{
	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
	threadsQueue->activeQuantums++;
	checkThreads();
	changeScheduler();
}

/* DESCRIPCION:
 * Se encarga de iterar sobre todos los hilos del queue y
 * le aplica la funcion de checkThread. */
static void checkThreads()
{
	TCB headThread = getHeadQueue(threadsQueue);
	TCB next_thread = headThread->next_thread;
    if(headThread != NULL)
    {
    	checkThread(headThread);
        while(headThread != next_thread)
        {
        	checkThread(next_thread);
            next_thread = next_thread->next_thread;
        }
    }
}

/* DESCRIPCION:
 * Asigna el scheduler individual a cada uno de los hilos, en caso de ser lottery
 * asigna los tiquetes segun su nivel de prioridad. */
static void checkThread(TCB pThread)
{
	if(pThread->timeLimit > 0 && pThread->warningLevel > 0 && pThread->threadCompleted == 0)
	{
		if(pThread->roundRobinScheduler && pThread->warningLevel == 1)
		{
			my_thread_chsched(pThread);
			giveTickets(threadsQueue, pThread, PRIORITY_ONE);
			pThread->ultimateWarningLevel = pThread->warningLevel;
			pThread->warningLevel = 0;
		}
		else if(pThread->warningLevel == 1)
		{
			giveTickets(threadsQueue, pThread, PRIORITY_ONE);
			pThread->ultimateWarningLevel = pThread->warningLevel;
			pThread->warningLevel = 0;
		}
		else if(pThread->warningLevel == 2)
		{
			giveTickets(threadsQueue, pThread, PRIORITY_TWO);
			pThread->ultimateWarningLevel = pThread->warningLevel;
			pThread->warningLevel = 0;
		}
		else if(pThread->warningLevel == 3)
		{
			giveTickets(threadsQueue, pThread, PRIORITY_THREE);
			pThread->ultimateWarningLevel = pThread->warningLevel;
			pThread->warningLevel = 0;
		}
	}
}

/* DESCRIPCION:
 * Ejecuta el cambio de scheduler en el contexto del usuario actual. */
static void changeScheduler()
{
	int sortCount = getNodeLotteryCount(threadsQueue);
	int roundRobinCount = getNodeRoundRobinCount(threadsQueue);
	if(sortControl &&  sortCount > 0 && roundRobinCount == 0)
	{
		roundRobinControl = 0;
        lotteryScheduler();
	}
	else if(roundRobinControl && roundRobinCount > 0 && sortCount == 0)
	{
		sortControl = 0;
		roundRobin();
	}
	else if(roundRobinControl && sortCount > 0)
	{
		roundRobinControl = 0;
		sortControl = 1;
        lotteryScheduler();
	}
	else if(sortControl && roundRobinCount > 0)
	{
		sortControl = 0;
		roundRobinControl = 1;
		roundRobin();
	}
	else
	{
		exit(0);
	}
}