#include "RealTimeScheduler.h"
#include "MyMutexDataStructure.h"
#include "MyThreadErrors.h"
#include "MyThread.h"


TCBQueue threadsQueue = NULL;
struct itimerval timeQuantum;
long timeInterval;
sigset_t sigProcMask;
DeadNodesQueue deadThreadsQueue = NULL;
struct sigaction schedulerHandle;
ucontext_t exitContext;
time_t randomTimeSeed;


extern MutexQueue mutexQueue;
extern int roundRobinControl;
extern int sortControl;

static void *wrapperFunction(void *(*pStartRoutine)(void *), void *pArgument);
static void clearBlockedThreads(TCB pTargetThread);
static void setExitContext();
static void executeExitContext();
static void setSchedulerType(TCB pThread, char *pSchedulerType);


/* DESCRIPCION:
 * Se encarga de inicializar la biblioteca para
 * poder utilizar las demas funciones de esta. */
void my_thread_init(long pTimeInterval)
{

    if (threadsQueue == NULL && deadThreadsQueue == NULL) 
    {
        sigemptyset(&sigProcMask);
        sigaddset(&sigProcMask, SIGPROF);
        deadThreadsQueue = createDeadNodesQueue();
        threadsQueue = createNodeQueue();
        if (deadThreadsQueue == NULL || threadsQueue == NULL) 
        {
            return;
        }
        else
        {
            srand((unsigned) time(&randomTimeSeed));
            timeInterval = pTimeInterval * 1000;
            threadsQueue->quantum = pTimeInterval;
            TCB TCBMain = createNewNode();
            getcontext(&(TCBMain->t_Context));
            setExitContext();
            TCBMain->t_Context.uc_link = &exitContext;
            //La linea de abajo indica que el thread principal es administrado por el scheduler RoundRobin.
             //TCBMain->roundRobinScheduler = 1;
            // roundRobinControl = 1;
            // Descomentar las lineas comentadas de abajo y comentar la linea de arriba si se quiere que el thread principal sea administrado por el scheduler Sort.
            TCBMain->lotteryScheduler = 1;
            int nextTicket = searchLastTicket(threadsQueue);
            TCBMain->initialTicket = nextTicket;
            TCBMain->finalTicket = nextTicket;
            sortControl = 1;
            //
            threadsQueue->currentThread = TCBMain;
            insertThread(threadsQueue, TCBMain);
            memset(&schedulerHandle, 0, sizeof (schedulerHandle));
            schedulerHandle.sa_handler = &realTime;
            sigaction(SIGPROF, &schedulerHandle, NULL);
            //printf("\nMyThread: Biblioteca MyThread Inicializada...\n");
            timeQuantum.it_value.tv_sec = 0;
            timeQuantum.it_value.tv_usec = timeInterval;
            timeQuantum.it_interval.tv_sec = 0;
            timeQuantum.it_interval.tv_usec = timeInterval;
            setitimer(ITIMER_PROF, &timeQuantum, NULL);
        }
    }
}

/* DESCRIPCION:
 * Inicia un nuevo hilo en la llamada proceso, ademas se le indica
 * el scheduler que va a utilizarel hilo. */
int my_thread_create(thread_t *pThread, void *(*pStartRoutine)(void *), void *pArgument, int pLimitTime, char *pSchedulerType)
{
    if (threadsQueue != NULL) 
    {
        sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
        TCB newTCB = createNewNode();
        getcontext(&(newTCB->t_Context));
        if (newTCB == NULL) 
        {
            freeThread(newTCB);
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return NOT_ENOUGH_MEMORY;
        }
        else
        {
            if(pLimitTime > 0)
            {
                newTCB->timeLimit = pLimitTime;
            }
        	newTCB->t_Context.uc_link = &exitContext;
            newTCB->threadQuantum = threadsQueue->activeQuantums;
        	setSchedulerType(newTCB, pSchedulerType);
	        makecontext(&(newTCB->t_Context), wrapperFunction, 2, pStartRoutine, pArgument);
	        *pThread = newTCB->threadID;
	        //printf("MyThread: Nuevo thread creado: %ld\n", *pThread);
	        insertThread(threadsQueue, newTCB);
	        sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	        return SUCESS;
        }
    }
    else
    {
    	return MY_THREAD_NOT_INITIALIZED;
    }
}

/* DESCRIPCION:
 * Espera a que termine el hilo ingresado*/
