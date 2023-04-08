#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * get_bs - requests a new backing store with ID store of size npages (in pages, not bytes). 
 *-------------------------------------------------------------------------
 * If a new backing store can be created, or a backing store with 
 * this ID already exists, the size of the new or existing backing store 
 * is returned. This size is in pages. If a size of 0 is requested, 
 * or the creation encounters an error, SYSERR should be returned. 
 * Also for practical reasons, npages should be no more than 128.
 */
int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
    //kprintf("To be implemented!\n");
    STATWORD ps;
	int		bs_is_unmapped 	= (bsm_tab[bs_id].bs_status == BSM_UNMAPPED);
	int		bs_is_private 	= (bsm_tab[bs_id].bs_private == BSM_PRIVATE);

	disable(ps);

	if( isbad_npage(npages) || isbad_bsid(bs_id) )
	{	
		restore(ps);				        
		return SYSERR;
	}

	if(bs_is_unmapped)
	{
		int bs_map_fail = bsm_map(currpid, 4096, bs_id, npages) == SYSERR;
	
		if( bs_map_fail ){
			restore(ps);
			return SYSERR;
		}
	}
	// TBD: check definition & use case for private heap
	if(bs_is_private )//&& bsm_tab[bs_id].bs_pid != currpid )
	{
			restore(ps);
			return SYSERR;
	}
	// 1. mapped & public
	// 2. mapped & private & allow access
	
	kprintf("get_bs: npages=%d, bsm_tab.bs_npages=%d\n", npages, bsm_tab[bs_id].bs_npages);

	restore(ps);
	return bsm_tab[bs_id].bs_npages;

}


