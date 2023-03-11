/* releaseall.c - Release the specified lockes */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

SYSCALL releaseall(nargs,args)
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD 	ps;    
	unsigned long	*a;		/* points to list of args	*/
	int 	ldes;
	int	i, lid, release_failed=0;
	disable(ps);

	a = (unsigned long *)(&args) + (nargs-1); /* last argument	*/
	for( i=nargs ; i > 0 ; i-- ){
		ldes = *a--;
		//kprintf("\nldes = %d\n", ldes);
		lid = get_lid(ldes);
		
		int 	process_doesnt_hold_lock = isbadlock(lid) || (!currpid_hold_lock(lid)); 
		
		if( process_doesnt_hold_lock ){
			release_failed = 1;
			continue;
		}
		
		release_one_lock(lid);
	}
	pinh_recalculate(currpid);

	resched();

	if(release_failed){
		restore(ps);
		return (SYSERR);
	}
	
	restore(ps);
	return (OK);

}

void release_one_lock(int lid){

	struct lentry	*lptr;
	int rpid, rpprio, wpid, wpprio;
	
	lptr = &locktab[lid];

	clear_hold(lid, currpid);

	int other_proc_hold_lock = ( sum_of_lock_holder(lid) != 0 );
	if( other_proc_hold_lock ){
		return;
	}
	
	locktab[lid].lstate = FREE;

	rpid = find_highest_prio(lid, READ, &rpprio);
	wpid = find_highest_prio(lid, WRITE, &wpprio);

	int no_proc_waiting = isbadpid(rpid) && isbadpid(wpid);
	int only_write_proc_waiting = isbadpid(rpid) && isgoodpid(wpid);
	int only_read_proc_waiting  = isgoodpid(rpid) && isbadpid(wpid);

	if( no_proc_waiting ){
		return;
	}

	if( only_write_proc_waiting ){
		proc_acquire_lock(lid, wpid);
		return;
	}

	if( only_read_proc_waiting ){
		// put all read proc in ready queue.
		int pid;
		while( (pid = wgetlast(lid, lptr->wqtail) ) != EMPTY ){
			proc_acquire_lock(lid, pid);
		}
		return;
	}

	// Both are good pid !
	
// Hence, when a writer or the last reader releases a lock, 
// the lock should be next given to a process having the 
// highest lock priority for the lock.
	if( wpprio > rpprio ){
		proc_acquire_lock(lid, wpid);
		return;	
	}
	if( wpprio < rpprio ){
		// put all rpprio that has priority higher than write into ready queue.
		int pid, tail;
		tail = lptr->wqtail;
		for( pid = lptr->wq[tail].qprev ; pid != lptr->wqhead ; pid = lptr->wq[pid].qprev )
		{
			if( lptr->wq[pid].qkey <= wpprio )
				break;
			proc_acquire_lock(lid, pid);
		}
		return;
	}

	// write proc priority == read proc priority !
	
// Condition 1: 
//   In the case of equal lock priorities among readers or writers, 
//   the lock will be first given to the reader or writer that
//   has the longest waiting time (in milliseconds) on the lock.
//
// Condition 2: 
//   If a writer’s lock priority is equal to the highest lock priority 
//   of the waiting reader, and the writer’s waiting time is
//   no more than 0.5 second longer (think of it as a grace period 
//   for writer), the writer should be given preference to 
//   acquire the lock over the waiting reader.
	int	write_wait_time = ctr1000 - proctab[wpid].pwaitlockstartime;
	int	read_wait_time = ctr1000 - proctab[rpid].pwaitlockstartime;
	int	write_wait_less_than_500ms = ( write_wait_time < 500 );
	int	read_wait_longer_than_write = write_wait_time < read_wait_time; 

	/*
	if(write_wait_less_than_500ms ){
		kprintf(" write wait less than 500 ms (%d)ms\n", write_wait_time);
	} else {
		kprintf(" write wait time (%d)ms\n", write_wait_time);
		kprintf(" read wait time (%d)ms\n", read_wait_time);
	}
	*/

	if( write_wait_less_than_500ms || read_wait_longer_than_write ){
		proc_acquire_lock(lid, rpid);
	}else{
		proc_acquire_lock(lid, wpid);
	}

	return;
}

void proc_acquire_lock(int lid, int pid)
{
	proctab[pid].pwaitlock = BADLID;
	wdequeue(lid, pid);
	ready(pid, RESCHNO);
	return;
};

int find_highest_prio(int lid, int type, int *maxprio){
	
	struct lentry	*lptr;
	int 		prev, maxpid, tail;
       
	lptr = &locktab[lid];
	tail	= lptr->wqtail;
	*maxprio = -1;
	maxpid = BADPID;

	// traverse from tail to head (max prio to min prio), 
	// the first R/W is the max prio in waiting queue.
	for( prev = lptr->wq[tail].qprev ; prev != lptr->wqhead ; prev = lptr->wq[prev].qprev ){
		if( lptr->wq[prev].qtype == type )
		{
			*maxprio = lptr->wq[prev].qkey;
			maxpid = prev;
			break;
		}
	}
	
	int cannot_find_write_proc = ( maxprio < 0 );
	if( cannot_find_write_proc )
		return BADPID;

	return maxpid;
};

int sum_of_lock_holder(int lid){
	int pid, sum=0;
	for(pid =0;pid<NPROC;pid++){
		sum += locktab[lid].lholdproc[pid];	
	}
	return sum;
}

void pinh_recalculate(int _pid)
{
	struct	pentry	*pptr;
	struct	lentry	*lptr;
	int		lid, pid, prev;
	int		maxprio, currprio;
       
	maxprio = (pptr = &proctab[_pid])->porgprio;
	currprio = pptr->pprio;

	// iterating through all locks that _pid current holding
	for(lid =0; lid <NLOCKS; lid++){
		if( proctab[_pid].pholdlock[lid] == 1){
		// searching all process that is waiting on the lock
			maxprio = find_max_prio_in_wait_queue(lid, maxprio);
		}
	}
	pptr->pprio = maxprio;
	
	int priority_is_not_lowered = maxprio >= currprio;
	int proc_is_ready = (pptr->pstate == PRREADY);
	int proc_is_waiting = (pptr->pstate == PRWAIT) && (!isbadlock( pptr->pwaitlock ) );
	
	if( priority_is_not_lowered )
		return;
	
	if( proc_is_ready )
	{
		dequeue(_pid);
		insert(_pid, rdyhead, maxprio);
	} 
	else if( proc_is_waiting )
	{
		lptr = &locktab[ pptr->pwaitlock ];
		for( pid=0; pid < NPROC ; pid++ ){
			if( lptr->lholdproc[pid] == 1 ){
				pinh_recalculate( pid );
			}
		}	
	}
	return;
}

int find_max_prio_in_wait_queue(int lid, int maxprio)
{
	int	prev;
	struct	lentry	*lptr;
	
	lptr = &locktab[lid];
	for( prev = lptr->wq[ lptr->wqtail ].qprev ; prev != lptr->wqhead ; prev = lptr->wq[prev].qprev ){
		if( proctab[prev].pprio > maxprio )
			maxprio = proctab[prev].pprio;
	}
	return maxprio;
}
