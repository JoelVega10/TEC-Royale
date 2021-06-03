#include "DataStructures.h"
#include "MyThreadErrors.h"

static long _currentThreadID = 0;

/* DESCRIPCION:
 * Devuelve el threaID que se va a utilizar al crear un nuevo thread. */
static int getThreadID()
{
    return ++_currentThreadID;
}

/* DESCRIPCION:
 * Obtiene el thread principal de la cola, el thread
 * que se encuentra en la cabeza de la cola. */
TCB getHeadQueue(TCBQueue pQueue)
{
    if(pQueue == NULL)
    {
        return NULL;
    }
    else
    {
    	return pQueue->head;
    }
}

/* DESCRIPCION:
 * Obtiene el thread que le sigue a la cabeza. */
TCB getNextHead(TCBQueue pQueue)
{
    if(pQueue != NULL && pQueue->head != NULL)
    {
        return pQueue->head->next_thread;
    }
    else
    {
    	return NULL;
    }
}

/* DESCRIPCION:
 * Crea un nuevo nodo para un thread. */
TCB createNewNode()
{
    TCB newTCB = (TCB)malloc(sizeof(struct Nodes));
    if(newTCB == NULL)
    {
    	free(newTCB);
        return NULL;
    }
    else
    {
	    newTCB->t_Context.uc_stack.ss_sp = malloc(STACKSIZE);
	    if(newTCB->t_Context.uc_stack.ss_sp == NULL)
	    {
	        free(newTCB);
	        return NULL;
	    }
	    else
	    {
	    	newTCB->t_Context.uc_stack.ss_size = STACKSIZE;
		    newTCB->t_Context.uc_stack.ss_flags = 0;
	    	newTCB->next_thread = NULL;
		    newTCB->t_Context.uc_link = 0;
		    newTCB->threadCompleted = 0;
		    newTCB->waiting_thread = NULL;
		    newTCB->threadBlocked = 0;
		    newTCB->threadsWaiting = 0;
            newTCB->roundRobinScheduler = 0;
            newTCB->lotteryScheduler = 0;
            newTCB->initialTicket = 0;
            newTCB->finalTicket = 0;
            newTCB->warningLevel = 0;
            newTCB->ultimateWarningLevel = 0;
            newTCB->timeLimit = 0;
		    newTCB->isDetached = 0;
            newTCB->threadQuantum = 0;
		    newTCB->threadID = getThreadID();
		    return newTCB;
	    }
    }
}

/* DESCRIPCION:
 * Devuelve un thread buscado por su id en la cola */
TCB searchThread(thread_t pThreadID, TCBQueue pQueue)
{
    TCB headThread = getHeadQueue(pQueue);
    if(headThread == NULL)
    {
    	return NULL;
    }
    else if(headThread->threadID == pThreadID)
    {
        return headThread;
    }
    else
    {
    	TCB iteratorThread = headThread->next_thread;
	    while(headThread != iteratorThread)
	    {
	        if(iteratorThread->threadID == pThreadID)
	        {
	            return iteratorThread;
	        }
	        else
	        {
	        	iteratorThread = iteratorThread->next_thread;
	        }
	    }
	    return NULL;
    }
}
/* DESCRIPCION:
 * Devuelve un thread buscado por su id de ticket en
 * la cola */
TCB searchThreadTicket(int pTicket, TCBQueue pQueue)
{
    TCB headThread = getHeadQueue(pQueue);
    if(headThread == NULL)
    {
        return NULL;
    }
    else if(headThread->initialTicket <= pTicket && headThread->finalTicket >= pTicket)
    {
        return headThread;
    }
    else
    {
        TCB iteratorThread = headThread->next_thread;
        while(headThread != iteratorThread)
        {
            if(iteratorThread->initialTicket <= pTicket && iteratorThread->finalTicket >= pTicket)
            {
                return iteratorThread;
            }
            else
            {
                iteratorThread = iteratorThread->next_thread;
            }
        }
        return NULL;  
    }
}
/* DESCRIPCION:
 * Crea una nueva cola donde se guardaran los nodos
 * que contienen los threads. */
TCBQueue createNodeQueue()
{
    TCBQueue newQueue = (TCBQueue)malloc(sizeof(struct TCBQueues));
    if(newQueue == NULL)
    {
        return NULL;
    }
    else
    {
        newQueue->quantum = 0;
        newQueue->activeQuantums = 0;
    	newQueue->threadQuantity = 0;
        newQueue->roundRobinCount = 0;
        newQueue->lotteryCount = 0;
	    newQueue->head = NULL;
	    newQueue->headParent = NULL;
        newQueue->currentThread = NULL;
        newQueue->currentThreadCopy = NULL;
	    return newQueue;
    }
}

