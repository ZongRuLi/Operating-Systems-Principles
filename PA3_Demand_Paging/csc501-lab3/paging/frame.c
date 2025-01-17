/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <fq.h>
#include <Debug.h>

LOCAL int find_avail_frame(int* avail);
LOCAL int find_victim_frame();

void clear_frm_entry(int i)
{
	frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_pid=-1;
	frm_tab[i].fr_vpno = 0;
	frm_tab[i].fr_refcnt=0;
	frm_tab[i].fr_type=FR_PAGE;
	frm_tab[i].fr_dirty=0;
	frm_tab[i].fr_bsid=-1;
	frm_tab[i].fr_pageth=0;
}
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
		clear_frm_entry(i);
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
	int 	fid;

	/* 1. Search inverted page table for an empty frame. If one exists stop. */
	if( find_avail_frame( avail ) == OK )
	{
		//if( debugLevel >= DBG_INFO ){ fq_print(); }
		return OK;
	}
	/* 2. Else, Pick a page to replace. 
	 * if no available frames, get next replaced frame based on page replacement policy */	
	if( (fid = find_victim_frame() ) < 0 )
	{
		//lDebug(DBG_ERR,"[ERROR][get_frm] frame is full, no PAGE frame exist, no frame can be replaced!");
		return SYSERR;
	}
	//if(print_replace){ lDebug(0,"[get_frm] replace frame (%d)", fid); }
	if(print_replace){ kprintf("%d\n",FRAME0 + fid); }

	//if( debugLevel >= DBG_INFO ){ fq_print(); }

	free_frm( fid );
	
	*avail = fid;

	return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
//kprintf("To be implemented!\n");
	int		bs_id, pageth, f, pid;
	pt_t 	*pt_entry;
	pd_t 	*pd_entry;
	int		illegal_fid;

	//lDebug(DBG_FLOW,"[INFO][free_frm] start ", i);

	// frm 0 -3 is global page table and should never be freed untill whole program end.
	// this is to prevent error.
	illegal_fid = i<4 || i >= NFRAMES;
	if(illegal_fid)
	{
		//lDebug(DBG_ERR,"[ERROR][free_frm] illegal frame id (%d)!!! ", i);
		return SYSERR;
	}

	if( frm_tab[i].fr_type != FR_PAGE )
	{
		//lDebug(DBG_ERR,"[ERROR][free_frm] should not free page table and directory!!! ");
		return SYSERR;
	}
	// get information
	//lDebug(DBG_INFO, "[INFO][free_frm] frame %d start free ", i);
	
	bs_id = frm_tab[i].fr_bsid;
	pageth = frm_tab[i].fr_pageth;
	
	/* 2.5 because there might be many process shared access this frame, 
	 * we have to clear PT entry & PD dentry & PT frame for all of them! */
	
	pid = frm_tab[i].fr_pid;

	//int bsm_map_proc = bsm_tab[bs_id].bs_pid[pid] == 1;
	//int frm_map_proc = frm_tab[i].fr_pid == pid && frm_tab[i].fr_vpno >= 4096;
	
	//if( !bsm_map_proc || !frm_map_proc )
	//{
	//	lDebug(DBG_ERR, "[ERROR][free_frm] process(%d) doesn't hold frame(%d)!!!",pid,i);
	//}

	/* 3. Using the inverted page table, 
	 * get vp, the virtual page number of the page to be replaced. */
	if( get_page_entry(i,pid, &pt_entry, &pd_entry) == SYSERR )
	{
		//lDebug(DBG_ERR, "[ERROR][free_frm] get_page_entry got invalid pd_entry.");
		write_bs((FRAME0 + i) * NBPG, bs_id, pageth);
		clear_frm_entry(i);
		//lDebug(DBG_FLOW, "free_frm done");
		return SYSERR;	
	}
		
	f = pd_entry->pd_base - FRAME0; // f is page table frame
		
	/* 9. Mark the appropriate entry of pt as not present.*/
	*pt_entry = clear_pt_entry;

	//lDebug(DBG_INFO,"[INFO][free_frm] pd_base(%d), page table frame[%d].fr_recnt=%d", pd_entry->pd_base,f, frm_tab[f].fr_refcnt -1 );
	
	/* 10. If the page being removed belongs to the current process, 
	 * invalidate the TLB entry for the page vp using the invlpg instruction (see Intel Volume III/II).
	 * */
	if(pid == currpid){
		asm("invlpg (%0)" : : "r"(frm_tab[f].fr_vpno));
	}
	/* 11. In the inverted page table decrement the reference count of the frame occupied by pt. 
	 * If the reference count has reached zero, you should mark the appropriate entry in pd as being not present. 
	 * This conserves frames by keeping only page tables which are necessary.
	 * */
	if( (--frm_tab[f].fr_refcnt) == 0)
	{
		// clear page directory entry
		*pd_entry = clear_pd_entry;
		// delete page table frame
		clear_frm_entry( f );
	}

	//info("[INFO][free_frm] pid(%d), pdbr=0x%08x, pd=0x%08x, pt=0x%08x",pid, proctab[pid].pdbr ,pd_entry,pt_entry);
	
	/* 12. If the dirty bit for page vp was set in its page table you must do the following: */
	//if( pt->pt_dirty == 1 )
	//{
	/* 12.1 Use the backing store map to find the store and page offset within store given pid and a. 
	 * If the lookup fails, something is wrong. Print an error message and kill the process pid.*/
	//	if (bsm_lookup(pid, vp, &store, &pageth) == SYSERR) 
	//	{
	//		error("[get_frm] Get free frame fail, page dirty and lookup fail!!!");
			//restore(ps); 
	//		return SYSERR;
	//	}
	/* 12.2 Write the page back to the backing store.*/
	write_bs((FRAME0 + i) * NBPG, bs_id, pageth);

	clear_frm_entry(i);
	
	//lDebug(DBG_FLOW, "free_frm done");
	
	return OK;
}

