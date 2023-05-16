/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
#define TPASSED 1
#define TFAILED 0

#define MYVADDR1 0x40000000
#define MYVPNO1 0x40000
#define MYVADDR2 0x80000000
#define MYVPNO2 0x80000
#define MYBS1 1
#define MAX_BSTORE 16

#ifndef NBPG
#define NBPG 4096
#endif
#ifndef MAXNPG
#define MAXNPG 128
#endif
#define assert(x, error) \
	if (!(x))            \
	{                    \
		kprintf(error);  \
		return;          \
	}


/*----------------------------------------------------------------*/
void proc_test2(int i, int j, int *ret, int s)
{
	char *addr;
	int bsize;
	int r;

	bsize = get_bs(i, 50);
	kprintf("requested %d, returned %d\n", j, bsize);

	if (bsize != 50)
	{
		*ret = TFAILED;
	}
	r = xmmap(MYVPNO1, i, j);
	if (j <= 50 && r == SYSERR)
	{
		*ret = TFAILED;
	}
	if (j > 50 && r != SYSERR)
	{
		*ret = TFAILED;
	}
	kprintf("-j=%d, r=%d, ret=%d\n",j,r,*ret);
	sleep(s);
	if (r != SYSERR)
		xmunmap(MYVPNO1);
	release_bs(i);
	return;
}

void test2()
{
	int pids[MAX_BSTORE];
	int mypid;
	int i, j;

	int ret = TPASSED;
	kprintf("\nTest 2: Testing backing store operations\n");

	mypid = create(proc_test2, 2000, 20, "proc_test2", 4, 1, 50, &ret, 4);
	resume(mypid);
	sleep(2);
	for (i = 1; i <= 5; i++)
	{
		pids[i] = create(proc_test2, 2000, 20, "proc_test2", 4, 1, i * 20, &ret, 0);
		resume(pids[i]);
	}
	sleep(3);
	kill(mypid);
	for (i = 1; i <= 5; i++)
	{
		kill(pids[i]);
	}
	if (ret != TPASSED)
		kprintf("\tFAILED!\n");
	else
		kprintf("\tPASSED!\n");
}

