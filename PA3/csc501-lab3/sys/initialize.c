/* initialize.c - nulluser, sizmem, sysinit */

#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <paging.h>
#include <fq.h>
#include <Debug.h>

/*#define DETAIL */
#define HOLESIZE	(600)	
#define	HOLESTART	(640 * 1024)	/* 160 * 4096 */
#define	HOLEEND		((1024 + HOLESIZE) * 1024)  /* 1624 * 1024 = 406 * 4096 */
/* Extra 600 for bootp loading, and monitor */

extern	int	main();	/* address of user's main prog	*/
extern buguser();

extern	int	start();

LOCAL		sysinit();

/* Declarations of major kernel variables */
struct	pentry	proctab[NPROC]; /* process table			*/
int	nextproc;		/* next process slot to use in create	*/
struct	sentry	semaph[NSEM];	/* semaphore table			*/
int	nextsem;		/* next sempahore slot to use in screate*/
struct	qent	q[NQENT];	/* q table (see queue.c)		*/
int	nextqueue;		/* next slot in q structure to use	*/
char	*maxaddr;		/* max memory address (set by sizmem)	*/
struct	mblock	memlist;	/* list of free memory blocks		*/
#ifdef	Ntty
struct  tty     tty[Ntty];	/* SLU buffers and mode control		*/
#endif

/* active system status */
int	numproc;		/* number of live user processes	*/
int	currpid;		/* id of currently running process	*/
int	reboot = 0;		/* non-zero after first boot		*/

int	rdyhead,rdytail;	/* head/tail of ready list (q indicies)	*/
char 	vers[80];
int	console_dev;		/* the console device			*/

/*  added for the demand paging */
int page_replace_policy = SC;
/* PA3 */
int 	debugLevel = DBG_LV;
int 	print_replace = 0;
int		vcheckmem_bypass =0;

bs_map_t bsm_tab[NBSM];
fr_map_t frm_tab[NFRAMES];
int		fid_global_pt[4];
//pr_map_t prp_tab[NFRAMES];

const pt_t clear_pt_entry = { 
		.pt_pres=0,		.pt_write=0,	.pt_user=0, 	.pt_pwt=0, 
		.pt_pcd=0,		.pt_acc=0, 		.pt_dirty=0, 	.pt_mbz=0, 
		.pt_global=0,	.pt_avail=0, 	.pt_base=0 
};

const pd_t clear_pd_entry = {
		.pd_pres=0, 	.pd_write=0, 	.pd_user=0,
		.pd_pwt=0, 		.pd_pcd=0, 		.pd_acc=0, 
		.pd_mbz=0, 		.pd_fmb=0, 		.pd_global=0, 
		.pd_avail=0, 	.pd_base=0
};

/*******/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED, and      ***/
/***   must eventually be enabled explicitly.  This routine turns     ***/
/***   itself into the null process after initialization.  Because    ***/
/***   the null process must always remain ready to run, it cannot    ***/
/***   execute code that might cause it to be suspended, wait for a   ***/
/***   semaphore, or put to sleep, or exit.  In particular, it must   ***/
/***   not do I/O unless it uses kprintf for polled output.           ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  nulluser  -- initialize system and become the null process (id==0)
 *------------------------------------------------------------------------
 */
nulluser()				/* babysit CPU when no one is home */
{
        int userpid;

	console_dev = SERIAL0;		/* set console to COM0 */

	initevec();

	kprintf("system running up!\n");
	sysinit();

	enable();		/* enable interrupts */

	sprintf(vers, "PC Xinu %s", VERSION);
	kprintf("\n\n%s\n", vers);
	if (reboot++ < 1)
		kprintf("\n");
	else
		kprintf("   (reboot %d)\n", reboot);


	kprintf("%d bytes real mem\n",
		(unsigned long) maxaddr+1);
#ifdef DETAIL	
	kprintf("    %d", (unsigned long) 0);
	kprintf(" to %d(pn %d)\n", (unsigned long) (maxaddr), (unsigned long) (maxaddr) >>12 );
#endif	

	kprintf("%d bytes Xinu code\n",
		(unsigned long) ((unsigned long) &end - (unsigned long) start));
#ifdef DETAIL	
	kprintf("    %d(pn %d)", (unsigned long) start, (unsigned long) start >>12);
	kprintf(" to %d(pn %d)\n", (unsigned long) &end, (unsigned long) &end >> 12 );
#endif

#ifdef DETAIL	
	kprintf("%d bytes user stack/heap space\n",
		(unsigned long) ((unsigned long) maxaddr - (unsigned long) &end));
	kprintf("    %d(pn %d)", (unsigned long) &end, (unsigned long) &end >> 12);
	kprintf(" to %d(pn %d)\n", (unsigned long) maxaddr, (unsigned long) maxaddr >> 12);
#endif	
	
	kprintf("clock %sabled\n", clkruns == 1?"en":"dis");


	//kprintf("proctab[20] = %s\n",proctab[20].pname);
	kprintf("currpid: %d, ready queue: ",currpid);
	int next;
	for( next = q[rdyhead].qnext ; next != rdytail ; next = q[next].qnext ){
		kprintf("%d,",q[next].qkey );
	}
	kprintf(".\n");
	/* create a process to execute the user's main program */
	userpid = create(main,INITSTK,INITPRIO,INITNAME,INITARGS);
	resume(userpid);

	while (TRUE)
		/* empty */;
}