LOCAL int find_victim_frame()
{
	// TBD: pick a victim frame
	int policy = grpolicy();
	// frame is 100% full, no frame can be replaced and allocate new frame.
	if( fq[fqhead].qnext == fqtail ) return -1;

	switch(policy) 
	{ 
		case FIFO:
			return fifo_policy(); 
		default:
		case SC:
			return sc_policy();
	}

	return 0;
}

LOCAL int find_avail_frame(int* avail)
{
	int		i;
	for (i = 0; i < NFRAMES; i++) {
		if (frm_tab[i].fr_status == FRM_UNMAPPED ) 
		{
			*avail = i;
			return OK;
		}						        
	}
	return SYSERR;	
}

int get_page_entry(int fid, int pid, pt_t **pt_entry, pd_t **pd_entry) 
{
	virt_addr_t *va;
	unsigned int 	a;
	//unsigned int	p, q;

	/* 3. Using the inverted page table, 
	 * get vp, the virtual page number of the page to be replaced. */
	/* 4. Let a be vp*4096 (the first virtual address on page vp). */
	a = frm_tab[fid].fr_vpno << 12;
	va = &a;
	//lDebug(DBG_INFO,"[get_page_entry] va=0x%08x, va->pd_offset=%d, va->pt_offset=%d, va_pg_offset=%d",a, va->pd_offset, va->pt_offset, va->pg_offset);
	/* 5. Let p be the high 10 bits of a. Let q be bits [21:12] of a. */
	/* 6. Let pid be the pid of the process owning vp. */
	/* 7. Let pd point to the page directory of process pid. */
	*pd_entry = (pd_t*) (proctab[pid].pdbr + va->pd_offset * sizeof(pd_t) );
	
	if(!(*pd_entry)->pd_pres){ 
		//lDebug(DBG_ERR, "[get_page_entry] page table does not exist!!");
		return SYSERR;
	}

	/* 8. Let pt point to the pid’s p-th page table. */
	*pt_entry = (pt_t*) ( (*pd_entry)->pd_base * NBPG + va->pt_offset * sizeof(pt_t) );
	//lDebug(DBG_INFO, "[get_page_entry] pid(%d), pdbr=0x%08x, pd=0x%08x, pt=0x%08x",pid, proctab[pid].pdbr ,*pd_entry,*pt_entry);
	return OK;
}