/* DESCRIPCION:
 * Crea una nueva lista donde se guardaran los threds
 * en espera de que otro finalice.*/
WaitingThreadsQueue createWaitingThreadsList()
{
	WaitingThreadsQueue newList = (WaitingThreadsQueue)malloc(sizeof(struct WaitingThreadsQueue));
    if(newList != NULL)
    {
        newList->waitingThreadNode = NULL;
        newList->next_waiting_thread = NULL;
    }
    return newList;
}

/* DESCRIPCION:
 * Devuelve el thread buscado por su id en la cola de
 * threads muertos. */
DeadNode searchDeadThread(DeadNodesQueue pQueue, thread_t pThreadID)
{
	if(pQueue != NULL)
    {
        DeadNode deadThread = pQueue->head;
        while((deadThread != NULL) && (deadThread->threadID != pThreadID))
        {
            deadThread = deadThread->nextDeadNode;
        }
        return deadThread;
    }
    else
    {
    	return NULL;
    }
}

/* DESCRIPCION:
 * Elimina un thread de la lista de terminados mediante
 * su id. */
void deleteDeadNode(DeadNodesQueue pQueue, thread_t pThreadID)
{
	DeadNode deadThread = pQueue->head;
    DeadNode previousDeadTread = NULL;
    while((deadThread != NULL) && (deadThread->threadID != pThreadID))
    {
        previousDeadTread = deadThread;
        deadThread = deadThread->nextDeadNode;
    }
    if(deadThread != NULL)
    {
        if(previousDeadTread == NULL)
        {
            pQueue->head = deadThread->nextDeadNode;
        }
        else
        {
            previousDeadTread->nextDeadNode = deadThread->nextDeadNode;
        }
        free(deadThread);
    }
}

/* DESCRIPCION:
 * Crea un nuevo nodo para un thread que ya haya terminado. */
DeadNode createNewDeadNode()
{
	DeadNode newDeadThread = (DeadNode)malloc(sizeof(struct DeadNodes));
    if(newDeadThread == NULL)
    {
        return NULL;
    }
    else
    {
    	newDeadThread->returnValue = (void**)malloc(sizeof(void*));
	    if(newDeadThread->returnValue == NULL)
	    {
	        free(newDeadThread);
	        return NULL;
	    }
	    else
	    {
	    	newDeadThread->threadID = INVALID_THREAD_T;
		    *(newDeadThread->returnValue) = NULL;
		    newDeadThread->threadsWaiting = 0;
		    newDeadThread->nextDeadNode = NULL;
		    return newDeadThread;
	    }
    }
}

/* DESCRIPCION:
 * Crea una nueva cola donde se guardaran los threads que ya han terminado. */
DeadNodesQueue createDeadNodesQueue()
{
	DeadNodesQueue newQueue = (DeadNodesQueue)malloc(sizeof(struct DeadNodeQueue));
    if(newQueue == NULL)
    {
        return NULL;
    }
    newQueue->count = 0;
    newQueue->head = NULL;
    return newQueue;
}

/* DESCRIPCION:
* Avanza la cabeza al siguiente thread y pone en el padre de la cabeza
 * el thread anterior a la cabeza actual en la cola. */

int moveForward(TCBQueue pQueue)
{
	int result = INVALID_OPERATION;
    if(pQueue != NULL)
    {
        TCB headThread = pQueue->head;
        if(headThread != NULL)
        {
        	pQueue->head = headThread->next_thread;
            pQueue->headParent = headThread;
            result = SUCESS;
        }
    }
    return result;
}

/* DESCRIPCION:
 * Devuelve la cantidad de threads que hay en la cola. */
int getNodeCountQueue(TCBQueue pQueue)
{
	if(pQueue == NULL)
    {
        return 0;
    }
    else
    {
    	return pQueue->threadQuantity;
    }
}

/* DESCRIPCION:
 * Devuelve la cantidad de threads que hay en la cola
 * del tipo scheduler RoundRobin. */
int getNodeRoundRobinCount(TCBQueue pQueue)
{
    if(pQueue == NULL)
    {
        return 0;
    }
    else
    {
        return pQueue->roundRobinCount;
    }
}

/* DESCRIPCION:
 * Devuelve la cantidad de threads que hay en la cola
 * del tipo scheduler Lottery. */
int getNodeLotteryCount(TCBQueue pQueue)
{
    if(pQueue == NULL)
    {
        return 0;
    }
    else
    {
        return pQueue->lotteryCount;
    }
}

/* DESCRIPCION:
 * Libera el espacio asociado a un thread en la cola..*/

