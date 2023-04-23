/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <Debug.h>

LOCAL int check_vp_no_overlap(int virtpage);
/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 * Much like its Unix counterpart (see man mmap), it maps a source file 
 * (“backing store” here) of size npages pages to the virtual page virtpage. 
 * A process may call this multiple times to map data structures, code, etc.
 */
/* This call simply creates an entry in the backing store mapping */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	//kprintf("xmmap - to be implemented!\n");
	STATWORD ps;

	disable(ps);

	int		bad_bs_id				= isbad_bsid((int)source);
	int		bad_npage				= isbad_npage(npages);
	int		vp_in_physical 			= virtpage < 4096; // TBD: define VM base as 4096 
	// DONE: should prevent virtpage inside vheap!!!
	// TBD: how about no overlap with other files?
	
	int		bs_is_unmapped 			= bsm_tab[source].bs_status == BSM_UNMAPPED;
	int		bs_map_to_vheap			= bsm_tab[source].bs_private == BSM_PRIVATE;
	int		file_larger_than_bs		= bsm_tab[source].bs_npages_max < npages;
	int		bs_already_open_files 	= bsm_tab[source].bs_vpno[currpid] != 0;
	int		vp_overlapped			= check_vp_no_overlap( virtpage ) == SYSERR;

	int		param_range_error 		= bad_bs_id || bad_npage || vp_in_physical || vp_overlapped;
	int		bs_unavailable 			= bs_is_unmapped || bs_map_to_vheap || file_larger_than_bs || bs_already_open_files;

	if( param_range_error || bs_unavailable )
	{
  		restore(ps);
  		return SYSERR;
	}

	bsm_map( currpid, virtpage, source, npages);

  	restore(ps);
  	return(OK);
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 * This call, like munmap, should remove a virtual memory mapping. 
 * See man munmap for the details of the Unix call.
 */
SYSCALL xmunmap(int virtpage)
{
	//kprintf("To be implemented!");
	STATWORD ps;
 	
	disable(ps);
  
	if( bsm_unmap(currpid, virtpage, 0) == SYSERR )
	{
  		restore(ps);
  		return SYSERR;
	}

  	restore(ps);
  	return(OK);
}


LOCAL int check_vp_no_overlap(int virtpage)
{
	struct	pentry	*pptr;
	int				i;
	int				proc_has_vheap;
	int				virtpage_in_vheap; 
	
	pptr = &proctab[currpid];
	
	proc_has_vheap = !isbad_bsid(pptr->store);
	virtpage_in_vheap = check_vp_in_range( virtpage, pptr->vhpno, pptr->vhpnpages ); 

	if( proc_has_vheap && virtpage_in_vheap )
		   return SYSERR;

	for( i = 0 ; i < NBSM ; i++ )
	{
		if(bsm_tab[i].bs_pid[currpid] == 1 )
		{
			if( check_vp_in_range( virtpage, bsm_tab[i].bs_vpno[currpid], bsm_tab[i].bs_npages[currpid] ) )
				return SYSERR;	
		}
	}
	return OK;
}

int check_vp_in_range(int virtpage, int vpno, int npages)
{
	return ( (virtpage >= vpno) && (virtpage < (vpno + npages)) );
}
