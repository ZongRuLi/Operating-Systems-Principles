/* lock.c - Acquisition of a lock for read/write */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <sleep.h>
#include <date.h>
#include <lock.h>

SYSCALL lock (int ldes, int type, int priority){
	
	STATWORD 	ps;    
	int		lid, token, token_expired, access_deleted_lock;
	struct	lentry	*lptr;
	struct	pentry	*pptr;
	
	disable(ps);
	
	lid = get_lid_and_token(ldes, &token);
	lptr = &(locktab[lid]);
	
	//  As before, any calls to lock() 
	//  after the lock is deleted should return SYSERR.
	
	token_expired = (token != lptr->ltoken);
	access_deleted_lock = ( token_expired || isdeletedlock(lid) );

	if( isbadlock(lid) || isuninitlock(lid) || access_deleted_lock ){ 
		restore(ps);
		return SYSERR;
	}

	if( isfreelock(lid) ){
		// acquire lock successfully
		// TBD: more queue
		lptr->lstate = type;
		set_hold(lid, currpid);
		restore(ps);
		return OK;	
	}
	int maxprio;
	find_highest_prio(lid, WRITE, &maxprio);
	int read_priority_larger_than_max_write_priority = ((type == READ) && (priority > maxprio));

	if( isreadlock(lid) && read_priority_larger_than_max_write_priority ){
		// acquire lock successfully
		// TBD: more queue
		set_hold(lid, currpid);
		restore(ps);
		return OK;
	}

	// fail to acquire lock
	(pptr = &proctab[currpid])->pstate = PRWAIT;
	pptr->pwaitlock = lid;
	pptr->pwaitlockstartime = ctr1000; // millisecond since boot
	pptr->pwaitret = OK;
	winsert(lid, currpid, lptr->wqhead, type, priority);
	// priority inheritance protocol
	pinh_raise(lid, getprio(currpid) );	
	
	resched();
	
//---------------------------------------------------------
//  Process wakeup, resume status, and continue execution.
//---------------------------------------------------------
	
	//  You must implement your lock system such that waiting on a lock
	//  will return a new constant DELETED instead of OK when returning 
	//  due to a deleted lock. This will indicate to the user that the 
	//  lock was deleted and not unlocked. 
	
	token_expired = (token != lptr->ltoken);
	access_deleted_lock = ( token_expired || isdeletedlock(lid) );
	
	if( access_deleted_lock ){
		restore(ps);
		return DELETED;
	}

	// TBD: check acquire lock due to release?
	/*if( (type==WRITE) && (!isfreelock(lid)) ) 
		kprintf("[ERROR] write can not get unfree lock after release!\n");
	if( (type==READ) && (!isfreelock(lid)) ){
		if( !isreadlock(lid))
			kprintf("[ERROR] read can not get unfree and unread lock after release!\n");
		int maxprio;
		find_highest_prio(lid, WRITE, &maxprio);
		int read_priority_larger_than_max_write_priority = (priority > maxprio);
		if( !read_priority_larger_than_max_write_priority )
			kprintf("[ERROR] read doesnt have high priority than max waiting write priority after release!\n");

	}*/
	lptr->lstate = type;
	set_hold(lid, currpid);
	
	restore(ps);
	return pptr->pwaitret;
}

/* ldes register definition
 * ldes bits = [	31 to (N+1)	|	N to 0	]
 * 			token		|	lid
 * */
int get_lid_and_token(int ldes, int* token){
	int 	LIDBIT_MASK, lid;

       	LIDBIT_MASK = ( 1 << LIDBITS ) - 1;
	lid = ldes & LIDBIT_MASK;
	*token = (ldes >> LIDBITS);
	return lid;	
}

int get_lid(int ldes){
	int 	LIDBIT_MASK, lid;
       	
	LIDBIT_MASK = ( 1 << LIDBITS ) - 1;
	lid = ldes & LIDBIT_MASK;
	return lid;	
}

void set_hold(int lid, int pid)
{
	locktab[lid].lholdproc[pid] = 1;
	proctab[pid].pholdlock[lid] = 1;
	return;
}

void clear_hold(int lid, int pid)
{
	locktab[lid].lholdproc[pid] = 0;
	proctab[pid].pholdlock[lid] = 0;
	return;
}

void pinh_raise(int lid, int currprio)
{
	int 		pid;
	int		lid2;
	int		holder_is_waiting, holder_is_ready;
	struct	lentry	*lptr;
	struct	pentry	*pptr;

	lptr = &locktab[lid];
	// find all lock holder, and raise their priority (if higher)
	for(pid=0 ; pid <NPROC; pid++){
		// raise priority if other process hold this lock
		if( lptr->lholdproc[pid] == 0 )
			continue;
		
		// raise priority if lock holder has lower priority
		if( getprio(pid) >= currprio )
			continue;

		pptr = &proctab[pid];
		pptr->pprio = currprio;
		holder_is_ready = (pptr->pstate == PRREADY);
		holder_is_waiting = (pptr->pstate == PRWAIT) && (!isbadlock( pptr->pwaitlock ) );
		
		if( holder_is_ready ){
			dequeue(pid);
			insert(pid,rdyhead, currprio);
		} else	
		// if lock holder is waiting on a second lock, 
		// raise priority of the holder of the second lock.
		if( holder_is_waiting ){
			pinh_raise( pptr->pwaitlock , currprio );
		}
	}
	return;
}
