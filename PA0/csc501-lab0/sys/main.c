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
	return 0;
}
