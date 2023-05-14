/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
extern	struct	sentry	semaph[];

       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(SCOUNT);

	if (isbadsem(sem) || semaph[sem].sstate==SFREE){
		if(scTraEn) syscall_stop(SCOUNT, startTime);
		return(SYSERR);
	}
	if(scTraEn) syscall_stop(SCOUNT, startTime);
	return(semaph[sem].semcnt);
}