void freeThread(TCB pThread)
{
    if(pThread != NULL)
    {
        WaitingThreadsQueue waitingThreadNode = pThread->waiting_thread;
        free(pThread->t_Context.uc_stack.ss_sp);
        while(waitingThreadNode != NULL)
        {
            WaitingThreadsQueue nextWaitingThreadNode = waitingThreadNode->next_waiting_thread;
            free(waitingThreadNode);
            waitingThreadNode = nextWaitingThreadNode;
        }
        free(pThread);
    }
}

/* DESCRIPCION:
 * Elimina el thread cabeza de la cola.*/
int deleteHeadThread(TCBQueue pQueue)
{
	int result = SUCESS;
    TCB previousThread, headThread;
    if(pQueue == NULL)
    {
        result = INVALID_OPERATION;
    }
    else
    {
        headThread = pQueue->head;
        previousThread = pQueue->headParent;
        if(headThread != NULL)
        {
            if(pQueue->threadQuantity == 1)
            {
                pQueue->head = NULL;
                pQueue->headParent = NULL;
            }
            else
            {
                pQueue->head = headThread->next_thread;
                previousThread->next_thread = pQueue->head;
            }
            headThread->roundRobinScheduler == 1 ? pQueue->roundRobinCount-- : pQueue->lotteryCount--;
            freeThread(headThread);
            pQueue->threadQuantity--;
        }
        else
        {
            result = INVALID_OPERATION;
        }
    }
    return result;
}

/*
 * DESCRIPCION:
 * Elimina el thread dado de la cola.
 * */

int deleteLotteryThread(thread_t pThreadID, TCBQueue pQueue)
{
    int result = SUCESS;
    TCB headThread;
    if(pQueue == NULL)
    {
        result = INVALID_OPERATION;
    }
    else
    {
        headThread = pQueue->head;
        if(headThread != NULL)
        {
            if(pQueue->threadQuantity == 1)
            {
                pQueue->head = NULL;
                pQueue->headParent = NULL;
            }
            else if(headThread->threadID == pThreadID)
            {
                pQueue->headParent->next_thread = headThread->next_thread;
                pQueue->head = pQueue->headParent->next_thread;
                if((pQueue->lotteryCount - 1) != 0)
                {
                    restructureTickets(pQueue, headThread);
                }
                free(headThread);
            }
            else
            {
                while(headThread->next_thread->threadID != pThreadID)
                {
                    headThread = headThread->next_thread;
                }
                TCB auxThred = headThread->next_thread;
                headThread->next_thread = auxThred->next_thread;
                if(headThread->next_thread == pQueue->head)
                {
                    pQueue->headParent = headThread;
                }
                if((pQueue->lotteryCount - 1) != 0)
                {
                    restructureTickets(pQueue, auxThred);
                }
                free(auxThred);
            }
            pQueue->threadQuantity--;
            pQueue->lotteryCount--;
        }
        else
        {
            result = INVALID_OPERATION;
        }
    }
    return result;
}

/* DESCRIPCION:
 * Inserta un nuevo thread al final de la cola. */
int insertThread(TCBQueue pQueue, TCB pThread)
{
	int result = SUCESS;
    if(pQueue == NULL || pThread == NULL)
    {
        result = INVALID_OPERATION;
    }
    else
    {
        if(pQueue->head == NULL)
        {
            pThread->next_thread = pThread;
            pQueue->headParent = pThread;
            pQueue->head = pThread;
        }
        else if(pQueue->roundRobinCount == 0)
        {
            pThread->next_thread = pQueue->head;
            pQueue->headParent->next_thread = pThread;
            pQueue->head = pThread;
        }
        else
        {
            pThread->next_thread = pQueue->head;
            pQueue->headParent->next_thread = pThread;
            pQueue->headParent = pThread;
        }
        pQueue->threadQuantity++;
        pThread->roundRobinScheduler == 1 ? pQueue->roundRobinCount++ : pQueue->lotteryCount++;
    }
    return result;
}

/* DESCRIPCION:
 * Inserta un nuevo thread a la lista de threads que se encuentran esperando
 * ya que hicieron join mientras esperan que otro thread termine. */
int insertWaitingThread(TCB pTargetThread, TCB pThreadAdded)
{
	WaitingThreadsQueue newWaitingThread = createWaitingThreadsList();
    if(newWaitingThread != NULL)
    {
    	pTargetThread->threadsWaiting++;
        newWaitingThread->waitingThreadNode = pThreadAdded;
        newWaitingThread->next_waiting_thread = pTargetThread->waiting_thread;
        pTargetThread->waiting_thread = newWaitingThread;
        pThreadAdded->threadBlocked = 1;
        return SUCESS;
    }
    else
    {
    	return INVALID_OPERATION;
    }
}

