#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <Debug.h>

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
	int		access_vheap 	= (bsm_tab[bs_id].bs_private == BSM_PRIVATE);

	disable(ps);

	if( isbad_npage(npages) || isbad_bsid(bs_id) || access_vheap )
	{
		restore(ps);				        
		return SYSERR;
	}

	if(bs_is_unmapped)
	{
		//bsm_map(currpid, 0, bs_id, npages);// TBD: ??
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
		bsm_tab[bs_id].bs_npages_max = npages;
	}
	
	lDebug(DBG_FLOW,"get_bs: npages=%d, bsm_tab.bs_npages=%d", npages, bsm_tab[bs_id].bs_npages[currpid]);

	restore(ps);
	return bsm_tab[bs_id].bs_npages;

}
