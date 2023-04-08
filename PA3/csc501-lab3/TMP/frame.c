/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

LOCAL int find_avail_frame(int* avail);
LOCAL int find_victim_frame();
LOCAL void get_page_entry(int pid, int frm_idx, pt_t **pt_entry, pd_t **pd_entry); 
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  //kprintf("To be implemented!\n");
	STATWORD ps;
	int		i;

	disable(ps);

	for (i = 0; i < NFRAMES; i++) 
	{
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = -1;	
		frm_tab[i].fr_vpno = 4096;			    
		frm_tab[i].fr_refcnt = 0;				    
		frm_tab[i].fr_type = FR_PAGE;				    
		frm_tab[i].fr_dirty = 0;
	}

	restore(ps);
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  //kprintf("To be implemented!\n");
	STATWORD ps;
	int 		i, fid, pid, vp;
	int			store, pageth;
	virt_addr_t	*a;
	pt_t		*pt;
	pd_t		*pd;
	unsigned int	p, q;

	disable(ps);

	/* 1. Search inverted page table for an empty frame. If one exists stop. */
	if( find_avail_frame( avail ) == OK )
	{
		restore(ps);
		return OK;
	}
	/* 2. Else, Pick a page to replace. 
	 * if no available frames, get next replaced frame based on page replacement policy */	
	if( (fid = find_victim_frame() ) < 0 )
	{
		restore(ps);
		return SYSERR;
	}

	/* 3. Using the inverted page table, 
	 * get vp, the virtual page number of the page to be replaced.
	 */
	//get_page_entry(frm_tab[fid].fr_pid, fid, &pt_entry, &pd_entry);
	vp = frm_tab[fid].fr_vpno;
	/* 4. Let a be vp*4096 (the first virtual address on page vp).
	 */
	a = vp * 4096;
	/* 5. Let p be the high 10 bits of a. Let q be bits [21:12] of a.
	 */
	p = a->pd_offset;
	q = a->pt_offset;
	/* 6. Let pid be the pid of the process owning vp. */
	pid = frm_tab[fid].fr_pid; //TBD
	/* 7. Let pd point to the page directory of process pid. */
	pd = proctab[pid].pdbr + p * sizeof(pd_t);
	/* 8. Let pt point to the pid’s p-th page table. */
	pt = (pt_t*) (pd->pd_base * NBPG + q * sizeof(pt_t));
	/* 9. Mark the appropriate entry of pt as not present.*/
	pt->pt_pres = 0;	

	/* 10. If the page being removed belongs to the current process, 
	 * invalidate the TLB entry for the page vp using the invlpg instruction (see Intel Volume III/II).
	 * */
	if (frm_tab[fid].fr_pid == currpid) {
	        asm("invlpg (%0)" : : "r"(frm_tab[fid].fr_vpno));
	}

	/* 11. In the inverted page table decrement the reference count of the frame occupied by pt. 
	 * If the reference count has reached zero, you should mark the appropriate entry in pd as being not present. 
	 * This conserves frames by keeping only page tables which are necessary.
	 * */
	if ( (--frm_tab[fid].fr_refcnt) == 0 ) 
	{
		pd->pd_pres = 0;
	}
	/* 12. If the dirty bit for page vp was set in its page table you must do the following: */
	if( pt->pt_dirty == 1 )
	{
	/* 12.1 Use the backing store map to find the store and page offset within store given pid and a. 
	 * If the lookup fails, something is wrong. Print an error message and kill the process pid.*/
		if (bsm_lookup(frm_tab[fid].fr_pid, frm_tab[fid].fr_vpno << 12, &store, &pageth) == SYSERR) 
		{
			kprintf("[get_frm] Get free frame fail, page dirty and lookup fail!!!\n");
			restore(ps); 
			return SYSERR;
		}
	/* 12.2 Write the page back to the backing store.*/
		write_bs((fid + FRAME0) * NBPG, store, pageth);
	}

	free_frm(fid);
	*avail = fid;

	restore(ps);										            
	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
//kprintf("To be implemented!\n");
	STATWORD ps;
	int		bs_id, pageth, fid;
	pt_t 	*pt_entry;
	pd_t 	*pd_entry;
	int		illegal_fid;

	disable(ps);

	illegal_fid = i<0 || i >= NFRAMES;
	if(illegal_fid)
	{
		restore(ps);										            
		return SYSERR;
	}

	if( frm_tab[i].fr_type != FR_PAGE )
	{
		restore(ps);										            
		return SYSERR;
	}

	bs_id = proctab[frm_tab[i].fr_pid].store;
	pageth = frm_tab[i].fr_vpno - proctab[frm_tab[i].fr_pid].vhpno;
	
	get_page_entry(frm_tab[i].fr_pid, i, &pt_entry, &pd_entry);

	write_bs((i + FRAME0) * NBPG, bs_id, pageth);
	
	pt_entry->pt_pres = 0;
	fid = pd_entry->pd_base - FRAME0;

	frm_tab[fid].fr_refcnt--;

	if(frm_tab[fid].fr_refcnt == 0)
	{
		pd_entry->pd_pres = 0;

		frm_tab[fid].fr_status = FRM_UNMAPPED;                
		frm_tab[fid].fr_type = FR_PAGE; 
		frm_tab[fid].fr_pid = -1;
		frm_tab[fid].fr_vpno = 4096;
	}
	restore(ps);									            
	return OK;
}

LOCAL int find_victim_frame()
{
	// TBD
	return 0;
}

LOCAL int find_avail_frame(int* avail)
{
	int		i;
	for (i = 0; i < NFRAMES; i++) {
		if (frm_tab[i].fr_status == FRM_UNMAPPED) {
			*avail = i;
			return OK;
		}											        
	}
	return SYSERR;	
}

LOCAL void get_page_entry(int pid, int frm_idx, pt_t **pt_entry, pd_t **pd_entry) 
{
	virt_addr_t *virt_addr;

	virt_addr = (virt_addr_t *) (frm_tab[frm_idx].fr_vpno << 12);

	*pd_entry = (proctab[pid].pdbr + virt_addr->pd_offset * sizeof(pd_t));
	*pt_entry = (*pd_entry)->pd_base * NBPG + virt_addr->pt_offset * sizeof(pt_t);
}
