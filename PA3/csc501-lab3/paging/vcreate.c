/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>
#include <Debug.h>

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
	struct	pentry	*pptr;
	struct	mblock	*vheap_in_bs; /* virtual heap in backing store */

	disable(ps);

	if( isbad_npage(hsize) )
	{
		lDebug(DBG_ERR,"[ERROR][vcreate] invalid hsize (%d)", hsize);
		restore(ps);
		return SYSERR;
	}

	if( get_bsm(&bs_id) == SYSERR )
	{
		lDebug(DBG_ERR,"[ERROR][vcreate] no avail bs_id");
		restore(ps);
		return SYSERR;
	}
	
	if( (pid = create(procaddr, ssize, priority, name, nargs, args)) == SYSERR )
	{
		lDebug(DBG_ERR,"[ERROR][vcreate] create fail\n");
		restore(ps);
		return SYSERR;
	}
	
	// update status to map & max_npage to hsize
	bsm_tab[bs_id].bs_status = BSM_MAPPED;
	bsm_tab[bs_id].bs_npages_max = hsize;

	if( bsm_map(pid, 4096, bs_id, hsize) == SYSERR )
	{
		lDebug(DBG_ERR,"[ERROR][vcreate] bsm_map fail\n");
		restore(ps);
		return SYSERR;
	}
	
	bsm_tab[bs_id].bs_private = BSM_PRIVATE;
	
	/* for process scheduling*/
	pptr->ppolicy = 0;                /* process scheduling policy    */
	pptr->ppi = 0;                    /* priority value in psp        */
	pptr->prate = 0;                  /* rate value in psp            */
	
	/* for demand paging */
	pptr = &proctab[pid];
	pptr->store 			= bs_id; // Warning: proctab[].store only use for vheap!!!
	pptr->vhpno 			= 4096; 
	pptr->vhpnpages 		= hsize;
	pptr->vmemlist.mlen 	= 0;
	pptr->vmemlist.mnext 	= (struct mblock *) ( 4096 * NBPG );
	vheap_in_bs 			= BACKING_STORE_BASE + (bs_id * BACKING_STORE_UNIT_SIZE);
	vheap_in_bs->mlen 		= hsize * NBPG;
	vheap_in_bs->mnext 		= (struct mblock *) NULL;
	
	restore(ps);
	return pid;
}

void release_vheap(int _pid)
{
	struct pentry *pptr = &proctab[_pid];

	if( !isbad_bsid(pptr->store) )
	{
		free_bsm(pptr->store); //don't know why call free_bsm will get memory fail
	}
	pptr->store = -1; // Warning: proctab[].store only use for vheap!!!
	pptr->vhpno 			= 0; 
	pptr->vhpnpages 		= 0;
	pptr->vmemlist.mlen 	= 0;
	pptr->vmemlist.mnext 	= (struct mblock *) NULL;	

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