int my_thread_join(thread_t pThread, void **pStatus)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    TCB currentThread = threadsQueue->currentThread;
    TCB targetThread = searchThread(pThread, threadsQueue);
    if (currentThread == targetThread || currentThread == NULL || (targetThread != NULL && targetThread->isDetached))
    {
        sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
        return INVALID_OPERATION;
    }
    else
    {
        if (targetThread == NULL || targetThread->threadCompleted) 
        {
            DeadNode deadNode = searchDeadThread(deadThreadsQueue, pThread);
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            if (deadNode != NULL)
            {
                if (pStatus != NULL) 
                {
                    *pStatus = *(deadNode->returnValue);
                }
                deleteDeadNode(deadThreadsQueue, pThread);
                return SUCESS;
            } 
            else 
            {
                return INVALID_OPERATION;
            }
        }
        else
        {
            insertWaitingThread(targetThread, currentThread);
            int isBlocked = currentThread->threadBlocked;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            while (isBlocked) 
            {
                isBlocked = currentThread->threadBlocked;
            }
            sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
            DeadNode deadNode = searchDeadThread(deadThreadsQueue, pThread);
            if(deadNode != NULL)
            {
                if (pStatus != NULL) 
                {   
                    *pStatus = *(deadNode->returnValue);
                }
                if(((deadNode->threadsWaiting) - 1) == 0)
                {   
                    deleteDeadNode(deadThreadsQueue, pThread);
                }
                else
                {
                    deadNode->threadsWaiting--;
                }
                sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
                return SUCESS;
            }
            else
            {
                //printf("MyThread: Un thread anterior a este ha realizado el join primero, intente realizando el join para ambos threads antes que el thread al cual desea hacer el join haya finalizado\n");
                sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
                return SUCESS;
            }
        }
    }
}

/* DESCRIPCION:
 * Se encarga de hacer que el hilo de llamada ceda la CPU.*/
void my_thread_yield(void)
{
    raise(SIGPROF);
}

/* DESCRIPCION:
 * Cancela el hilo enviado por parámetro y libera los que estén esperando */

int my_thread_end(thread_t pThread)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    TCB currentThread = threadsQueue->currentThread;
    if ((currentThread != NULL) && (currentThread->threadID != pThread)) 
    {
        TCB targetThread = searchThread(pThread, threadsQueue);
        if (targetThread != NULL) 
        {
            clearBlockedThreads(targetThread);
    		//printf("MyThread: Thread %ld cancelado\n", targetThread->threadID);
    		targetThread->threadCompleted = 1;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return SUCESS;
        }
        else
        {
        	sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
        	return INVALID_OPERATION;
        }
    }
    else
    {
    	sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    	return INVALID_OPERATION;
    }
}


//no se usa
void myThreadExit(void *pReturnValue) 
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    TCB currentTCB = threadsQueue->currentThread;
    DeadNode deadNode = createNewDeadNode();
    if (deadNode != NULL && currentTCB != NULL)
    {
        *(deadNode->returnValue) = pReturnValue;
        deadNode->threadID = currentTCB->threadID;
        deadNode->threadsWaiting = currentTCB->threadsWaiting;
        insertDeadNode(deadThreadsQueue, deadNode);
    }
    executeExitContext();
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    raise(SIGPROF);
}

/* DESCRIPCION:
 * Se encarga de que cuando termina un hilo, su recursos se
 * devuelven automaticamente al sistema sin la necesidad de
 * que otro hilo se una al hilo terminado. */
int my_thread_detach(thread_t pThread)
{
	TCB targetThread = searchThread(pThread, threadsQueue);
	if(targetThread != NULL)
	{
		targetThread->isDetached = 1;
		return SUCESS;
	}
	else
	{
		return INVALID_OPERATION;
	}
}

/* DESCRIPCION:
 * Se encarga de cambiar el tipo scheduler del hilo.*/
void my_thread_chsched(TCB pThread)
{
    if(pThread->threadCompleted == 0)
    {
        if(pThread->lotteryScheduler)
        {
            sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
            pThread->lotteryScheduler = 0;
            pThread->roundRobinScheduler = 1;
            restructureTickets(threadsQueue, pThread);
            pThread->initialTicket = 0;
            pThread->finalTicket = 0;
            if(threadsQueue->roundRobinCount == 0)
            {
                setNewHead(threadsQueue, pThread);
            }
            threadsQueue->roundRobinCount++;
            threadsQueue->lotteryCount--;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
        }
        else
        {
            sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
            pThread->roundRobinScheduler = 0;
            setSchedulerType(pThread, "Sort");
            moveHeadToNextRoundRobin(threadsQueue, pThread);
            threadsQueue->roundRobinCount--;
            threadsQueue->lotteryCount++;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
        }
    }
    else
    {
        //printf("MyThread: No puede cambiar de scheduler un thread que ya ha sido completado.\n");
    }
}


static void setSchedulerType(TCB pThread, char *pSchedulerType)
{
	if(!strcmp("Sort", pSchedulerType))
	{
		pThread->lotteryScheduler = 1;
        int nextTicket = searchLastTicket(threadsQueue);
        pThread->initialTicket = nextTicket;
        pThread->finalTicket = nextTicket;
	}
	else
	{
		pThread->roundRobinScheduler = 1;
	}
}
/* DESCRIPCION:
 * Se encarga de ejecutar la rutina que se ingresa
 * al crear el hilo. */
