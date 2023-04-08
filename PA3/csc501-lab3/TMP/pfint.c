/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

int create_pt(){
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

	/* initialize page table */	
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
	unsigned int pd_offset;
	unsigned int pt_offset;
	int f; /* free frame */
	int s; /* store in bs */
	int o; /* page offset */
	virt_addr_t *virtual_address;
	pt_t *pt;
	pd_t *pd;    

	disable(ps);

	/* 1. Get the faulted address a. */
	a = read_cr2();
	
	/* 2. Let vp be the virtual page number of the page containing the faulted address. */	
	virtual_address = (virt_addr_t*)&a;

	/* 3. Let pd point to the current page directory. */
   	pd = proctab[currpid].pdbr + ( virtual_address->pd_offset * sizeof(pd_t) );

	/* 4. Check that a is a legal address (i.e. that it has been mapped in pd).
	 * 	  If it is not, print an error message and kill the process. 
	 * 8.1. Using the backing store map, find the store s and page offset o which correspond to vp. */
	int a_is_not_mapped_in_pd = bsm_lookup(currpid, a, &s, &o) == SYSERR;
	
	if( a_is_not_mapped_in_pd ) {
		kill(currpid);
		restore(ps);
		return SYSERR;
	}

	/* 5. Let p be the upper ten bits of a. [31:22], which is the page directory offset */
	pd_offset = virtual_address->pd_offset;

	/* 6. Let q be the bits [21:12] of a, which is the page table offset */
	pt_offset = virtual_address->pt_offset;

	/* 7. If the p-th page table does not exist, obtain a frame for it and initialize it. 
	 * page directory entry is not present */
	if(!pd->pd_pres) {
		int		frame_num = create_pt();
		int		create_pt_fail = (frame_num == SYSERR);

		if(create_pt_fail){
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
        /* set page directory entry to point to the above crated page table */
		*pd = clear_pd_entry;
		pd->pd_pres = 1;
		pd->pd_write = 1;
		/*pd->pd_user = 0;
		pd->pd_pwt = 0;
		pd->pd_pcd = 0;
		pd->pd_acc = 0;
		pd->pd_mbz = 0;
		pd->pd_fmb = 0;
		pd->pd_global = 0;
		pd->pd_avail = 0;*/
		pd->pd_base = frame_num;
	}

	/* 7. Let pt point to the p-th page table. allocate page table entry */
	pt = (pt_t*) (pd->pd_base * NBPG + pt_offset * sizeof(pt_t));
	
	/* page table entry is not present, page is not in frame. this is why we have page fault */
	if(!pt->pt_pres) {
		/* 8.2. In the inverted page table, increase the reference count of the frame that holds pt.
		 * This indicates that one more of pt's entries is marked as "present." */
		frm_tab[pd->pd_base - FRAME0].fr_refcnt++;

		// TBD: will increase frame cnt before get frame be problem? if get frame fail
		/* 8.3. Obtain a free frame, f. */
		get_frm(&f);
	
		int fail_to_get_frame = (f == SYSERR);

		if( fail_to_get_frame ){
			kill(currpid);
			restore(ps);
			return SYSERR;
		}
		
		/* append page replacement policy queue*/
		// TBD
		//append_prp_tab(f);

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
		frm_tab[f].fr_vpno = a >> 12; /* vp number =[31:12], vp offset =[11:0], page size= 4KB */
	}
	
	write_cr3(proctab[currpid].pdbr);
	restore(ps);
	return OK;
}


