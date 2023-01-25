#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

int scTraEn; // enable system call trace
int pscTrace[NPROC]; // trace system call for each process
struct SysCallEntry sctab[NPROC][NSYSCALL];
/*
struct SysCallEntry{ 
	unsigned int 	Freq; 
	unsigned int 	AvgTime; 
	char 	       	*Name;
};
*/
char scname[NSYSCALL][20] = {
"sys_freemem",
"sys_chprio",
"sys_getpid",
"sys_getprio",
"sys_gettime",
"sys_kill",
"sys_receive",
"sys_recvclr",
"sys_recvtim",
"sys_resume",
"sys_scount",
"sys_sdelete",
"sys_send",
"sys_setdev",
"sys_setnok",
"sys_screate",
"sys_signal",
"sys_signaln",
"sys_sleep",
"sys_sleep10",
"sys_sleep100",
"sys_sleep1000",
"sys_sreset",
"sys_stacktrace",
"sys_suspend",
"sys_unsleep",
"sys_wait"
};

void syscallsummary_start()
{
	int pid,scid;
	for(pid=0;pid<NPROC;pid++){
		pscTrace[currpid] = 0;
		for(scid=0;scid<NSYSCALL;scid++){
			sctab[pid][scid].cnt=0;
			sctab[pid][scid].TimeSum=0;
		}
	}
	scTraEn = 1;
};
void syscallsummary_stop(){
	scTraEn = 0;
};
void syscall_start(int _scid ){
	pscTrace[currpid] = 1;
	sctab[currpid][_scid].cnt++;
};
void syscall_stop(int _scid, unsigned int _startTime){
	sctab[currpid][_scid].TimeSum += (ctr1000 - _startTime);
};
void printsyscallsummary(){

	struct SysCallEntry *scptr;
	kprintf("\n\nvoid printsyscallsummary()\n");
	int pid,scid;
	for(pid=0;pid<NPROC;pid++){
		if( pscTrace[pid] == 0) continue;
		kprintf("Process [pid:%d]\n",pid);

		for(scid=0;scid<NSYSCALL;scid++){
			scptr = &sctab[pid][scid];
			if(scptr->cnt == 0) continue;
			kprintf("\tSyscall: %s, count: %d, average execution time: %d (ms)\n",scname[scid],scptr->cnt, scptr->TimeSum/scptr->cnt);
			
		}
	}

};

