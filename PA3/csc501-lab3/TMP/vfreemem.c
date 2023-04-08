/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	struct	mblock	*p, *q;	
	unsigned top;
	int		blk_out_of_range;
	int		blk_has_no_size;

	disable(ps);

	blk_out_of_range 	= (unsigned)block < 4096 * NBPG;
	blk_has_no_size		= size == 0;

	if(blk_has_no_size)
	{
		restore(ps);
		return SYSERR;	
	}

	size = (unsigned) roundmb(size);
	
	for( p = proctab[currpid].vmemlist->mnext ,q = proctab[currpid].vmemlist;
		p != (struct mblock *) NULL && p < block ;
		q=p,p=p->mnext )
			;

	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= proctab[currpid].vmemlist) ||
		(p!=NULL && (size+(unsigned)block) > (unsigned)p )) 
	{
			restore(ps);
			return SYSERR;
	}

	if ( q!= proctab[currpid].vmemlist && top == (unsigned)block) 
	{
		q->mlen += size;
	} 
	else 
	{
		block->mlen = size;
        block->mnext = p;        
		q->mnext = block;		        
		q = block;
	}
	
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) 
	{
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}

	restore(ps);
	return(OK);
}
