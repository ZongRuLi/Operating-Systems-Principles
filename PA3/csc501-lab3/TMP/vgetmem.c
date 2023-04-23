/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
#include <Debug.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	//kprintf("To be implemented!\n");
	STATWORD ps;
	struct	mblock	*p, *q, *leftover;
	struct	pentry	*pptr;

	disable(ps);
	vcheckmem_bypass = 1;

	if (nbytes==0 || proctab[currpid].vmemlist.mnext == (struct mblock *) NULL) 
	{    
		restore(ps); 
		return( (WORD *)SYSERR);
	} 
	
	pptr= &proctab[currpid];
	nbytes = (unsigned int) roundmb(nbytes);
	//lDebug(DBG_FLOW, "[INFO] enter vgetmem with roundmb(nbytes) = %d", nbytes );
	//lDebug(DBG_INFO, "[INFO] &vmemlist = 0x%08x(==NULL? %d), vmemlist->next= 0x%08x", &pptr->vmemlist, &pptr->vmemlist == (struct mblock*) NULL, pptr->vmemlist.mnext );

	for ( q = &proctab[currpid].vmemlist, p = proctab[currpid].vmemlist.mnext ;
		p != (struct mblock *) NULL ;
		q = p , p = p->mnext ) 
	{
		//lDebug(DBG_INFO,"[LOOP] q= 0x%08x, p= 0x%08x, p->mlen=%d", q , p, p->mlen);
		if (p->mlen < nbytes) 
			continue;
		
		if (p->mlen == nbytes) 
			q->mnext = p->mnext; 

		if (p->mlen > nbytes) 
		{
			leftover = (struct mblock *) ((unsigned) p + nbytes);
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
		}
		vcheckmem_bypass = 0;
		restore(ps);
		return ((WORD *) p);
	}
	
	vcheckmem_bypass = 0;
	restore(ps);
	return( (WORD *)SYSERR);
}
