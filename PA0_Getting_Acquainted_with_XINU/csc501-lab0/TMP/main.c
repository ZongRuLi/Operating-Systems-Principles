/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lab0.h>

int prA, prX;
void halt();

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
prch(c)
char c;
{
	int i;
	sleep(5);	
}
int main()
{

	kprintf("\n\nHello World, Xinu lives\n\n");
	kprintf("\nzfunction(0xaabbccdd)=%lx",zfunction(0xaabbccdd));
	kprintf("\nzfunction(0x00000000)=%lx",zfunction(0x00000000));
	kprintf("\nzfunction(0xffffffff)=%lx",zfunction(0xffffffff));
	if(zfunction(0xaabbccdd)==0xa800cdd0)
		kprintf("\n[INFO] zfunction test pass!");
	else
		kprintf("\n[ERROR] zfunction test fail!");

	printsegaddress();
	printtos();
	resume(prA = create(prch,1024,40,"proc A",1,'A'));
	
	//kprintf("\n\nTask 5 (printsyscallsummary)\n");
	syscallsummary_start();
	resume(prX = create(prch,2000,20,"proc X",1,'A'));
	printprocstks(0);
	sleep(1);
	sleep(2);
	sleep(3);
	syscallsummary_stop();
	printsyscallsummary();
	return 0;
}

