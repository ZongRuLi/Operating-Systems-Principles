/* resched.c  -  resched */
/* PA1
 *	add two scheduler - Aging based scheduler and Linux-like scheduler
 * */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lab1.h"

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------
 * Notes:	In XINU OS, q[] is maintained as following logic: 
 *	q[0~NPROC]: store the link list node of each process	
 *	q[rdyhead] -> q[pid_1] -> q[pid_2] -> q[pid_3] -> q[rdytail] 
 *	The relation of nodes following by the acending order of qkey(pprio) and age
 *	q[pid_1].qkey < q[pid_2].qkey < q[pid_3].qkey .....
 *	if they has same key,
 *	q[pid_1] T3(youngest) < q[pid_2] T2 < q[pid_3] T1(oldest) .....
 *	To maintain the order, make sure you follows:
 *		1. using insert() function to insert current process back to ready queue
 *		2. dequeue doesn't matter, you can either use dequeue(pid) or getlast(rdytail) function
 * */
static int currSchedClass =0;

void setschedclass (int sched_class){ currSchedClass = sched_class; };

int getschedclass(){ return currSchedClass; };

int resched()
{

	// default schedule class
	if( getschedclass() == LINUXSCHED ){ 
	       return LinuxResched();
	} else if( getschedclass() == AGESCHED ){
		return AgeResched();
	} else {
		return DefaultResched();
	}
}

int DefaultResched(){
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	
	/* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
int AgeResched(){
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	
	/* increment all process's pprio(instead of PRCURR) in ready queue */

	int pid;
	for( pid = firstid(rdyhead) ; pid != rdytail ; pid = q[pid].qnext){
		if(pid == NULLPROC) continue;
		q[pid].qkey++;
		proctab[pid].pprio++;
	}

	/* no switch needed if current process priority higher than next*/
	
	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
int LinuxResched(){
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	register struct pentry 	*pptr;
	
	/* decrease goodness by the amount consumed quantum */
	
	(optr= &proctab[currpid])->goodness += (preempt - optr->counter); 
	optr->counter = preempt;

	/* if counter <= 0, clear goodness
	 * if running NULL process, counter should clear
	 *  to prevent affecting calculation of total runnable time quantum */

	if(optr->counter <=0 || currpid == NULLPROC){	
		optr->goodness = 0;
		optr->counter = 0;
	}

	/* start new epoch it there is no runnable time quantum */

	if(getTotalRunnableTimeQuantum() <= 0 ){
		startNewEpoch();
	}

	/* find the max goodness in all ready process */
	
	int	maxpid = NULLPROC;
	int	maxgoodness = 0;
	findMaxGoodness(&maxpid, &maxgoodness);

	/* no switch needed if NULL process is the only one runnable process */ 
	
	if( (currpid == NULLPROC) && (maxpid == NULLPROC) ){ return(OK); }

	/* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (optr->counter > 0) && (optr->goodness >= maxgoodness) ) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest goodness process in the ready list */

	nptr = &proctab[ (currpid = dequeue(maxpid)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = nptr->counter;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}

int findMaxGoodness(int *maxpid, int *maxgoodness){
	register struct pentry 	*pptr;
	int pid;
	for( pid = firstid(rdyhead) ; pid != rdytail ; pid = q[pid].qnext){
		// if all process has same goodness, maxgoodness is the process which is oldest and the last one before rdytail.
		if( ((pptr = &proctab[pid])->goodness >= *maxgoodness) && (pptr->counter > 0) ){
			*maxpid = pid;
			*maxgoodness = pptr->goodness;
		}	
	}
	return maxgoodness;
}

void startNewEpoch(){
	register struct pentry 	*pptr;
	int 	pid;
	for( pid=0 ; pid<NPROC ; pid++ ){
		pptr= &proctab[pid];
		int timeQuantum = pptr->pprio + pptr->counter/2;
		pptr->counter = timeQuantum;
		pptr->goodness = 0;
	}
	for( pid = firstid(rdyhead) ; pid != rdytail ; pid = q[pid].qnext){
		if(pid == NULLPROC) continue;
		pptr->goodness = (pptr= &proctab[pid])->pprio + pptr-> counter; 
	}
	pptr= &proctab[currpid];
	pptr->goodness = pptr->pprio + pptr-> counter;
	return;
}
int getTotalRunnableTimeQuantum(){
	int pid, sum=0;
	for( pid = firstid(rdyhead) ; pid != rdytail ; pid = q[pid].qnext){
		sum += proctab[pid].counter; 
	}
	sum += proctab[currpid].counter; 
	return sum;
}