/* DESCRIPCION:
 * Inserta un nuevo thread terminado al inicio de la
 * cola de threads terminados, ademas lo asigna como
 * cabeza de la cola. */
int insertDeadNode(DeadNodesQueue pQueue, DeadNode pThread)
{
    if(pQueue != NULL && pThread != NULL)
    {
        pThread->nextDeadNode = pQueue->head;
        pQueue->head = pThread;
        return SUCESS;
    }
    else
    {
    	return INVALID_OPERATION;
    }
}

/* DESCRIPCION:
 * Busca el ultimo ticket que fue asignado a un
 * proceso y lo devuelve sumandole uno. */
int searchLastTicket(TCBQueue pQueue)
{
    TCB headThread = getHeadQueue(pQueue);
    if(headThread == NULL)
    {
        return 1;
    }
    else
    {
        int nextTicket = 0;
        TCB next_thread = headThread->next_thread;
        if(headThread->lotteryScheduler && headThread->finalTicket > nextTicket)
        {
            nextTicket = headThread->finalTicket;
        }
        while(next_thread != headThread)
        {
            if(next_thread->lotteryScheduler)
            {
                if(next_thread->finalTicket > nextTicket)
                {
                    nextTicket = next_thread->finalTicket;
                }
                next_thread = next_thread->next_thread;
            }
            else
            {
                next_thread = next_thread->next_thread;
            }
        }
        return ++nextTicket;
    }
}

/* DESCRIPCION:
 * Da mas ticketes al thread dado con la cantidad de tiquetes dados. */

void giveTickets(TCBQueue pQueue, TCB pThread, int pTickets)
{
    TCB headThread = getHeadQueue(pQueue);
    if(headThread != NULL)
    {
        TCB next_thread = headThread->next_thread;
        int previousTicket = pThread->finalTicket;
        pThread->finalTicket = pThread->finalTicket + pTickets;
        if(headThread->lotteryScheduler && headThread != pThread && headThread->initialTicket > previousTicket)
        {
            headThread->initialTicket = headThread->initialTicket + pTickets;
            headThread->finalTicket = headThread->finalTicket + pTickets;
        }
        while(next_thread != headThread)
        {
            if(next_thread->lotteryScheduler && next_thread != pThread)
            {
                if(next_thread->initialTicket > previousTicket)
                {
                    next_thread->initialTicket = next_thread->initialTicket + pTickets;
                    next_thread->finalTicket = next_thread->finalTicket + pTickets;
                }
                next_thread = next_thread->next_thread;
            }
            else
            {
                next_thread = next_thread->next_thread;
            }
        }
    }
}

/* DESCRIPCION:
 * Mueve la cabeza al siguiente thread administrado por un scheduler RoundRobin. */

void moveHeadToNextRoundRobin(TCBQueue pQueue, TCB pThread)
{
    if(getNodeRoundRobinCount(pQueue) > 1 && getHeadQueue(pQueue) == pThread)
    {
        TCB auxThred = pThread->next_thread;
        while(auxThred->lotteryScheduler)
        {
            auxThred = auxThred->next_thread;
        }
        setNewHead(pQueue, auxThred);
    }
}


/* DESCRIPCION:
 * Setea la cabeza al nuevo thread dado. */
void setNewHead(TCBQueue pQueue, TCB pThread)
{
    TCB headThread = getHeadQueue(pQueue);
    while(headThread != pThread)
    {
        moveForward(pQueue);
        headThread = getHeadQueue(pQueue);
    }
}

/*
 * DESCRIPCION:
 * Reestructura por completo los ticketes de los threads administrados por el scheduler de loteria. */
void restructureTickets(TCBQueue pQueue, TCB pErasedThread)
{
    TCB headThread = getHeadQueue(pQueue);
    TCB next_thread = headThread->next_thread;
    int erasedTickets = (pErasedThread->finalTicket - pErasedThread->initialTicket) + 1;
    if(headThread != NULL)
    {
        if(headThread->lotteryScheduler && headThread->initialTicket > pErasedThread->finalTicket)
        {
            headThread->initialTicket = headThread->initialTicket - erasedTickets;
            headThread->finalTicket = headThread->finalTicket - erasedTickets;
        }
        while(next_thread != headThread)
        {
            if(next_thread->lotteryScheduler && next_thread->initialTicket > pErasedThread->finalTicket)
            {
                next_thread->initialTicket = next_thread->initialTicket - erasedTickets;
                next_thread->finalTicket = next_thread->finalTicket - erasedTickets;
                next_thread = next_thread->next_thread;
            }
            else
            {
                next_thread = next_thread->next_thread;
            }
        }
    }
}

