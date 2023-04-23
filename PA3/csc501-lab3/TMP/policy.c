/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <fq.h>
#include <Debug.h>

extern int page_replace_policy;

/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
	/* sanity check ! */

	//kprintf("To be implemented!\n");
	if( (policy == SC) && (page_replace_policy != policy) )
	{
		fqcurrpos = ( fq[fqhead].qnext == fqtail ) ? -1 : fq[fqhead].qnext ;
	}
	
	page_replace_policy = policy;
	
	print_replace = 1;

	return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}

// return victim frame using fifo policy
int fifo_policy() 
{
	int victim = fq_pop_front();

	return victim;
}

// return victim frame using SC policy
int sc_policy() 
{
	int 	i,j;
	pt_t 	*pt_entry;
	pd_t 	*pd_entry;

	while(1)
	{
		int fid = fqcurrpos;

		int pid = frm_tab[fid].fr_pid;
		int vpno = frm_tab[fid].fr_vpno;
		
		int isbad_fid = fid < 0 || fid >= 1024;
	   	int frm_map_proc = vpno >= 4096;
		int frm_unmapped = frm_tab[fid].fr_status == FRM_UNMAPPED;

		//lDebug(DBG_FLOW,"[sc_policy] enter sc_policy, current position = frm(%d), vpno = 0x%08x", fqcurrpos, vpno);

		if( isbadpid(pid) || isbad_fid || !frm_map_proc || frm_unmapped ){
			//lDebug(DBG_ERR, "[ERROR][sc_policy] frame(%d) inside circular queue is illegal!",fid);
			fq_next_pos();
			fq_delete(fid);
			continue;
		}
		
		if( get_page_entry(fid, pid, &pt_entry, &pd_entry) == SYSERR )
		{
			//lDebug(DBG_ERR, "[ERROR][sc_policy] Can't find page entry of frame (%d) pid(%d)", fid, pid);
			// if can't find in PT, treat this frame as no longer needed.
			fq_next_pos();
			fq_delete(fid);
			return fid;
		}

		int ref_bit_is_not_set = pt_entry->pt_acc == 0;

		//lDebug(DBG_INFO,"[INFO] fid(%d) pt_acc = %d",fid, pt_entry->pt_acc);

		if(ref_bit_is_not_set)
		{
			fq_next_pos();
			fq_delete(fid);
			return fid;
		}
		pt_entry->pt_acc = 0;
		//lDebug(DBG_INFO,"[INFO] page table 0x%08x -> pt_acc = %d",pt_entry, pt_entry->pt_acc);
		fq_next_pos();
		
	}// end while

	return SYSERR;
}
