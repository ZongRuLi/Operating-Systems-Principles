
	.text
	.globl zfunction
	.type zfunction, @function
zfunction:
	
	pushl %ebp
	movl %esp,%ebp
	movl 8(%ebp),%eax /* load  value of param in eax */
			 /* TBD: rewrite description
				displacement = 8 as we need to move two locations */ 
			 /* below after base pointer location first "return address" */ 
			 /* and then "paramter" can be pointed below it */	
	shll $4,%eax		/* shift left long variable for 4 bits */
	andl $0xFC01FFF0,%eax	/* mask out bit from 10-4 th to 18-4 th bits, MSB=0th bit,LSB=31th bit */
	
	movl %ebp,%esp
	popl %ebp
	ret
