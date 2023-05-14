/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);

	// release all locks
	int lid;
	for(lid=0 ; lid <NLOCKS ; lid++){
		if( pptr->pholdlock[lid] == 1 ){
			release_one_lock(lid);
		}
	}

	// priority inheritance lower
	int 	pid2, wait_on_lock;
	int 	pwaitlock = pptr->pwaitlock;
	pptr->porgprio = 0;
	pptr->pwaitlockstartime=0;

	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	wait_on_lock = !isbadlock( pptr->pwaitlock );
			if( wait_on_lock ){ 
				pptr->pwaitlock = BADLID;
				wdequeue(pwaitlock, pid);
				for( pid2 =0; pid2 < NPROC; pid2++ ){
					if( locktab[pwaitlock].lholdproc[pid2] == 1 ){
						pinh_recalculate( pid2 );	
					}
				}
				pptr->pstate = PRFREE;
				break;
			}
			// wait on semaphore
			semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}

	restore(ps);
	return(OK);
}
