/* sleep.c - sleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * sleep  --  delay the calling process n seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep(int n)
{
	STATWORD ps;   
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(SLEEP);

	if (n<0 || clkruns==0){
		if(scTraEn) syscall_stop(SLEEP, startTime);
		return(SYSERR);
	}
	if (n == 0) {
	        disable(ps);
		resched();
		restore(ps);
		if(scTraEn) syscall_stop(SLEEP, startTime);
		return(OK);
	}
	while (n >= 1000) {
		sleep10(10000);
		n -= 1000;
	}
	if (n > 0)
		sleep10(10*n);

	if(scTraEn) syscall_stop(SLEEP, startTime);
	return(OK);
}
