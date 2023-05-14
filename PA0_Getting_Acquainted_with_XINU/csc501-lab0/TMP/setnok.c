/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
SYSCALL	setnok(int nok, int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(SETNOK);

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		if(scTraEn) syscall_stop(SETNOK, startTime);
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);
	if(scTraEn) syscall_stop(SETNOK, startTime);
	return(OK);
}
