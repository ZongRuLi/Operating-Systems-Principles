/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	int		bs_id;
	int		pid;
	struct 	mblock 	*virt_heap;
	struct	pentry	*pptr;

	disable(ps);

	if( isbad_npage(hsize) )
	{
		restore(ps);
		return SYSERR;
	}

	if( get_bsm(&bs_id) == SYSERR )
	{
		restore(ps);
		return SYSERR;
	}
	
	if( (pid = create(procaddr, ssize, priority, name, nargs, args)) == SYSERR )
	{
		restore(ps);
		return SYSERR;
	}
	
	if( bsm_map(pid, 4096, bs_id, hsize) == SYSERR )
	{
		restore(ps);
		return SYSERR;
	}
	
	bsm_tab[bs_id].bs_private = BSM_PRIVATE;
	
	virt_heap = BACKING_STORE_BASE + (bs_id * BACKING_STORE_UNIT_SIZE);
	virt_heap->mlen 	= hsize * NBPG;
	virt_heap->mnext 	= NULL;

	pptr = &proctab[pid];
	pptr->vhpnpages 		= hsize;
	pptr->store 			= bs_id;
	pptr->vmemlist->mlen 	= hsize * NBPG;
	pptr->vmemlist->mnext 	= 4096 * NBPG;

	restore(ps);
	return OK;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
