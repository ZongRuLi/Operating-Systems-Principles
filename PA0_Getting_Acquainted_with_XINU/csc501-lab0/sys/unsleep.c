/* unsleep.c - unsleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * unsleep  --  remove  process from the sleep queue prematurely
 *------------------------------------------------------------------------
 */
SYSCALL	unsleep(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	struct	qent	*qptr;
	int	remain;
	int	next;
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(UNSLEEP);

        disable(ps);
	if (isbadpid(pid) ||
	    ( (pptr = &proctab[pid])->pstate != PRSLEEP &&
	     pptr->pstate != PRTRECV) ) {
		restore(ps);
		if(scTraEn) syscall_stop(UNSLEEP, startTime);
		return(SYSERR);
	}
	qptr = &q[pid];
	remain = qptr->qkey;
	if ( (next=qptr->qnext) < NPROC)
		q[next].qkey += remain;
	dequeue(pid);
	if ( (next=q[clockq].qnext) < NPROC)
		sltop = (int *) & q[next].qkey;
	else
		slnempty = FALSE;
        restore(ps);
	if(scTraEn) syscall_stop(UNSLEEP, startTime);
	return(OK);
}
