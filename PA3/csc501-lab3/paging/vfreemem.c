/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
#include <Debug.h>

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
	unsigned 		top;
	int				blk_out_of_range;
	int				blk_has_no_size;
	unsigned 		minvaddr, maxvaddr;

	disable(ps);
	vcheckmem_bypass = 1;

	if( isbad_bsid(proctab[currpid].store) ){ 
		restore(ps); 
		vcheckmem_bypass = 0;
		return SYSERR; 
	} 

	minvaddr = (unsigned) (proctab[currpid].vhpno) * NBPG; // 4096 * NBPG
	maxvaddr = (unsigned) (proctab[currpid].vhpno + proctab[currpid].vhpnpages) * NBPG -4;
	
	blk_has_no_size		= size == 0;
	blk_out_of_range 	= (unsigned)block < (unsigned)minvaddr
		   	|| (unsigned)block > (unsigned)maxvaddr;

	if(blk_has_no_size || blk_out_of_range)
	{
		vcheckmem_bypass = 0;
		restore(ps);
		return SYSERR;	
	}

	size = (unsigned) roundmb(size);
	
	for( p = proctab[currpid].vmemlist.mnext , q = &proctab[currpid].vmemlist;
		p != (struct mblock *) NULL && p < block ;
		q = p , p = p->mnext )
			;

	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &proctab[currpid].vmemlist) ||
		(p!=NULL && (size+(unsigned)block) > (unsigned)p )) 
	{
		vcheckmem_bypass = 0;
		restore(ps);
		return SYSERR;
	}

	if ( q!= &proctab[currpid].vmemlist && top == (unsigned)block) 
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
	
	/* free frame, prevent access to freed virtaul address. */	
	/*lDebug(DBG_INFO,"[INFO] vfreemem, free frames.");
	int npage = (size - 4) / NBPG + 1;
	int i, fid;
	pt_t *pt;
	pd_t *pd;
	unsigned long a;
	virt_addr_t *va;
	
	va = &a;
	for( i = 0 ; i < npage ; i++ )
	{
		a = block + i * NBPG;
		
		pd = (pd_t*) (proctab[currpid].pdbr + va->pd_offset * sizeof(pd_t) );
	
		if(!(pd)->pd_pres){ 
			lDebug(DBG_ERR, "[get_page_entry] page table does not exist!!");
			continue;
		}

		pt = (pt_t*) ( pd->pd_base * NBPG + va->pt_offset * sizeof(pt_t) );	

		fid = pt->pt_base - FRAME0;

		free_frm(fid);
		if( i == fqcurrpos ){ fq_next_pos(); }	
		fq_delete(i);	
	}*/

	vcheckmem_bypass = 0;
	restore(ps);
	return(OK);
}


/*------------------------------------------------------------------------
 *  check_va_legal  --  check if we are able to free one virtual memory address, 
 *  if yes, this means it is a legal virtual address allocated by vgetmem!
 *------------------------------------------------------------------------
 */
int check_va_legal(unsigned long block)
{
	int 			size = 4; 
	STATWORD ps;
	struct	mblock	*p, *q;	
	unsigned 		top;
	int				blk_out_of_range;
	int				blk_has_no_size;
	unsigned 		minvaddr, maxvaddr;

	disable(ps);
	vcheckmem_bypass = 1;

	if( isbad_bsid(proctab[currpid].store) ){ restore(ps); return SYSERR; } 

	minvaddr = (unsigned) (proctab[currpid].vhpno) * NBPG; // 4096 * NBPG
	maxvaddr = (unsigned) (proctab[currpid].vhpno + proctab[currpid].vhpnpages) * NBPG -4;
	
	blk_has_no_size		= size == 0;
	blk_out_of_range 	= (unsigned)block < (unsigned)minvaddr
		   	|| (unsigned)block > (unsigned)maxvaddr;

	if(blk_has_no_size || blk_out_of_range)
	{
		vcheckmem_bypass = 0;
		return SYSERR;	
	}

	size = (unsigned) roundmb(size);
	lDebug(DBG_FLOW, "[INFO][check_va_legal] enter with va 0x%08x roundmb(size) = %d", block, size );
	
	for( p = proctab[currpid].vmemlist.mnext , q = &proctab[currpid].vmemlist;
		p != (struct mblock *) NULL && p < block ;
		q = p , p = p->mnext ){
			lDebug(DBG_INFO,"[LOOP] q= 0x%08x, q->mlen= %d, p= 0x%08x", q ,q->mlen, p);
			;
	}
	lDebug(DBG_INFO,"vmemlist->next = 0x%08x ", proctab[currpid].vmemlist.mnext);	
	lDebug(DBG_INFO,"block= %d, top = 0x%08x q= 0x%08x, p= 0x%08x size+block= 0x%08x\n", block, q->mlen + (unsigned)q, q, p, (size+(unsigned)block));
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &proctab[currpid].vmemlist) ||
		(p!=NULL && (size+(unsigned)block) > (unsigned)p )) 
	{
		vcheckmem_bypass = 0;
		return SYSERR;
	}
	vcheckmem_bypass = 0;
	return OK;
}
