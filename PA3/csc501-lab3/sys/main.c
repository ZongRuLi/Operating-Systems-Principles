/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>
#include <proc.h>
#include <Debug.h>

#define PROC1_VADDR 0x40000000
#define PROC1_VPNO 0x40000
#define PROC2_VADDR 0x80000000
#define PROC2_VPNO 0x80000
#define TEST1_BS 1

void print_frame_status(int num){
	int i;
	int f_per_line = 4;
	char mapp;
	char *type;

	for( i = 0; i< num && i < NFRAMES; i++)
	{
		mapp = (frm_tab[i].fr_status == FRM_MAPPED)? 'M' : 'X';
		type = (frm_tab[i].fr_type == FR_PAGE && mapp == 'M' )? "PG" :
				(frm_tab[i].fr_type == FR_TBL )? "PT": 
				(frm_tab[i].fr_type == FR_DIR )? "PD" : "XX";
		kprintf(" [f%2d,%s,%2d]",i, type, frm_tab[i].fr_pid);
		if( i % f_per_line == f_per_line -1 ) kprintf("\n");
	}
}

void proc0_test0(char *msg, int lck)
{
	kprintf("\n- [TEST][0.0] xmmap without get backing store\n");
	int BSID0 = 14;
	release_bs( BSID0 );

	if (xmmap(PROC1_VPNO, BSID0, 100) == SYSERR)
		kprintf("- PASS TEST 0.0 PASS\n");
	else
		kprintf("- [ERROR] test 0.0 failed!\n");
}
void proc1_test0(char *msg, int lck)
{
	kprintf("\n- [TEST][0.1] vgetmem size more than vcreate's hsize\n");
	
	if( vgetmem(5000) == SYSERR )
		kprintf("- PASS TEST 0.2 PASS\n");
	else
		kprintf("- [ERROR] test 0.1 failed!\n");
}
void proc2_test0(char *msg, int lck)
{
	int BSID0 = 14;
	kprintf("\n- [TEST][0.2] xmmap virtual address to vheap. \n");
	
	get_bs( BSID0 ,128);	
	
	if( bsm_tab[0].bs_status != BSM_MAPPED ){
		kprintf("[ERROR] try other bs id\n");
	}
	
	if (xmmap( 4096 , TEST1_BS, 100) == SYSERR)
		kprintf("- PASS TEST 0.2 PASS\n");
	else
		kprintf("- [ERROR] test 0.2 failed!\n");
	
	release_bs( BSID0 );
	return;
}
void proc3_test0(char *msg, int lck)
{
	int BSID0 = 14;
	int BSID1 = 15;

	kprintf("\n- [TEST][0.3] xmmap overlap previous xmmap \n");
	get_bs( BSID0 ,128);	
	get_bs( BSID1 ,128);	

	if (xmmap(PROC1_VPNO, BSID0, 100) == SYSERR)
	{
		kprintf("- xmmap call failed\n");
		sleep(3);
		return;
	}
	if (xmmap(PROC1_VPNO + 50, BSID1, 100) == SYSERR)
		kprintf("- PASS TEST 0.3 PASS\n");
	else
		kprintf("- [ERROR] test 0.3 failed!\n");
	
	release_bs( BSID0 );
	release_bs( BSID1 );
	return;
}

