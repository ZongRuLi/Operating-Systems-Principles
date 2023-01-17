/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	kprintf("\n\nzfunction(0xaabbccdd)=%lx\n\n",zfunction(0xaabbccdd));
	if(zfunction(0xaabbccdd)==0xa800cdd0)
		kprintf("[INFO] zfunction test pass!\n");
	else
		kprintf("[ERROR] zfunction test fail!\n");


	return 0;
}
