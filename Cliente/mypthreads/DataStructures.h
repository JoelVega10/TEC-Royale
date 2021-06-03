#ifndef _MYTHREADDATASTRUCTURE_H
#define _MYTHREADDATASTRUCTURE_H

#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define STACKSIZE 1024*8

/*
 *  ESTRUCTURAS DE DATOS
 * */

typedef long thread_t;

typedef struct Nodes
{
    thread_t threadID;
    ucontext_t t_Context;
    struct Nodes *next_thread;
    struct WaitingThreadsQueue *waiting_thread;
    long threadQuantum;
    int threadCompleted;
    int threadBlocked;
    int threadsWaiting;
    int roundRobinScheduler;
    int lotteryScheduler;
    int initialTicket;
    int finalTicket;
    int warningLevel;
    int ultimateWarningLevel;
    int timeLimit;
    int isDetached;
}*TCB;

typedef struct WaitingThreadsQueue
{
    TCB waitingThreadNode;
    struct WaitingThreadsQueue *next_waiting_thread;
}*WaitingThreadsQueue;

typedef struct TCBQueues
{
    TCB head;
    TCB headParent;
    TCB currentThread;
    TCB currentThreadCopy;
    long roundRobinCount;
    long lotteryCount;
    long threadQuantity;
    long quantum;
    long activeQuantums;
}*TCBQueue;

typedef struct DeadNodes
{
    thread_t threadID;
    void **returnValue;
    int threadsWaiting;
    struct DeadNodes *nextDeadNode;
}*DeadNode;

typedef struct DeadNodeQueue
{
    DeadNode head;
    long count;
}*DeadNodesQueue;

/*
 *
 * FUNCIONES DE MANEJO DE ESTRUCTURAS DE DATOS
 *
 * */


TCB getHeadQueue(TCBQueue pQueue);
TCB getNextHead(TCBQueue pQueue);
TCB createNewNode();
TCB searchThread(thread_t pThreadID,TCBQueue pQueue);
TCB searchThreadTicket(int pTicket, TCBQueue pQueue);
TCBQueue createNodeQueue();
WaitingThreadsQueue createWaitingThreadsList();
DeadNode searchDeadThread(DeadNodesQueue pQueue, thread_t pThreadID);
DeadNode createNewDeadNode();
DeadNodesQueue createDeadNodesQueue();
int moveForward(TCBQueue pQueue);
int getNodeCountQueue(TCBQueue pQueue);
int getNodeRoundRobinCount(TCBQueue pQueue);
int getNodeLotteryCount(TCBQueue pQueue);
int deleteHeadThread(TCBQueue pQueue);
int deleteLotteryThread(thread_t pThreadID, TCBQueue pQueue);
int insertThread(TCBQueue pQueue, TCB pThread);
int insertWaitingThread(TCB pTargetThread,TCB pThreadAdded);
int insertDeadNode(DeadNodesQueue pQueue, DeadNode pThread);
int searchLastTicket(TCBQueue pQueue);
void deleteDeadNode(DeadNodesQueue pQueue, thread_t pThreadID);
void freeThread(TCB pThread);
void giveTickets(TCBQueue pQueue, TCB pThread, int pTickets);
void setNewHead(TCBQueue pQueue, TCB pThread);
void restructureTickets(TCBQueue pQueue, TCB pErasedThread);
void moveHeadToNextRoundRobin(TCBQueue pQueue, TCB pThread);


#endif