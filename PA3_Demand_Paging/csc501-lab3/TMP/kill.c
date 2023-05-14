/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>
#include <fq.h>
#include <Debug.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int		dev;
	register int _pid = pid;

	disable(ps);
	//TBD: comment out this print because I don't know why compiler failed at this.
	//lDebug(DBG_FLOW,"[INFO][kill] start kill pid(%d). ",_pid);
	
	if (isbadpid(_pid) || (pptr= &proctab[_pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	/* PA3 */
	/* 1. All frames which currently hold any of its pages should be written to the backing store and be freed */
	//lDebug(DBG_INFO,"[INFO][kill] start delete page frame. pid=(%d)", _pid);
	kill_proc_page(_pid);

	//lDebug(DBG_INFO,"[INFO][kill] start delete page table frame. pid (%d)",_pid);
	kill_proc_page_table(_pid);
	
	//lDebug(DBG_INFO,"[INFO][kill] start unmap. pid (%d)",_pid);
	/* 2. All of its mappings should be removed from the backing store map. */
	kill_proc_bsm(_pid);
	
	//lDebug(DBG_INFO,"[INFO][kill] unmap all bsm. pid (%d)",_pid);
	/* 3. The backing stores for its heap (and stack if have chosen to implement a private stack) should be released (remember
	 * backing stores allocated to a process should persist unless the process explicitly releases them).*/
	release_vheap(_pid);
	
	//lDebug(DBG_FLOW,"[INFO][kill] pid (%d). ",_pid);
	/*******/
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	//lDebug(DBG_INFO,"[INFO][kill] start send page. ");
	send(pptr->pnxtkin, _pid);

	//lDebug(DBG_INFO,"[INFO][kill] start freestk. ");
	freestk(pptr->pbase, pptr->pstklen);

	/* 4. The frame used for the page directory should be released. */
	kill_proc_page_directroy(_pid);

	//lDebug(DBG_FLOW,"[INFO][kill] (%d) done. ",_pid);
	/*******/
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(_pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(_pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
