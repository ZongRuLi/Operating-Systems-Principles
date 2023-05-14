/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(CHPRIO);

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if(scTraEn) syscall_stop(CHPRIO, startTime);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
	if(scTraEn) syscall_stop(CHPRIO, startTime);
	return(newprio);
}
