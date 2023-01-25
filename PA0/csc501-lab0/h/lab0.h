
#ifndef _LAB0_H_
#define _LAB0_H_

#define NSYSCALL 	27

#define FREEMEM 	0
#define CHPRIO		1
#define GETPID 		2
#define GETPRIO		3
#define GETTIME		4
#define KILL		5
#define RECEIVE		6
#define RECVCLR		7
#define RECVTIM		8
#define RESUME		9
#define SCOUNT		10
#define SDELETE		11
#define SEND		12
#define SETDEV		13
#define SETNOK		14
#define SCREATE		15
#define SIGNAL		16
#define SIGNALN		17
#define SLEEP		18
#define SLEEP10		19
#define SLEEP100	20
#define SLEEP1000	21
#define SRESET		22
#define STACKTRACE	23
#define SUSPEND		24
#define UNSLEEP		25
#define WAIT		26

extern long zfunction(long param);
extern void printsegaddress();	
extern void printtos();		
extern void printprocstks(int);

extern void printsyscallsummary();
extern void syscallsummary_start();
extern void syscallsummary_stop();
extern void syscall_start(int );
extern void syscall_stop(int, unsigned int);

extern unsigned long ctr1000;

struct SysCallEntry{ 
	unsigned int 	cnt;			// count how many times of system call 
	unsigned int 	TimeSum; 		// total time of system call
};
extern struct SysCallEntry sctab[][NSYSCALL];	// record system call for each process	
extern int scTraEn; 				// enable OS to trace system call on all process
extern int pscTrace[]; 				// flags that indicate which process did system call

#endif