/*-------------------------------------------------------------------------------------*/
void test_func7()
{
	int PAGE0 = 0x40000;
	int i, j, temp;
	int addrs[1200];
	int cnt = 0;
	// can go up to  (NFRAMES - 5 frames for null prc - 1pd for main - 1pd + 1pt frames for this proc)
	// frame for pages will be from 1032-2047
	int maxpage = (NFRAMES - (5 + 1 + 1 + 1));
	// int maxpage = (NFRAMES - 25);

	for (i = 0; i <= maxpage / 120; i++)
	{
		if (get_bs(i, 120) == SYSERR)
		{
			kprintf("get_bs call failed \n");
			return;
		}
		if (xmmap(PAGE0 + i * 120, i, 120) == SYSERR)
		{
			kprintf("xmmap call failed\n");
			return;
		}
		for (j = 0; j < 120; j++)
		{
			// store the virtual addresses
			addrs[cnt++] = (PAGE0 + (i * 120) + j) << 12;
		}
	}

	/* all of these should generate page fault, no page replacement yet
	   acquire all free frames, starting from 1032 to 2047, lower frames are acquired first
	   */
	for (i = 0; i < maxpage; i++)
	{
		*((int *)addrs[i]) = i + 1;
	}

	// trigger page replacement, this should clear all access bits of all pages
	// expected output: frame 1032 will be swapped out
	kprintf("\n\t 7.1 Expected replaced frame: 1032\n\t ");
	*((int *)addrs[maxpage]) = maxpage + 1;
	// kprintf("%d", *((int *)addrs[maxpage]));
	//  int adr1 = (frm_tab[609].fr_vpno * NBPG);
	//  virt_addr_t vaddress1 = *(virt_addr_t *)&adr1;
	//  int pg_no1 = vaddress1.pt_offset;
	//  int pt_no1 = vaddress1.pd_offset;
	//  pd_t *pde1 = (pd_t *)(proctab[frm_tab[609].fr_pid].pdbr + pt_no1 * sizeof(pd_t));
	//  pt_t *pte1 = (pt_t *)(pde1->pd_base * NBPG + pg_no1 * sizeof(pt_t));
	for (i = 1; i <= maxpage; i++)
	{
		// kprintf("%d\n",pte1->pt_acc);
		if ((i != 600) && (i != 800)) // reset access bits of all pages except these
			*((int *)addrs[i]) = i + 1;
	}
	// kprintf("\n%d\n", *((int *)addrs[601]));
	// Expected page to be swapped: 1032+600 = 1632
	kprintf("\n\t 7.2 Expected replaced frame: 1632\n\t");
	*((int *)addrs[maxpage + 1]) = maxpage + 2;
	temp = *((int *)addrs[maxpage + 1]);
	if (temp != maxpage + 2)
		kprintf("\tFAILED!\n");

	kprintf("\n\t 7.3 Expected replaced frame: 1832\n\t");
	*((int *)addrs[maxpage + 2]) = maxpage + 3;
	temp = *((int *)addrs[maxpage + 2]);
	if (temp != maxpage + 3)
		kprintf("\tFAILED!\n");

	for (i = 0; i <= maxpage / 120; i++)
	{
		xmunmap(PAGE0 + (i * 120));
		release_bs(i);
	}
}
void test7()
{
	int pid1;
	int ret = TPASSED;

	kprintf("\nTest 7: Test SC page replacement policy\n");
	srpolicy(SC);
	pid1 = create(test_func7, 2000, 20, "test_func7", 0, NULL);

	resume(pid1);
	sleep(13);
	kill(pid1);

	pid1 = create(test_func7, 2000, 20, "test_func7", 0, NULL);
	resume(pid1);
	sleep(13);
	kill(pid1);

	kprintf("\n\t Finished! Check error and replaced frames\n");
}

