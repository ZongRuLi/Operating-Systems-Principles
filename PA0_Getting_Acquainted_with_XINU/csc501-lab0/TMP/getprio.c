/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
SYSCALL getprio(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(GETPRIO);

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if(scTraEn) syscall_stop(GETPRIO, startTime);
		return(SYSERR);
	}
	restore(ps);
	if(scTraEn) syscall_stop(GETPRIO, startTime);
	return(pptr->pprio);
}
