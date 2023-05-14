#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

/*
Review definition from proc.h:

struct	pentry	{
	char	pstate;			// process state: PRCURR, etc.	//
	int	pprio;			// process priority		//
	int	pesp;			// saved stack pointer		//
	STATWORD pirmask;		// saved interrupt mask		//
	int	psem;			// semaphore if process waiting	//
	WORD	pmsg;			// message sent to this process	//
	char	phasmsg;		// nonzero iff pmsg is valid	//
	WORD	pbase;			// base of run time stack	//
	int	pstklen;		// stack length			//
	WORD	plimit;			// lowest extent of stack	//
	char	pname[PNMLEN];		// process name			//
	int	pargs;			// initial number of arguments	//
	WORD	paddr;			// initial code address		//
	WORD	pnxtkin;		// next-of-kin notified of death//
	Bool	ptcpumode;		// proc is in TCP urgent mode	//
	short	pdevs[2];		// devices to close upon exit	//
	int	fildes[_NFILE];		// file - device translation	//
	int	ppagedev;		// pageing dgram device		//
	int	pwaitret;
};

extern	struct	pentry proctab[];
extern	int	currpid;		// currently executing process	//	
*/

static unsigned long *esp; //Stack pointer access as defined in ../sys/stacktrace.c

void printprocstks(int priority){	
	
	struct	pentry	*pptr; // process pointer
	int pid;

	kprintf("\n\nvoid printprocstks(int priority)");
	
	for(pid=0; pid <= NPROC ; pid++)
	{
		pptr = &proctab[pid];
		if( (pptr->pstate) == PRFREE || (pptr->pprio <= priority) ) continue;
	
		kprintf("\nProcess [%s]",pptr->pname);		//process name
		kprintf("\n\tpid: %d",pid);			//process id
		kprintf("\n\tpriority: %d",pptr->pprio);	//process priority
		kprintf("\n\tbase: 0x%08x",pptr->pbase);	//stack base
		kprintf("\n\tlimit: 0x%08x",pptr->plimit);	//stack limit
		kprintf("\n\tlen: %d",pptr->pstklen);		//stack size
		if(pid == currpid)
		{
			asm("movl %esp,esp");
			kprintf("\n\tpointer: 0x%08x",esp);	
		}else{
			kprintf("\n\tpointer: 0x%08x",pptr ->pesp);
		}
	
	}	


}
