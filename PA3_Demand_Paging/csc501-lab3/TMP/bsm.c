/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
#include <fq.h>
#include <Debug.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	STATWORD ps;
	int		i;

	disable(ps);
	for( i = 0 ; i < NBSM ; i++ )
	{
		free_bsm(i);
	}
	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	int		i;

	for( i = 0 ; i < NBSM ; i++ )
	{
		if( bsm_tab[i].bs_status == BSM_UNMAPPED )
		{
			*avail = i;
			return(OK);
		}
	}
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	int		j;

	if( isbad_bsid(i) )
	{
		return(SYSERR);
	}

	bsm_tab[i].bs_status 	= BSM_UNMAPPED;
	
	for( j = 0 ; j < NPROC ; j++ )
	{
		bsm_tab[i].bs_pid[j] 		= 0;
		bsm_tab[i].bs_npages[j] 	= 0;
		bsm_tab[i].bs_vpno[j] 		= 0;
	}
	bsm_tab[i].bs_sem 		= 0;
	bsm_tab[i].bs_private 	= 0;
	bsm_tab[i].bs_npages_max = 0;

	return(OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vpno, int* store, int* pageth)
{
	int		i;

	for( i = 0 ; i < NBSM ; i++ )
	{
		int		proc_can_access = bsm_tab[i].bs_pid[pid] == 1 || bsm_tab[i].bs_private == BSM_PUBLIC;
		int		bs_is_mapped 	= bsm_tab[i].bs_status == BSM_MAPPED;
		int		vp_match_bs		= check_vp_in_range( vpno, bsm_tab[i].bs_vpno[pid], bsm_tab[i].bs_npages[pid] );
		//lDebug(DBG_INFO,"proc(%d)_can_access(%d)=%d, bs_is_mapped=%d, vp(0x%08x)_match_bs(0x%08x,0x%08x)=%d",
		//				pid, proc_can_access,i, bs_is_mapped, vpno, bsm_tab[i].bs_vpno[pid], bsm_tab[i].bs_npages[pid], vp_match_bs);
		
		if( proc_can_access && bs_is_mapped && vp_match_bs )
		{
			*store = i;

			*pageth = vpno - bsm_tab[i].bs_vpno[pid];

			return(OK);
		}
	}
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	int		no_access;

	if(isbad_npage(npages) || isbad_bsid(source)) 
	{
		return SYSERR;
	}

	/* should not map to it's vheap twice or map to others vheap */
	int		map_to_vheap = (bsm_tab[source].bs_private == BSM_PRIVATE);
	
	if(map_to_vheap)
	{
		return SYSERR;
	}

	bsm_tab[source].bs_pid[pid] = 1;
	bsm_tab[source].bs_vpno[pid] = vpno;		        
	bsm_tab[source].bs_npages[pid] = npages;

	proctab[pid].bsm_num++;

	//lDebug(DBG_FLOW,"[INFO][bsm_map] pid(%d) map bs(%d), vpno=0x%08x, vpno_max=0x%08x, npages = %d",pid, source, vpno, vpno+npages, npages);

	return (OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int isPrivate)
{
	int		i, bs_id, pageth, npages;
	pd_t	*pd_entry;
	pt_t	*pt_entry;

	//lDebug(DBG_FLOW,"[INFO][bsm_unmap] enter unmap");
	if (bsm_lookup(pid, vpno, &bs_id, &pageth) == SYSERR)
	{
		//lDebug(DBG_ERR,"[INFO][bsm_unmap] fail lookup bsm ");
		return SYSERR;
	}
	//lDebug(DBG_INFO,"[INFO][bsm_unmap] start unmap");
   	vpno = bsm_tab[bs_id].bs_vpno[pid];
	npages = bsm_tab[bs_id].bs_npages[pid];
	//if (free_bsm(pid) == SYSERR)
	//{
	//	restore(ps);
	//	return SYSERR;	
	//}
	
	// TBD: isPrivate? if frames are private heap, do we need to write back?
	// TBD: which frame to write back? lets said xmmap(50), xmmap(100) and then xunmap(100). first 50 frame? last 50 frame?
	
	// 1. free frm or not? -> if only 1 process maps to this frame, we can free this frame.
	// 		if more process maps to this frame, it is possible other process's Page Table point to this frame!!!
	// 2. must delete this pid's page table entry because it is unmapped!
	// 		use frm_tab[].fr_bsid -> find vpno in bsm_tab[].bs_vpno[]
	// 		vpno has pd_offset & pt_offset + frm_tab[].fr_pageth
	 
	for( i = 0 ; i < NFRAMES ; i++ )
	{
		int	fr_is_mapped 	= frm_tab[i].fr_status == FRM_MAPPED;
		int	fr_is_paged 	= frm_tab[i].fr_type == FR_PAGE;
		int	fr_used_by_pid	= frm_tab[i].fr_pid == currpid;
		int fr_map_bsid		= frm_tab[i].fr_bsid == bs_id;
		int fr_map_proc		= frm_tab[i].fr_vpno >= 4096; // vpno < 4096 is physical page num, which is illegal
			
		//kprintf("[INFO][bsm_unmap] frm(%d), fr_is_mapped(%1d), fr_is_paged(%1d), fr_used_by_pid(%1d), fr_in_range(frm_tab[i].fr_vpno(0x%08x),vpno(0x%08x),npages(0x%08x))=%d ",
		//				i, fr_is_mapped, fr_is_paged, fr_used_by_pid, frm_tab[i].fr_vpno, vpno, npages,fr_in_range);

		if( fr_is_mapped && fr_is_paged && fr_used_by_pid && fr_map_proc && fr_map_bsid )
		{
			write_bs( (FRAME0 + i) * NBPG, bs_id, frm_tab[i].fr_pageth);

			//lDebug(DBG_INFO,"[INFO][bsm_unmap] free_frm(%d)",i);
				
			free_frm( i );
				
			if( i == fqcurrpos ){ fq_next_pos(); }
			
			fq_delete(i);
		}
	}// end for i

	bsm_tab[bs_id].bs_pid[pid] = 0; // process still own this bs
	bsm_tab[bs_id].bs_vpno[pid] = 0; 
	bsm_tab[bs_id].bs_npages[pid] = 0;

	proctab[pid].bsm_num--;

	return (OK);
}

void kill_proc_bsm(int _pid)
{
	int i;
	//if( isbadpid(_pid) ){ return; }

	//lDebug(DBG_INFO, "[INFO] enter kill_proc_bsm, pid= %d",_pid);
	
	for( i = 0 ; i < NBSM ; i++ )
	{
		int		proc_hold_bs 	= bsm_tab[i].bs_pid[_pid] == 1;
		int		bs_is_mapped 	= bsm_tab[i].bs_status == BSM_MAPPED;

		//lDebug(DBG_INFO, "[INFO] bsm_tab[i].bs_pid[%d]= %d",_pid, bsm_tab[i].bs_pid[_pid]);
		
		if(proc_hold_bs && bs_is_mapped)
		{
			bsm_tab[i].bs_pid[_pid] = 0; // process still own this bs
			bsm_tab[i].bs_vpno[_pid] = 0; 
			bsm_tab[i].bs_npages[_pid] = 0;
		}
	}
	proctab[_pid].bsm_num = 0;

	return;
}
/*
int bsm_map_refcnt( int bsid ) // how many process map this backing store
{
	int i, sum=0;
	for(i=0;i<NPROC;i++)
	{
		if( bsm_tab[bsid].bs_pid[i] == 1 ) sum++;	
	}
	return sum;
}
*/
/*
int frm_map_refcnt( int f ) // how many process map this frame
{
	int bsid, i, sum=0;
	bsid = frm_tab[f].fr_bsid;

	for(i=0;i<NPROC;i++)
	{
		int proc_map_bs = bsm_tab[bsid].bs_pid[i] == 1;
	   	int pageth_in_range = bsm_tab[bsid].bs_npages[i] > frm_tab[f].fr_pageth;
	   	
		if( proc_map_bs && pageth_in_range ) sum++;
	}
	return sum;
}
*/