void proc1_test1(char *msg, int lck)
{
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR)
	{
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	addr = (char *)PROC1_VADDR;
	for (i = 0; i < 26; i++)
	{
		*(addr + i * NBPG) = 'A' + i;
	}

	sleep(6);

	for (i = 0; i < 26; i++)
	{
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	xmunmap(PROC1_VPNO);
	//proc1_test4("tt",0);
	return;
}

void proc1_test2(char *msg, int lck)
{
	int *x;

	kprintf("- ready to allocate heap space\n");
	x = vgetmem(1024);
	kprintf("- heap allocated at %x\n", x);
	*x = 100;
	*(x + 1) = 200;

	kprintf("heap variable: %d %d\n", *x, *(x + 1));
	vfreemem(x, 1024);
	
	//kprintf("- test freed heap variable(illegal), should page fault & kill process\n");

	//kprintf("illegal heap variable x: %d\n", *x );
	//kprintf("[ERROR] TEST 2 FAIL!!!!\n");

}

void proc1_test3(char *msg, int lck)
{
	char *addr;
	int i;

	get_bs(TEST1_BS, 100);

	if (xmmap(PROC1_VPNO, TEST1_BS, 100) == SYSERR)
	{
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	addr = (char *)PROC1_VADDR;
	for (i = 0; i < 26; i++)
	{
		kprintf("[proc1_test4] addr = 0x%08x\n",(addr + i * NBPG));
		*(addr + i * NBPG) = 'Z' - i;
	}
	
	kprintf("-proc1 write all data in bs, sleep 6s.\n\n");
	//print_frame_status(100);
	sleep(10);

	kprintf("-proc1 resume.\n\n");
	//if(debugLevel >= DBG_ERR ){ print_frame_status(20); }
	
	kprintf("-print directly from bsm.\n\n");
	addr = (char*) ( (2048 + TEST1_BS * 128 ) * NBPG );
	for (i = 0; i < 26; i++)
	{
		kprintf("0x%08x: %c, expect(%c)\n", addr + i * NBPG, *(addr + i * NBPG), 'A'+i );	
	}
	kprintf("-print from virtual address\n\n");
	addr = (char *)PROC1_VADDR;
	for (i = 0; i < 26; i++)
	{
		kprintf("0x%08x: %c, expect(%c)\n", addr + i * NBPG, *(addr + i * NBPG), 'A'+i );
	}

	xmunmap(PROC1_VPNO);
	return;
}
void proc2_test3(char *msg, int lck)
{
	char *addr;
	int i;

	//get_bs(TEST1_BS, 100);
	//if(debugLevel >= DBG_ERR ){ print_frame_status(20); }
	kprintf("-proc2 start. Expect read out Z -> A.\n\n");

	if (xmmap(PROC2_VPNO, TEST1_BS, 100) == SYSERR)
	{
		kprintf("xmmap call failed\n");
		sleep(3);
		return;
	}

	addr = (char *)PROC2_VADDR;
	for (i = 0; i < 26; i++)
	{
		kprintf("0x%08x: %c, expect(%c)\n", addr + i * NBPG, *(addr + i * NBPG), 'Z'-i );
		*(addr + i * NBPG) = 'A' + i;
	}

	xmunmap(PROC2_VPNO);
	return;
}

void proc_replacement_test(int _policy)
{
	char* replace_policy;
	int *addr, *var;
	int i;
	int bsid;
	int total_bsid = 5;
	int npage = 3;
	int seq_num = 20;
	int max_vpno = 12;
	int vpno_seq[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11, 12, 1,3,5, 0, 2, 4, 1};
		// inserted f :  8, 9,10,11,12,13,14,15,16,17,18,19,  
		// FIFO replacement policy:
		// replaced f during wr:							  8, H,H,H, 9, H, H,10
		// result f:    12, 0, 1, 3, 4, 5, 6, 7, 8, 9,10,11
		// replaced f during rd: [f11,rd(2)]
		
		// SC replacement policy:
		// inserted f :  8, 9,10,11,12,13,14,15,16,17,18,19,  
		// replaced f :										  8, H,H,H,10,12,14, H
		// result f:    12, 1, 0, 3, 2, 4, 6, 7, 8, 9,10,11
		// replace f during rd: [f11,rd(6)], [f15, rd(3)]
/*
 * [frame #, frame type, owner process]
 [f 0,PT, 0] [f 1,PT, 0] [f 2,PT, 0] [f 3,PT, 0]
 [f 4,PD, 0] [f 5,PD,49] [f 6,XX,-1] [f 7,XX,-1]
 [f 8,XX,-1] [f 9,XX,-1] [f10,XX,-1] [f11,XX,-1]
 [f12,XX,-1] [f13,XX,-1] [f14,XX,-1] [f15,XX,-1]
 [f16,XX,-1] [f17,XX,-1] [f18,XX,-1] [f19,XX,-1]
 proc(47) will create [f 6,PD,47] [f 7,PT,47], 12 frame left availble.
*/
	srpolicy(_policy);
	
	if(_policy == FIFO) kprintf("test FIFO policy\n");
	else if(_policy == SC) kprintf("test SC policy\n");
	else{ kprintf("[ERROR]test undefined policy!!!!\n");
		return;
	}

	for(bsid =0; bsid < total_bsid ; bsid ++ )
	{
		get_bs( bsid , 100 );

		if (xmmap(PROC1_VPNO + (npage * bsid), bsid, npage ) == SYSERR)
		{
			kprintf("xmmap call failed\n");
			sleep(3);
			return;
		}
	}
	addr = (int *)(PROC1_VADDR);
	for (i = 0; i < seq_num; i++)
	{
		var = addr + (vpno_seq[i] * NBPG)/4;
		*(var) = 1000 * (vpno_seq[i]/npage) + vpno_seq[i];
		kprintf("0x%08x: %d\n", var, *(var));
	}
	
	for (i = max_vpno; i >= 0; i--)
	{
		var = addr + (i*NBPG)/4;
		kprintf("0x%08x: %d\n", var, *(var));
	}
	
	for(bsid =0; bsid < total_bsid ; bsid ++ ){
		xmunmap(PROC1_VPNO + npage * bsid );
	}
	return;
}

void proc1_test6(char *msg, int lck)
{

	char *addr;
	int i;
	
	addr = (char *)0x0;
	for (i = 0; i < 100; i++)
	{
		*(addr + i * NBPG) = 'B';
	}

	for (i = 0; i < 100; i++)
	{
		kprintf("0x%08x: %c\n", addr + i * NBPG, *(addr + i * NBPG));
	}

	return;
}
/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main(int argc, const char* argv[])
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
	int pid1;
	int pid2;

	//if(debugLevel >= DBG_ERR ){ print_frame_status(100); }
	kprintf("\n0: illegal use cases\n");
	pid1 = create(proc0_test0, 2000, 20, "proc0_test0", 0, NULL);
	resume(pid1);
	sleep(1);
	pid1 = vcreate(proc1_test0, 2000, 1, 20, "proc1_test0", 0, NULL);
	resume(pid1);
	sleep(1);
	pid1 = vcreate(proc2_test0, 2000, 100, 20, "proc2_test0", 0, NULL);
	resume(pid1);
	sleep(1);
	pid1 = create(proc3_test0, 2000, 20, "proc3_test0", 0, NULL);
	resume(pid1);
	sleep(1);

	kprintf("\n1: shared memory\n");
	pid1 = create(proc1_test1, 2000, 20, "proc1_test1", 0, NULL);
	resume(pid1);
	sleep(10);
	if(debugLevel >= DBG_ERR ){ print_frame_status(100); }
	
	kprintf("\n2: vgetmem/vfreemem\n");
	pid1 = vcreate(proc1_test2, 2000, 100, 20, "proc1_test2", 0, NULL);
	kprintf("pid %d has private heap\n", pid1);
	resume(pid1);
	sleep(3);
	if(debugLevel >= DBG_ERR ){ print_frame_status(20); }
	
	//if(debugLevel >= DBG_ERR ){ print_frame_status(100); }

	kprintf("\n3: shared memory between processes test,\n\
Expect proc2 read out the same conetent proc1 write into bs.\n");
	pid1 = create(proc1_test3, 2000, 21, "proc1_test3", 0, NULL);
	pid2 = create(proc2_test3, 2000, 22, "proc2_test3", 0, NULL);
	resume(pid1);
	
	while(proctab[pid1].pstate != PRSLEEP){
		kprintf("state=%d\n", proctab[pid1].pstate);
	};
	resume(pid2);
	sleep(10);
	//if(debugLevel >= DBG_ERR ){ print_frame_status(100); }
	
	
	//force_frame_only_avail(20);
	// test SC
	if( NFRAMES == 20 )
	{
		
		kprintf("\n4: Replacement policy SC test\n");
		pid1 = create(proc_replacement_test, 2000, 20, "proc_replacement_test", 1, SC);
		resume(pid1);
		sleep(3);
		if(debugLevel >= DBG_ERR ){ 
				print_frame_status(20); 
				fq_print();
		}
		// test FIFO
		kprintf("\n5: Replacement policy FIFO test\n");
		//srpolicy(FIFO);
		pid1 = create(proc_replacement_test, 2000, 20, "proc_replacement_test", 1, FIFO);
		resume(pid1);
		sleep(3);
		if(debugLevel >= DBG_ERR ){ print_frame_status(20); }
	}
		
	kprintf("\n6: Frame test\n");
	pid1 = create(proc1_test6, 2000, 20, "proc1_test6", 0, NULL);
	resume(pid1);
	sleep(3);
	
	kprintf("ALL TEST FINISH!\n");
	        
	/* The hook to shutdown QEMU for process-like execution of XINU.
	 * This API call terminates the QEMU process.
	 */    
	shutdown();
}

