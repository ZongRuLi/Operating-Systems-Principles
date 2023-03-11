#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
	            kprintf(error);\
	            return;\
	            }

/*----------------------------------Test XINU semaphore---------------------------*/
/*
 * Result of priority inversion:
 * 
 * priority: A > B > C, Note: * means blocked
 * 
 * proc A	    L *(A blocked)  R R
 * proc B             L L ... L L
 * proc C	L R  		  R
 * output	C C A B B ... B B C A A 
 * 
 * */
char	output[20];
int	count;

void proc_A(char msg, int sem)
{		
	output[count++]=msg;
	kprintf ("  %c: to acquire sem\n", msg);
        wait (sem);
	output[count++]=msg;
	kprintf ("  %c: acquired sem\n", msg);
	
	kprintf ("  %c: to release sem\n", msg);
	output[count++]=msg;
	signal (sem);
}

void proc_B(char msg)
{	
	int	i, cnt = 0;
	do{
		for(i=0; i<100000000; i++)
			;
		cnt++;
		if( cnt > 10 )
			break;
		kprintf ("  %c: running.\n", msg);
		output[count++]=msg;
	}while (1);	
}
void proc_C(char msg, int sem)
{		
	output[count++]=msg;
	kprintf ("  %c: to acquire sem\n", msg);
        wait (sem);
	output[count++]=msg;
	kprintf ("  %c: acquired sem, sleep 1s\n", msg);
	sleep (2);

	kprintf ("  %c: to release sem\n", msg);
	output[count++]=msg;
	signal (sem);
}

void testsem()
{
	count = 0;	
	int	sem;
	int	procA, procB, procC;

        kprintf("\nTest : test the priority inversion with XINU semaphore\n");

	sem = screate (1);
	assert(sem != SYSERR, "Test semaphore failed")
	
	procA = create(proc_A, 2000, 15, "procA", 2, 'A', sem	);
	procB = create(proc_B, 2000, 10, "procB", 1, 'B'	);
	procC = create(proc_C, 2000,  5, "procC", 2, 'C', sem	);

	resume(procC);
	sleep (1);

	resume(procA);
	resume(procB);
	sleep (5);
		
	kprintf("output=%s\n", output);
        assert(mystrncmp(output,"CCABBBBBBBBBBCAA",16)==0,"Test semaphore FAILED\n");
        kprintf ("Test semaphore OK\n");
	sdelete(sem);
}
/*----------------------------------Test reader/writer lock---------------------------*/
/*
 * Result of priority inversion:
 * 
 * priority: A > B > C, Note: * means blocked
 *
 * proc A	    L * R R
 * proc B                   L L ... L L
 * proc C	L R   R	  
 * output	C C A C A A B B ... B B 
 * 
 * */

void writerA(char msg, int lck)
{		
	output[count++]=msg;
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	output[count++]=msg;
	kprintf ("  %c: acquired lock\n", msg);
	
	kprintf ("  %c: to release lock\n", msg);
	output[count++]=msg;
	releaseall (1, lck);
}
void writerC(char msg, int lck)
{		
	output[count++]=msg;
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	output[count++]=msg;
	kprintf ("  %c: acquired lock, sleep 1s\n", msg);
	sleep (1);

	kprintf ("  %c: to release lock\n", msg);
	output[count++]=msg;
	releaseall (1, lck);
}
void testlock()
{
	count = 0;
	int	lck;
	int	procA, procB, procC;

        kprintf("\nTest : test the priority inversion with XINU semaphore\n");

	lck = lcreate ();
	assert(lck != SYSERR, "Test lock failed")
	
	procA = create(writerA, 2000, 15, "procA", 2, 'A', lck	);
	procB = create( proc_B, 2000, 10, "procB", 1, 'B'	);
	procC = create(writerC, 2000,  5, "procC", 2, 'C', lck	);

	resume(procC);
	sleep (1);

	resume(procA);
	resume(procB);
	sleep (5);
		
	kprintf("output=%s\n", output);
        assert(mystrncmp(output,"CCACAABBBBBBBBBB",16)==0,"Test lock FAILED\n");
        kprintf ("Test lock OK\n");
	ldelete(lck);
};

void task1(){
	testsem();
	testlock();
}
