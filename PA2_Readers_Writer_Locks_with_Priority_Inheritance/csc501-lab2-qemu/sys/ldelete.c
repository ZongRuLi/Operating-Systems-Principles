/* ldelete - delete the specified lock by lock descripter */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

SYSCALL ldelete(int ldes)
{
	STATWORD 	ps;
	int 		pid,lid,token;
	struct	lentry	*lptr;
	
	disable(ps);

	lid = get_lid_and_token(ldes, &token);
	lptr = &locktab[lid];
	
	int token_expired = (token != lptr->ltoken);
	int access_deleted_lock = ( token_expired || isdeletedlock(lid) );
	
	if ( isbadlock(lid) || isuninitlock(lid) || access_deleted_lock ){
		restore(ps);
		return (SYSERR);
	}
	
	if( isfreelock(lid) ){
		lptr->lstate = DELETED;	
		restore(ps);
		return(OK);
	}

	lptr->lstate = DELETED;	

	// 1. process in waiting queue: put them into ready queue and return DELETED 
	// 	How to deal with other process waiting on deleted lock?
	// 	=> let them return DELETE instead of OK
	if( wnonempty(lid) ){
		while( (pid = wgetfirst(lid, lptr->wqhead) ) != EMPTY )
		{
			proctab[pid].pwaitlock = -1;
			ready(pid, RESCHNO);
		}
		// 2. process that also hold this lock : lower priority due to inheritant
		for( pid =0; pid < NPROC; pid++ )
		{
			if( locktab[lid].lholdproc[pid] == 1 ){
				pinh_recalculate( pid );	
			}
		}
		resched();
	}
	// 3. no matter there are process waiting or not, no process hold this lock now.
	for( pid =0; pid < NPROC; pid++ )
	{
		clear_hold(lid, pid);
	}

	restore(ps);
	return(OK);

}
