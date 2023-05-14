/* sleep1000.c - sleep1000 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * sleep1000 --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
SYSCALL sleep1000(int n)
{
	STATWORD ps;    
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(SLEEP1000);

	if (n < 0  || clkruns==0){
		if(scTraEn) syscall_stop(SLEEP1000, startTime);
	        return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep1000(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
	if(scTraEn) syscall_stop(SLEEP1000, startTime);
	return(OK);
}
