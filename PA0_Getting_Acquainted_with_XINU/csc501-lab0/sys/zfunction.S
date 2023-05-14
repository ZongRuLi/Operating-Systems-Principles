
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

			 	/* address map, refer to: https://en.wikibooks.org/wiki/X86_Disassembly/Functions_and_Stack_Frames
				 EBP +16->	| 3rd argument
				 EBP +12->	| 2nd argument
				 EBP +8 ->	| 1st argument 
				 EBP +4 ->	| return address
				 EBP    -> 	| saved EBP (in line 10)
				 EBP -4 ->	| 1st local variable
				 EBP -8 ->	| 2nd local variable
					... (if allocate more local variable)
				 ESP ->		| X
				*/ 
	shll $4,%eax		/* shift left long variable for 4 bits */
	andl $0xFC01FFF0,%eax	/* mask out bit from 10-4 th to 18-4 th bits, MSB=0th bit,LSB=31th bit */
	
	movl %ebp,%esp 		/* restore esp */
	popl %ebp		/* pop from the stack */
	ret