// LFU/FIFO
/*-------------------------------------------------------------------------------------*/
void test_func8()
{
	STATWORD ps;
	int PAGE0 = 0x40000;
	int i, j, temp = 0;
	int addrs[1200];
	int cnt = 0;

	// can go up to  (NFRAMES - 5 frames for null prc - 1pd for main - 1pd + 1pt frames for this proc)
	int maxpage = (NFRAMES - (5 + 1 + 1 + 1)); //=1016
											   // int maxpage = (NFRAMES - 25);

	for (i = 0; i <= maxpage / 120; i++)
	{
		if (get_bs(i, 120) == SYSERR)
		{
			kprintf("get_bs call failed \n");
			return;
		}
		if (xmmap(PAGE0 + i * 120, i, 120) == SYSERR)
		{
			kprintf("xmmap call failed\n");
			return;
		}
		for (j = 0; j < 120; j++)
		{
			// store the virtual addresses
			addrs[cnt++] = (PAGE0 + (i * 120) + j) << 12;
		}
	}

	/* all of these should generate page fault, no page replacement yet
	   acquire all free frames, starting from 1032 to 2047, lower frames are acquired first
	   */
	for (i = 0; i < maxpage; i++)
	{
		*((int *)addrs[i]) = i + 1; // bring all pages in, only referece bits are set
	}

	// LFU counter would not be cleared
	// sleep(3); //after sleep, all reference bits should be cleared

	disable(ps); // reduce the possibility of trigger reference bit clearing routine while testing

	// trigger page replacement
	// reference pages are replaced backwards from highest vpno
	// from 2047 to 1032 + maxpage/2 = 1540
	// dont show numerous frame output - optimization may mess with order
	// debug_page_replace_policy = NOPRINTFRM;
	for (i = maxpage; i > (maxpage / 2) + 1; i--)
	{
		*((int *)addrs[i]) = i + 1; // set both ref bits and dirty bits for these pages
		/*
		kprintf("in: \t%8x\n", addrs[i]);
		temp = *((int *)addrs[i]);
		if (temp != i + 1)
			kprintf("\tFAILED!\n");
		//*/
	}

	kprintf("\t 8.1 Expected replaced frame: %d\n\t", 1032 + maxpage / 2);
	enable(ps); // to allow page fault
	// allow frame output
	// debug_page_replace_policy = PRINTFRM;
	// trigger page replace ment, expected output: frame 1032+maxpage/2=1540 will be swapped out
	// this test might have a different result (with very very low possibility) if bit clearing routine is called before executing the following instruction
	*((int *)addrs[maxpage / 2 + 1]) = maxpage + 1;
	// kprintf("in: \t%8x\n", addrs[maxpage/2 + 1]);
	temp = *((int *)addrs[maxpage / 2 + 1]);
	if (temp != maxpage + 1)
		kprintf("\tFAILED!\n");

	// LFU counter would not be cleared but delay output
	// sleep(3); //after sleep, all reference bits should be cleared

	disable(ps); // reduce the possibility of trigger reference bit clearing routine while testing

	/*
	for(i=0; i < maxpage; i++)
	{
			*((int *)addrs[i]) = i + 1; //set both ref bits and dirty bits for these pages

	}
	//*/

	// continue replacement
	// from 1032 + maxpage/2 to 1032 + maxpage/2 = 1540
	// dont show numerous frame output - optimization may mess with order
	// debug_page_replace_policy = NOPRINTFRM;
	for (i = maxpage / 2; i > 1; i--)
	{
		*((int *)addrs[i]) = i + 1; // set both ref bits and dirty bits for these pages
		/*
		kprintf("in: \t%8x\n", addrs[i]);
		temp = *((int *)addrs[i]);
		if (temp != i + 1)
			kprintf("\tFAILED!\n");
		//*/
	}

	// kprintf("\t 8.2 Expected replaced frame: %d\n\t",1032+maxpage/3);
	kprintf("\t 8.2 Expected replaced frame: %d\n\t", 2047);
	enable(ps); // to allow page fault
	// allow frame output
	// debug_page_replace_policy = PRINTFRM;
	// trigger page replace ment, expected output: frame 1032+maxpage/3=1370 will be swapped out
	// this test might have a different result (with very very low possibility) if bit clearing routine is called before executing the following instruction
	*((int *)addrs[1]) = maxpage + 2;
	temp = *((int *)addrs[1]);
	if (temp != maxpage + 2)
		kprintf("\tFAILED!\n");

	/*
	// dont show numerous frame output - optimization may mess with order
	// debug_page_replace_policy = NOPRINTFRM;
	// replace remaining to even counts for next test
	for(i=maxpage/3; i > 0; i--)
	{
			*((int *)addrs[i]) = i + 1; //set both ref bits and dirty bits for these pages

	}
	kprintf("\tleave testfunc \n");
	//*/

	for (i = 0; i <= maxpage / 120; i++)
	{
		xmunmap(PAGE0 + (i * 120));
		release_bs(i);
	}
}

void test8()
{
	int pid1;

	kprintf("\nTest 8: Test FIFO page replacement policy\n");
	srpolicy(FIFO); // LFU
	kprintf("\n\t First run:\n");
	pid1 = create(test_func8, 2000, 20, "test_func8", 0, NULL);
	resume(pid1);
	sleep(15);
	kill(pid1);

	///*
	kprintf("\n\t Second run (test where killing process is handled correctly):\n");
	pid1 = create(test_func8, 2000, 20, "test_func8", 0, NULL);
	resume(pid1);
	sleep(15);
	kill(pid1);
	//*/

	kprintf("\n\t Finished! Check error and replaced frames\n");
}

int main()
{
	int i, s;
	char buf[8];

	kprintf("\n\nHello World, Xinu lives\n\n");

	kprintf("nframes: %d\n", NFRAMES);
	test2();

	test7();

	test8();

	shutdown();
}
