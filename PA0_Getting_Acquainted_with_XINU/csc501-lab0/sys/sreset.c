/* sreset.c - sreset */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  sreset  --  reset the count and queue of a semaphore
 *------------------------------------------------------------------------
 */
SYSCALL sreset(int sem, int count)
{
	STATWORD ps;    
	struct	sentry	*sptr;
	int	pid;
	int	slist;
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(SRESET);

	disable(ps);
	if (isbadsem(sem) || count<0 || semaph[sem].sstate==SFREE) {
		restore(ps);
		if(scTraEn) syscall_stop(SRESET, startTime);
		return(SYSERR);
	}
	sptr = &semaph[sem];
	slist = sptr->sqhead;
	while ((pid=getfirst(slist)) != EMPTY)
		ready(pid,RESCHNO);
	sptr->semcnt = count;
	resched();
	restore(ps);
	if(scTraEn) syscall_stop(SRESET, startTime);
	return(OK);
}
