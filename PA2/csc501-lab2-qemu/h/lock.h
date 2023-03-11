/* lock.h */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define NLOCKS 30	/* number of locks */
#endif

// LOCK IS NOT IN USE
#define UINIT 	0	/* lock is un-initialized */
//#define DELETED 4	// already been defined in kernel.h

// LOCK IS IN USE
#define FREE 	1	/* lock is free */
#define WRITE 	2
#define READ 	3

#define NWQENT	NPROC+2

#define HOLD	1
#define UNHOLD	0

struct	wqent	{		/* one for each process plus two for	*/
				/* each list				*/
	int	qkey;		/* key on which the queue is ordered	*/
	int	qtype;		/* waiting lock type: READ or WRITE	*/
	int	qnext;		/* pointer to next process or tail	*/
	int	qprev;		/* pointer to previous process or head	*/
};

struct lentry {
	int		lstate;	/* the status of lock */
	int		ltoken;	/* the valid token in current lock descriptor */
	int		lprio;	/* max prio among all proc waiting in the waiting queue */
	struct wqent	wq[NWQENT];	/* waiting queue */
	int		wqhead;
	int		wqtail;
	Bool		lholdproc[NPROC];	/* the process hold this lock */
};
extern struct lentry locktab [];
extern int nextlock;

extern int LIDBITS;	/* bit width of lock id */
extern int locktoken; 	/* use to identify different lock using same lock number. */

extern unsigned long ctr1000;

#define BADLID		-1

#define	isbadlock(x)	(x<0 || x>=NLOCKS)
#define	isuninitlock(x)	( locktab[x].lstate == UINIT )
#define	isfreelock(x)	( locktab[x].lstate == FREE )
#define	isreadlock(x)	( locktab[x].lstate == READ )
#define	iswritelock(x)	( locktab[x].lstate == WRITE )
#define	isdeletedlock(x)	( locktab[x].lstate == DELETED )
#define currpid_hold_lock(x)	( locktab[x].lholdproc[currpid] == 1 )

extern int lock (int ldes, int type, int priority);
extern void linit();

extern int GDB;

#define wisempty(lid)	( locktab[lid].wq[ locktab[lid].wqhead ].qnext >= NPROC )
#define wnonempty(lid)	( locktab[lid].wq[ locktab[lid].wqhead ].qnext < NPROC )

int wenqueue(int lock, int item, int tail);
int wdequeue(int lock, int item);
void printwq(int lock);
int winsert(int lock, int proc ,int head, int type, int key);
int wgetfirst(int lock, int head);
int wgetlast(int lock, int tail);

int find_available_lock();
int generate_locktoken();
int generate_lock_descriptor(int lid);
int get_lid_and_token(int ldes, int* token);
int get_lid(int ldes);
void release(int lid);
int find_highest_prio(int lid, int type, int *maxprio);
int sum_of_lock_holder(int lid);
void set_hold(int lid, int pid);
void clear_hold(int lid, int pid);

void pinh_raise(int lid, int currprio);
void pinh_recalculate(int pid);

#endif
