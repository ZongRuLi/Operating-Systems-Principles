/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
#include <Debug.h>

int create_pt(pd_t* pd){
	int f, i;
	pt_t *pt;
	
	/* obtain a frame for pt */	
	get_frm( &f );

	int fail_to_get_frame = (f == SYSERR);
	
	if(fail_to_get_frame)
		return SYSERR;

	/* define the new frame to page table in frame table */
	frm_tab[f].fr_status = FRM_MAPPED;
	frm_tab[f].fr_type = FR_TBL;
	frm_tab[f].fr_pid = currpid;

	/* set page directory entry to point to the above crated page table */
	*pd = clear_pd_entry;
	pd->pd_pres = 1;
	pd->pd_write = 1;
	pd->pd_base = FRAME0 + f;

	/* initialize each entry in page table */	
	for( pt = (pt_t *)((FRAME0 + f) * NBPG), i = 0; i < NFRAMES; i++ ) 
	{
			pt[i] = clear_pt_entry;
	}
	return ( FRAME0 + f );
}
/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

  //kprintf("To be implemented!\n");
	STATWORD        ps;
	unsigned long a;
	unsigned long pdbr;
	unsigned int pd_offset;
	unsigned int pt_offset;
	int f; /* free frame */
	int s; /* store in bs */
	int o; /* page offset */
	int	vp; /* virtual page number */
	virt_addr_t *virtual_address;
	pt_t *pt;
	pd_t *pd;    

	disable(ps);

	/* 1. Get the faulted address a. */
	a = read_cr2();
	
	/* 2. Let vp be the virtual page number of the page containing the faulted address. */	
	vp = a >> 12; // [31:12] virtual page number , [11:0] page offset, page size 4096
	virtual_address = (virt_addr_t*)&a;

	virt_addr_t *va = virtual_address;
	lDebug(DBG_FLOW,"[pfint]va=0x%08x, 2, va->pd_offset=%d, va->pt_offset=%d, va_pg_offset=%d, vp=0x%08x",*va, va->pd_offset, va->pt_offset, va->pg_offset, vp);
	
	/* 3. Let pd point to the current page directory. */
	pdbr = proctab[currpid].pdbr;
   	pd = pdbr + ( virtual_address->pd_offset * sizeof(pd_t) );
	lDebug(DBG_INFO,"[pfint] pdbr(%02d)=0x%08x, pd=0x%08x",currpid, pdbr, pd);
	/* 4. Check that a is a legal address (i.e. that it has been mapped in pd). TBD: what is mapped in pd??
	 * 	  If it is not, print an error message and kill the process. 
	 * 8.1. Using the backing store map, find the store s and page offset o which correspond to vp. */
	int a_is_not_mapped_in_bsm = bsm_lookup(currpid, vp, &s, &o) == SYSERR;
	
	int va_is_not_legal = 0;

	/*if( (!a_is_not_mapped_in_bsm) && (bsm_tab[s].bs_private == BSM_PRIVATE) &&(!vcheckmem_bypass) )
	{
		va_is_not_legal = check_va_legal(a) == SYSERR;
	}*/

	if( a_is_not_mapped_in_bsm || va_is_not_legal ) 
	{
		lDebug(DBG_ERR,"[ERROR][pfint] va= 0x%08x is not mapped in pd! or it mapped in vheap but not vgetmem yet!",a);
		kill(currpid);
		restore(ps);
		return SYSERR;
	}
	// TBD: detect if va is in legal virtual heap! ex. map but not vgetmem, virtual address is illegal.

	/* 5. Let p be the upper ten bits of a. [31:22], which is the page directory offset */
	pd_offset = virtual_address->pd_offset;

	/* 6. Let q be the bits [21:12] of a, which is the page table offset */
	pt_offset = virtual_address->pt_offset;

	/* 7. If the p-th page table does not exist, obtain a frame for it and initialize it. 
	 * page directory entry is not present */
	if(!pd->pd_pres) 
	{
		int		fid = create_pt(pd);
		int		create_pt_fail = (fid == SYSERR);

		if(create_pt_fail)
		{
			lDebug(DBG_ERR,"[ERROR][pfint] can't create frame for page directory!");
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
		lDebug(DBG_INFO,"[INFO][pfint] create PT in frame(%d)",fid - FRAME0);
	}

	/* 7. Let pt point to the p-th page table. allocate page table entry */
	/* p-th page table = p * NBPG */
	/* entry in p-th page table = p * NBPG + q * sizeof(pt_t) */
	pt = (pt_t*) (pd->pd_base * NBPG + pt_offset * sizeof(pt_t));
	
	/* page table entry is not present, page is not in frame. this is why we have page fault */
	if(!pt->pt_pres) {
		
		/* 8.3. Obtain a free frame, f. */
	
		int fail_to_get_frame = get_frm(&f) == SYSERR;
		if( fail_to_get_frame || (f<0) || (f>=NFRAMES) )
		{
			kprintf("[ERROR][pfint] fail to get frame for page!");
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
		lDebug(DBG_INFO,"[INFO][pfint] get frame (%d) for page.",f);
		/* 8.2. In the inverted page table, increase the reference count of the frame that holds pt.
		 * This indicates that one more of pt's entries is marked as "present." */
		frm_tab[pd->pd_base - FRAME0].fr_refcnt++;

		lDebug(DBG_INFO,"[INFO][pfint] increase refcnt(%d) in PT frame(%d)",frm_tab[pd->pd_base - FRAME0].fr_refcnt, pd->pd_base - FRAME0);
		
		/* append page replacement policy queue*/
		// TBD: insert frame in to SC
		fq_push_back(f);

		/* 8.4. Copy the page o of store s to f. */
		read_bs((char *)((FRAME0 + f) * NBPG), s, o);

		/* 8.5. Update pt to mark the appropriate entry as present and set any other fields.
		 * Also set the address portion within the entry to point to frame f. */
		pt->pt_pres = 1;
		pt->pt_write = 1;
		pt->pt_base = FRAME0 + f;

		/* define the new frame to page entry in frame table */
		frm_tab[f].fr_status = FRM_MAPPED;
		frm_tab[f].fr_type = FR_PAGE;
		frm_tab[f].fr_pid = currpid;
		frm_tab[f].fr_vpno = vp; /* vp number =[31:12], vp offset =[11:0], page size= 4KB */
		frm_tab[f].fr_bsid = s;
		frm_tab[f].fr_pageth = o;

		lDebug(DBG_INFO,"[INFO][pfint] create page in frame(%d), pid=%d, vpno=0x%08x",f, currpid, vp);
		lDebug(DBG_INFO,"[INFO][pfint] pid(%d), pdbr=0x%08x, pd=0x%08x, pt=0x%08x",currpid, proctab[currpid].pdbr, pd,pt);
	}
	
	write_cr3(proctab[currpid].pdbr);
	lDebug(DBG_FLOW,"[INFO][pfint] finish pfint");
	
	restore(ps);
	return OK;
}
