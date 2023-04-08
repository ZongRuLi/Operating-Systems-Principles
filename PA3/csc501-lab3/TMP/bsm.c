/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

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
	STATWORD ps;
	int		i;

	disable(ps);
	for( i = 0 ; i < NBSM ; i++ )
	{
		if( bsm_tab[i].bs_status == BSM_UNMAPPED )
		{
			*avail = i;
			restore(ps);
			return(OK);
		}
	}
	restore(ps);
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	STATWORD ps;
	disable(ps);

	if( isbad_bsid(i) )
	{
		restore(ps);
		return(SYSERR);
	}

	bsm_tab[i].bs_status 	= BSM_UNMAPPED;
	bsm_tab[i].bs_pid 		= -1;
	bsm_tab[i].bs_npages 	= 0;
	bsm_tab[i].bs_sem 		= 0;
	bsm_tab[i].bs_vpno 		= 4096;

	restore(ps);
	return(OK);
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	int		i;
	disable(ps);
	for( i = 0 ; i < NBSM ; i++ )
	{
		if( bsm_tab[i].bs_status == BSM_MAPPED && bsm_tab[i].bs_pid == pid )
		{
			*store = i;

			*pageth = (vaddr / NBPG) - bsm_tab[i].bs_vpno;

			restore(ps);
			return(OK);
		}
	}
	restore(ps);
	return(SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	STATWORD ps;
	int		no_access;

	disable(ps);

	if(isbad_npage(npages) || isbad_bsid(source)) 
	{
		restore(ps);
		return SYSERR;
	}

	no_access = (bsm_tab[source].bs_private == BSM_PRIVATE) && (bsm_tab[source].bs_pid != pid);
	
	if(no_access)
	{
		restore(ps);
		return SYSERR;
	}

	bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_pid = pid;		 
	bsm_tab[source].bs_vpno = vpno;				        
	bsm_tab[source].bs_npages = npages;
						        
	/* set proctab */						        
	proctab[currpid].vhpno = vpno;								        
	proctab[currpid].store = source;

	restore(ps);
	return (OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
	int		i, bs_id, pageth;

	disable(ps);
	if (bsm_lookup(pid, vpno, &bs_id, &pageth) == SYSERR)
	{
		restore(ps);
		return SYSERR;	
	}

	if (free_bsm(pid) == SYSERR)
	{
		restore(ps);
		return SYSERR;	
	}
	
	for( i = 0 ; i < NFRAMES ; i++ )
	{
		int	fr_is_mapped 	= frm_tab[i].fr_status == FRM_MAPPED;
		int	fr_is_paged 	= frm_tab[i].fr_type == FR_PAGE;
		int	fr_used_by_pid	= frm_tab[i].fr_pid == pid;
		
		if( fr_is_mapped && fr_is_paged && fr_used_by_pid )
		{
			write_bs((i + NFRAMES) * NBPG, bs_id, pageth);
			break;
		}
	}
	restore(ps);
	return (OK);
}