static void *wrapperFunction(void *(*pStartRoutine)(void *), void *pArgument) 
{
    void *returnValueFunction;
    TCB currentTCB = threadsQueue->currentThread;
    returnValueFunction = (*pStartRoutine)(pArgument);
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    if(!currentTCB->isDetached)
    {
    	DeadNode deadNode = createNewDeadNode();
	    if (deadNode != NULL)
	    {
	        *(deadNode->returnValue) = returnValueFunction;
	        deadNode->threadID = currentTCB->threadID;
	        deadNode->threadsWaiting = currentTCB->threadsWaiting;
	        insertDeadNode(deadThreadsQueue, deadNode);
	    }
    }
    //
    //printDeadQueue(deadThreadsQueue);
    //
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    return returnValueFunction;
}

static void clearBlockedThreads(TCB pTargetThread) 
{
    WaitingThreadsQueue blockedThread = pTargetThread->waiting_thread;
    while(blockedThread != NULL)
    {
        blockedThread->waitingThreadNode->threadBlocked = 0;
        blockedThread = blockedThread->next_waiting_thread;
    }
}

static void executeExitContext()
{
    TCB currentThread = threadsQueue->currentThread;
    clearBlockedThreads(currentThread);
    //printf("MyThread: Thread %ld completado\n", currentThread->threadID);
    currentThread->threadCompleted = 1;
    raise(SIGPROF);
}

static void setExitContext()
{
    static int exitContextCreated;
    if(!exitContextCreated)
    {
        getcontext(&exitContext);
        exitContext.uc_link = 0;
        exitContext.uc_stack.ss_sp = malloc(STACKSIZE);
        exitContext.uc_stack.ss_size = STACKSIZE;
        exitContext.uc_stack.ss_flags= 0;
        makecontext(&exitContext, (void (*) (void))&executeExitContext, 0);
        exitContextCreated = 1;
    }
}

/* DESCRIPCION:
 * Se encarga de inicializar un mutex. */
int my_mutex_init(thread_mutex_t *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    if(pMutex != NULL)
    {
        if(!isMutexInQueue(pMutex))
        {
            MutexQueue newMutexQueue = createNewMutexQueue();
            newMutexQueue->mutex = createNewMutexNode();
            pMutex->lockNumber = newMutexQueue->mutex->lockNumber;
            insertMutexQueue(newMutexQueue);
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return SUCESS;
        }
    }
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    return INVALID_OPERATION;
}

/* DESCRIPCION:
 * Intenta adquirir el mutex lock de un recurso, si no está dispinonible se bloquea
 * hasta que este sea liberado y llegue su turno*/
int my_mutex_lock(thread_mutex_t *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    MutexQueue auxQueue = searchMutexQueue(pMutex);
    TCB currentTCB = threadsQueue->currentThread;
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    if(auxQueue == NULL)
    {        
        return INVALID_OPERATION;
    }
    else
    {
    	if(currentTCB != NULL)
	    {
            while(auxQueue->threadNodeList);
            sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
            WaitingThreadsQueue waitingTCBNode = createWaitingThreadsList();
            waitingTCBNode->waitingThreadNode = currentTCB;
            waitingTCBNode->next_waiting_thread = auxQueue->threadNodeList;
            auxQueue->threadNodeList = waitingTCBNode;
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return SUCESS;
	    }
	    else
	    {
	    	sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	    	return INVALID_OPERATION;
	    }
    }
}

/* DESCRIPCION:
 * Intenta adquirir el mutex lock de un recurso, si no está dispinonible continua
 * la ejecución normal sin bloquearse*/

int my_mutex_trylock(thread_mutex_t *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    MutexQueue auxQueue = searchMutexQueue(pMutex);
    TCB currentTCB = threadsQueue->currentThread;
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    if(auxQueue == NULL)
    {
        return INVALID_OPERATION;
    }
    else
    {
        if(currentTCB != NULL)
        {
            if(auxQueue->threadNodeList == NULL){
                sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
                WaitingThreadsQueue waitingTCBNode = createWaitingThreadsList();
                waitingTCBNode->waitingThreadNode = currentTCB;
                waitingTCBNode->next_waiting_thread = auxQueue->threadNodeList;
                auxQueue->threadNodeList = waitingTCBNode;
                sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
                return SUCESS;
            }else{
                //ThreadNodeList head is not NULL therefore Mutex has been acquired by another thread
                return TRYLOCK_FAILED;
            }
        }
        else
        {
            sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
            return INVALID_OPERATION;
        }
    }
}


/* DESCRIPCION:
 * Quita el mutex lock de un recurso*/

int my_mutex_unlock(thread_mutex_t *pMutex)
{
    sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
    MutexQueue auxQueue = searchMutexQueue(pMutex);
    sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
    if(auxQueue == NULL)
    {   
        return INVALID_OPERATION;
    }
    else if(auxQueue->threadNodeList == NULL)
    {
        return INVALID_OPERATION;
    }
    else
    {
    	sigprocmask(SIG_BLOCK, &sigProcMask, NULL);
        TCB currentTCB = threadsQueue->currentThread;
        WaitingThreadsQueue waitingTCBNode = auxQueue->threadNodeList;
        if(waitingTCBNode->waitingThreadNode == currentTCB)
        {
            auxQueue->threadNodeList = NULL;
            free(waitingTCBNode);
        }
        sigprocmask(SIG_UNBLOCK, &sigProcMask, NULL);
	    return SUCESS;
    }
}