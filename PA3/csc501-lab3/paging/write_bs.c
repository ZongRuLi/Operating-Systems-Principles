#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mark.h>
#include <bufpool.h>
#include <paging.h>
#include <Debug.h>

int write_bs(char *src, bsd_t bs_id, int page) {

  /* write one page of data from src
     to the backing store bs_id, page
     page.
  */
   char * phy_addr = BACKING_STORE_BASE + bs_id*BACKING_STORE_UNIT_SIZE + page*NBPG;
   //if( ((unsigned int)src)>>12 < 1024 ){
   //		lDebug(DBG_ERR,"[ERROR][write_bs] invalid src!!");
	//	return SYSERR;
   //}
   //if( isbad_bsid(bs_id) || page < 0 || page >= 128 ){
   //		lDebug(DBG_ERR,"[ERROR][write_bs] illegal bs(%d) or page(%d)",bs_id, page);
//		return SYSERR;
  // }
   bcopy((void*)src, phy_addr, NBPG);

}

