/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 * Much like its Unix counterpart (see man mmap), it maps a source file 
 * (“backing store” here) of size npages pages to the virtual page virtpage. 
 * A process may call this multiple times to map data structures, code, etc.
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
	//kprintf("xmmap - to be implemented!\n");
	STATWORD ps;
	int		fail;

	disable(ps);

	int		bs_is_unmapped 	= bsm_tab[source].bs_status == BSM_UNMAPPED;
	int		bs_is_private	= bsm_tab[source].bs_private == BSM_PRIVATE;
	int		bad_bs_id		= isbad_bsid((int)source);
	int		bad_npage		= isbad_npage(npages);
	int		illegal_virtpage = virtpage < 4096; // TBD: define VM base as 4096 

	fail = bs_is_unmapped || bs_is_private || bad_bs_id || bad_npage || illegal_virtpage;

	if(fail)
	{
  		restore(ps);
  		return SYSERR;
	}

	if( bsm_map(currpid, virtpage, source, npages) == SYSERR )
	{
  		restore(ps);
  		return SYSERR;
	}

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
