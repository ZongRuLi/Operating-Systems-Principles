
	.text
	.globl zfunction
	.type zfunction, @function
zfunction:
/*
Stack Pointer register (SP). Pointer to the top of the stack.
Stack Base Pointer register (BP). Used to point to the base of the stack.
*/	
	pushl %ebp 		/* stores the stack frame of the calling function on the stack. */
	movl %esp,%ebp		/* takes the current stack pointer and uses it as the base pointer for the current stack frame(the called function). */
	movl 8(%ebp),%eax 	/* load  value of param in eax */
			 	/* address map
					-8   local Variavle 2
					-4   local Variable 1
					0x00 saved EBP (extended base pointer)
					0x04 return address
					0x08 parameter 1 
					0x0c parameter 2 
					0x10 parameter 3 
				*/ 
	shll $4,%eax		/* shift left long variable for 4 bits */
	andl $0xFC01FFF0,%eax	/* mask out bit from 10-4 th to 18-4 th bits, MSB=0th bit,LSB=31th bit */
	
	movl %ebp,%esp 		/* restore esp */
	popl %ebp		/* pop from the stack */
	ret