/*------------------------------------------------------------------------
 *  sysinit  --  initialize all Xinu data structeres and devices
 *------------------------------------------------------------------------
 */
LOCAL
sysinit()
{
	static	long	currsp;
	int	i,j;
	struct	pentry	*pptr;
	struct	sentry	*sptr;
	struct	mblock	*mptr;
	SYSCALL pfintr();

	/* PA3 */	
	int		fid =0;
	pt_t	*pt_entry;
	pd_t	*pd_entry;
	/*******/

	numproc = 0;			/* initialize system variables */
	nextproc = NPROC-1;
	nextsem = NSEM-1;
	nextqueue = NPROC;		/* q[0..NPROC-1] are processes */

	/* initialize free memory list */
	/* PC version has to pre-allocate 640K-1024K "hole" */
	if (maxaddr+1 > HOLESTART) {
		memlist.mnext = mptr = (struct mblock *) roundmb(&end);
		mptr->mnext = (struct mblock *)HOLEEND;
		mptr->mlen = (int) truncew(((unsigned) HOLESTART -
	     		 (unsigned)&end));
        mptr->mlen -= 4;

		mptr = (struct mblock *) HOLEEND;
		mptr->mnext = 0;
		mptr->mlen = (int) truncew((unsigned)maxaddr - HOLEEND -
	      		NULLSTK);
/*
		mptr->mlen = (int) truncew((unsigned)maxaddr - (4096 - 1024 ) *  4096 - HOLEEND - NULLSTK);
*/
	} else {
		/* initialize free memory list */
		memlist.mnext = mptr = (struct mblock *) roundmb(&end);
		mptr->mnext = 0;
		mptr->mlen = (int) truncew((unsigned)maxaddr - (int)&end -
			NULLSTK);
	}
	

	for (i=0 ; i<NPROC ; i++)	/* initialize process table */
		proctab[i].pstate = PRFREE;

#ifdef	MEMMARK
	_mkinit();			/* initialize memory marking */
#endif

#ifdef	RTCLOCK
	clkinit();			/* initialize r.t.clock	*/
#endif

	mon_init();     /* init monitor */

#ifdef NDEVS
	for (i=0 ; i<NDEVS ; i++ ) {	    
	    init_dev(i);
	}
#endif

	pptr = &proctab[NULLPROC];	/* initialize null process entry */
	pptr->pstate = PRCURR;
	for (j=0; j<7; j++)
		pptr->pname[j] = "prnull"[j];
	pptr->plimit = (WORD)(maxaddr + 1) - NULLSTK;
	pptr->pbase = (WORD) maxaddr - 3;
	//lDebug(DBG_FLOW,"null pbase=0x%08x, pn=%d",pptr->pbase, pptr->pbase>>12);
	//lDebug(DBG_FLOW,"null plimit=0x%08x, pn=%d",pptr->plimit, pptr->plimit>>12);
/*
	pptr->plimit = (WORD)(maxaddr + 1) - NULLSTK - (4096 - 1024 )*4096;
	pptr->pbase = (WORD) maxaddr - 3 - (4096-1024)*4096;
*/
	pptr->pesp = pptr->pbase-4;	/* for stkchk; rewritten before used */
	*( (int *)pptr->pbase ) = MAGIC;
	pptr->paddr = (WORD) nulluser;
	pptr->pargs = 0;
	pptr->pprio = 0;
	currpid = NULLPROC;

	for (i=0 ; i<NSEM ; i++) {	/* initialize semaphores */
		(sptr = &semaph[i])->sstate = SFREE;
		sptr->sqtail = 1 + (sptr->sqhead = newqueue());
	}

	rdytail = 1 + (rdyhead=newqueue());/* initialize ready list */

	/* PA3 */
	/* for process scheduling*/
	pptr->ppolicy = 0;                /* process scheduling policy    */
	pptr->ppi = 0;                    /* priority value in psp        */
	pptr->prate = 0;                  /* rate value in psp            */

	/* for demand paging */
	pptr->store= -1;                  /* backing store for vheap      */
    pptr->vhpno = 0;                  /* starting pageno for vheap    */
	pptr->vhpnpages = 0;              /* vheap size                   */
    pptr->vmemlist.mlen = 0;
	pptr->vmemlist.mnext = (struct mblock *) NULL;        /* vheap list              	*/

	pptr->bsm_num = 0;

	/* 2. Initialize all necessary data structures. */
	init_bsm();
	init_frm();
	init_fqueue();

	/* 3. Create 4 global page tables which will map 
	 * pages 0 through 4095 to the physical 16 MB. */ 
	for (i = 0; i < 4; i++) 
	{
		/* create new frame, labeled it as page table */
		get_frm(&fid);
		
		frm_tab[fid].fr_status = FRM_MAPPED;
		frm_tab[fid].fr_type = FR_TBL;
		frm_tab[fid].fr_pid = NULLPROC;// TBD: NULLPROC ?

		pt_entry = (pt_t *)((FRAME0 + fid) * NBPG);

		/* insert 1024 entry to page table */
		for (j = 0; j < (NBPG/4); j++, pt_entry++) 
		{
			*pt_entry = clear_pt_entry;
			pt_entry->pt_pres = 1;
			pt_entry->pt_write = 1;
			pt_entry->pt_global = 1;
			pt_entry->pt_base = (i * NBPG)/4 + j;	
		}
		fid_global_pt[i] = fid;
		lDebug(DBG_FLOW,"global PT(%d) at frame(%d)\n",i, fid);
	}
	/* 4. Allocate and initialize a page directory for the NULL process.*/
	get_frm(&fid);
	
	frm_tab[fid].fr_status = FRM_MAPPED;
	frm_tab[fid].fr_type = FR_DIR;
	frm_tab[fid].fr_pid = NULLPROC;

	proctab[NULLPROC].pdbr = (FRAME0 + fid) * NBPG;
	pd_entry = (pd_t *)proctab[NULLPROC].pdbr;
	
	for (i = 0; i < (NBPG/4); i++, pd_entry++) 
	{
		*pd_entry = clear_pd_entry;
		pd_entry->pd_write = 1;
		
		if (i < 4) {
				pd_entry->pd_pres = 1;
				pd_entry->pd_base = FRAME0 + fid_global_pt[i];
		}
	}
	/* 5. Set the PDBR register to the page directory for the NULL process.*/
	write_cr3(proctab[NULLPROC].pdbr);
	/* 6. Install the page fault interrupt service routine. */
	set_evec(14, (u_long)pfintr); /* handle page fault */
	/* 7. Enable paging. */
	enable_paging();
	/*******/

	return(OK);
}

stop(s)
char	*s;
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* empty */;
}

delay(n)
int	n;
{
	DELAY(n);
}


#define	NBPG	4096

/*------------------------------------------------------------------------
 * sizmem - return memory size (in pages)
 *------------------------------------------------------------------------
 */
long sizmem()
{
	unsigned char	*ptr, *start, stmp, tmp;
	int		npages;

	/* at least now its hacked to return
	   the right value for the Xinu lab backends (16 MB) */

	return 4096; 

	start = ptr = 0;
	npages = 0;
	stmp = *start;
	while (1) {
		tmp = *ptr;
		*ptr = 0xA5;
		if (*ptr != 0xA5)
			break;
		*ptr = tmp;
		++npages;
		ptr += NBPG;
		if ((int)ptr == HOLESTART) {	/* skip I/O pages */
			npages += (1024-640)/4;
			ptr = (unsigned char *)HOLEEND;
		}
	}
	return npages;
}
