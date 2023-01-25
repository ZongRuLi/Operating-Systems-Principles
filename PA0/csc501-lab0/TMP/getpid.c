/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{
       	unsigned int startTime = ctr1000;
	if(scTraEn) syscall_start(GETPID);
	if(scTraEn) syscall_stop(GETPID, startTime);
	return(currpid);
}
