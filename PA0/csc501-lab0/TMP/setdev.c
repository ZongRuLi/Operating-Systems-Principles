/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
SYSCALL	setdev(int pid, int dev1, int dev2)
{
	short	*nxtdev;
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(SETDEV);

	if (isbadpid(pid)){
		if(scTraEn) syscall_stop(SETDEV, startTime);
		return(SYSERR);
	}
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;
	if(scTraEn) syscall_stop(SETDEV, startTime);
	return(OK);
}
