/* linit.c - initialize locks */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

void linit(){
	
	STATWORD 	ps;
	int	lid, pid;
	struct	lentry	*lptr;
	
	disable(ps);

	for( lid=0 ; lid < NLOCKS ; lid++ ){
		(lptr = &locktab[lid])->lstate = UINIT;
		lptr->ltoken = 0;
		
		lptr->lprio = 0;
		lptr->wqhead=NPROC;
		lptr->wqtail=NPROC+1;
		lptr->wq[ lptr->wqhead ].qnext = lptr->wqtail;
		lptr->wq[ lptr->wqhead ].qprev = EMPTY;
		lptr->wq[ lptr->wqhead ].qkey  = MININT;
		lptr->wq[ lptr->wqtail ].qnext = EMPTY;
		lptr->wq[ lptr->wqtail ].qprev = lptr->wqhead;
		lptr->wq[ lptr->wqtail ].qkey  = MAXINT;
		for(pid=0; pid < NPROC ; pid++)
			clear_hold(lid,pid);
	}
	nextlock = 0;
	locktoken = 0;
	
	restore(ps);
	return;
}
