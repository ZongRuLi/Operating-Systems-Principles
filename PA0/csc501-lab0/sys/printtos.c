
static unsigned long 	*ebp;

void printtos()
{
	/*
	 * refer: https://eli.thegreenplace.net/2011/02/04/where-the-top-of-the-stack-is-on-x86/
	 * The beginning part of all functions in assembelly:
	 *
	 * 0: push %ebp
	 * 1: mov %esp, %ebp
	 * 3: sub %0x?? %esp 
	 * ... (your hand write code)
	 *
	 * When we want to access top of stack through stack pointer, we need to use %ebp instead of %esp.
	 * because esp already been subtract by the offset of local variables. It didn't point to original
	 * stack position(top of stack). Only %ebp is stores original stack pointer and point to top of stack.
	 *
	 * To find address of top of stack before function call, we need can know what push did.
	 * > push %ebp 
	 * equals to 
	 * > sub $4, esp
	 * > mov %ebp, (%esp)
	 * Therefore, we can get stack pointer(esp) before program execute "push %ebp"
	 * by increasing stack pointer by 0x04 (ebp + 1).
	 * */
	long *prev, *curr;
	kprintf("\n\nvoid printtos()\n");
		
	asm("movl %ebp, ebp");
	prev = ebp + 1;
	curr = ebp;
	
	kprintf("Before[0x%08x]: 0x%08x\n",prev, *prev);
	kprintf("After [0x%08x]: 0x%08x\n",curr, *curr);

	//Printing upto four stack locations below the top of the stack 
	int i;
	for(i=0; i<4; i++) {
		kprintf("\telement[0x%08x] : 0x%08x\n", (curr-i),*(curr - i));
	}
	/* refer: https://en.wikibooks.org/wiki/X86_Disassembly/Functions_and_Stack_Frames
	 * function foo is used to verify stack map, you will see terminal print out 0xaa at (ebp+2) and 0xbb at (ebp +3).
	 * assembly of calling function foo is
	 * > push 0xaa,
	 * > push 0xbb,
	 * > call _foo,
	 * and call instruction equals to
	 * > push eip +2 ; return address is current address + size of two instructions
	 * > jmp _foo
	 * */
	//foo(0xaa, 0xbb);
}

/*
void foo2(int a, int b){
	asm("movl %ebp, ebp");
	kprintf("After [0x%08x]: 0x%08x\n",ebp, *ebp);
	kprintf("Before[0x%08x]: 0x%08x\n",(ebp+1), *(ebp+1));
	kprintf("Before[0x%08x]: 0x%08x\n",(ebp+2), *(ebp+2));
	kprintf("Before[0x%08x]: 0x%08x\n",(ebp+3), *(ebp+3));
}
*/