void update_bs()
{
	int f;
	int proc_hold_frame;
	int frm_is_mapped;
	int frm_is_page;
	fr_map_t *fptr;

	for( f = 0 ; f < NFRAMES ; f++ ){
		fptr = &frm_tab[f];
		proc_hold_frame = fptr->fr_pid == currpid;
	   	frm_is_mapped = fptr->fr_status == FRM_MAPPED; 
		frm_is_page = fptr->fr_type == FR_PAGE;
		
		if(proc_hold_frame && frm_is_mapped && frm_is_page ){
			write_bs((char*)((FRAME0 + f) * NBPG), fptr->fr_bsid, fptr->fr_pageth);
		}
	}
}
void update_page()
{
	int f;
	int proc_hold_frame;
	int frm_is_mapped;
	int frm_is_page;
	fr_map_t *fptr;

	//lDebug(DBG_FLOW,"[INFO] enter update page. pid(%d)", currpid);	
	for( f = 0 ; f < NFRAMES ; f++ ){
		fptr = &frm_tab[f];
		proc_hold_frame = fptr->fr_pid == currpid;
	   	frm_is_mapped = fptr->fr_status == FRM_MAPPED; 
		frm_is_page = fptr->fr_type == FR_PAGE;
		
		if(proc_hold_frame && frm_is_mapped && frm_is_page ){
			read_bs((char*)((FRAME0 + f) * NBPG), fptr->fr_bsid, fptr->fr_pageth);
			//lDebug(DBG_INFO,"[update_page] read_bs, frm(%d), bsid(%d), pageth(%d), value(%c)", f,fptr->fr_bsid, fptr->fr_pageth, *((char*)((FRAME0 + f) * NBPG )));
		}
	}
}

void kill_proc_page(int _pid)
{
	int i;
	for( i = 0 ; i < NFRAMES ; i++ )
	{
		int bsid = frm_tab[i].fr_bsid;

		int fr_not_page = frm_tab[i].fr_type != FR_PAGE;
		int fr_is_unmapped = frm_tab[i].fr_status != FRM_MAPPED;
		int proc_doesnt_use_fr = frm_tab[i].fr_pid != _pid;
		int fr_map_bs = !isbad_bsid(bsid);

		//if( i == 8 ){
		//	kprintf("fr_status= %d, fr_bsid=%d\n",frm_tab[i].fr_status, frm_tab[i].fr_bsid );
			//kprintf();	
		//}
		if( fr_not_page || fr_is_unmapped || proc_doesnt_use_fr )	{ continue; }
		//info("[INFO][kill] pid(%d). frm_tab[%d].fr_type=%d",_pid, i, frm_tab[i].fr_type);
		//lDebug(DBG_INFO,"[kill] Frame (%d)", i);
		if(fr_map_bs){
			write_bs((FRAME0 + i) * NBPG, frm_tab[i].fr_bsid, frm_tab[i].fr_pageth);
		}
		clear_frm_entry(i);
		
		if( i == fqcurrpos ){
			fq_next_pos();
		}
		fq_delete(i);
	}// end for

}

void kill_proc_page_table(int _pid)
{
	pd_t *pd_entry = proctab[_pid].pdbr;
	
	int i;
	for( i = 0 ; i < (NBPG/4) ; i++, pd_entry++ )
	{
		if(i < 4) continue; // entry 0 ~ 3 is global page table, never delete!
		
		if( pd_entry->pd_pres )
		{
			int f = pd_entry->pd_base - FRAME0;
			//lDebug(DBG_INFO,"[INFO][kill] clear frame (%d), pd_base(%d)=%d ", f, i , pd_entry);

			clear_frm_entry(f);
		}
	}

}

void kill_proc_page_directroy(int _pid)
{
	int fid_dir = ( proctab[_pid].pdbr / NBPG ) - FRAME0;
	clear_frm_entry( fid_dir );
	//lDebug(DBG_INFO,"[INFO][kill] free pd frm (%d). ",fid_dir);
}
