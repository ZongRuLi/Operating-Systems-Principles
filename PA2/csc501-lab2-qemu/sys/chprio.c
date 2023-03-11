/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	int	pid2;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	//pptr->pprio = newprio;
	// TBD: priority inheritance
	int	oldprio = pptr->pprio;

	pptr->porgprio = newprio;

	pinh_recalculate( pid );// update pptr->pprio
	
	int wait_on_lock = (pptr->pstate == PRWAIT) && !(isbadlock( pptr->pwaitlock ) );
	if( wait_on_lock ){
		pinh_raise( pptr->pwaitlock , pptr->pprio  );	
	}

	// TBD: should chprio affect ready queue?
	if( (pptr->pstate == PRREADY) && (oldprio != pptr->pprio) ){
		dequeue(pid);
		insert(pid, rdyhead, newprio);
		resched();
	}

	restore(ps);
	return(newprio);
}
