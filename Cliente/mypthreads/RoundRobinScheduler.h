#ifndef _ROUNDROBINSCHEDULER_H
#define _ROUNDROBINSCHEDULER_H

#include <stdio.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* Scheduler de tipo round robin */
void roundRobin();

#endif