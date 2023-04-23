/* paging.h */

#define NPPBS		128		/* number of pages per backing stores */
typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  int bs_status;			/* MAPPED or UNMAPPED		*/
  int bs_pid[NPROC];				/* process id using this slot   */
  int bs_vpno[NPROC];				/* starting virtual page number */
  int bs_npages[NPROC];			/* number of pages in the store */
 // int bs_frm[NPPBS];		/* frames contain current bs page */
  int bs_sem;				/* semaphore mechanism ?	*/
  int bs_private;			/* PRIVATE heap or PUBLIC */
  int bs_npages_max;		/* number of pages in the store */
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
  int fr_bsid;				/* bs mapping of this PAGE frame */
  int fr_pageth;			/* pageth in bs */
}fr_map_t;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
//#define NFRAMES 	1024	/* number of frames		*/
#define NFRAMES 	1024	/* number of frames		*/
#define NBSM	    16		/* number of backing stores	*/

#define isbad_bsid(x) 			( x >= NBSM || x < 0 )

#define BSM_UNMAPPED	0
#define BSM_MAPPED		1

#define BSM_PUBLIC 		0
#define BSM_PRIVATE 	1

#define FRM_UNMAPPED	0
#define FRM_MAPPED		1

#define FR_PAGE			0
#define FR_TBL			1
#define FR_DIR			2

#define SC 				3
#define FIFO 			4

#define BACKING_STORE_BASE		0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000
#define BS_NPAGE_MAX 			(BACKING_STORE_UNIT_SIZE/NBPG) 	/* number of pages per backing store id. 0x80000/4096 =128 */

#define isbad_npage(x) 			( x > BS_NPAGE_MAX || x <= 0 )

extern int 		print_replace;
extern int		fid_global_pt[4];
extern const pt_t clear_pt_entry;
extern const pd_t clear_pd_entry;
extern int		vcheckmem_bypass;

extern SYSCALL bsm_map(int pid, int vpno, int source, int npages);
extern SYSCALL init_bsm();
extern SYSCALL init_frm();
extern SYSCALL get_frm(int* avail);
extern void write_cr3(unsigned long n);
extern void enable_paging();

extern int check_va_legal(unsigned long va);
extern int check_vp_in_range(int virtpage, int vpno, int npages);
extern int bsm_map_refcnt( int bsid ); 
extern int get_page_entry(int fid, int pid, pt_t **pt_entry, pd_t **pd_entry); 

extern void update_bs();
extern void update_page();

extern void kill_proc_page(int pid);
extern void kill_proc_page_table(int pid);
extern void kill_proc_page_directroy(int pid);
extern void kill_proc_bsm(int pid);
extern void release_vheap(int pid);
//#define DETAIL 1
#define DBG_LV 0

#define DBG_ERR	 1	
#define DBG_FLOW 2
#define DBG_WARN 3
#define DBG_INFO 4	
/*
 * debugLevel use case:
 * 	level 0 : PA3 defined must-printed error message, please use kprintf directly
 * 	level 1 : personal defined error message
 * 	level 2 : debug message
 * 	level 3 : warning message // rarely used
 * 	level 4 : info message
 * */
