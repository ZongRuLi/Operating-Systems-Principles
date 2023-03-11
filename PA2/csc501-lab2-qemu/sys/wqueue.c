/* wqueue.c  -  winsert, wenqueue, wdequeue, wgetfirst, wgetlast, wprintwq */

#include <conf.h>
#include <kernel.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * winsert.c  --  insert an process into a waiting queue list in key order
 *------------------------------------------------------------------------
 */
int winsert(int lid, int proc, int head, int type, int key)
{
	int	next;			/* runs through list		*/
	int	prev;
	struct lentry	*lptr;
	
	lptr = &locktab[lid];
	next = lptr->wq[head].qnext;
	while (lptr->wq[next].qkey < key)	/* tail has maxint as key	*/
		next = lptr->wq[next].qnext;
	lptr->wq[proc].qnext = next;
	lptr->wq[proc].qprev = prev = lptr->wq[next].qprev;
	lptr->wq[proc].qkey  = key;
	lptr->wq[proc].qtype  = type;
	lptr->wq[prev].qnext = proc;
	lptr->wq[next].qprev = proc;
	return(OK);
}

/*------------------------------------------------------------------------
 * wenqueue  --	insert an item at the tail of a waiting queue list
 *------------------------------------------------------------------------
 */
int wenqueue(int lid, int item, int tail)
/*	int	item;			- item to enqueue on a list	*/
/*	int	tail;			- index in q of list tail	*/
{
	struct lentry	*lptr;
	struct	wqent	*tptr;		/* points to tail entry		*/
	struct	wqent	*mptr;		/* points to item entry		*/

	lptr = &locktab[lid];
	tptr = &(lptr->wq[tail]);
	mptr = &(lptr->wq[item]);
	mptr->qnext = tail;
	mptr->qprev = tptr->qprev;
	lptr->wq[tptr->qprev].qnext = item;
	tptr->qprev = item;
	return(item);
}


/*------------------------------------------------------------------------
 *  wdequeue  --  remove an item from the head of a waiting queue list and return it
 *------------------------------------------------------------------------
 */
int wdequeue(int lid, int item)
{
	struct lentry	*lptr;
	struct	wqent	*mptr;		/* pointer to q entry for item	*/

	lptr = &locktab[lid];
	mptr = &(lptr->wq[item]);
	lptr->wq[mptr->qprev].qnext = mptr->qnext;
	lptr->wq[mptr->qnext].qprev = mptr->qprev;
	return(item);
}

/*------------------------------------------------------------------------
 * wgetfirst  --	 remove and return the first process on a waiting queue list
 *------------------------------------------------------------------------
 */
int wgetfirst(int lid, int head)
{
	int	proc;			/* first process on the list	*/

	if ((proc=locktab[lid].wq[head].qnext) < NPROC)
		return( wdequeue(lid, proc) );
	else
		return(EMPTY);
}

/*------------------------------------------------------------------------
 * wgetlast  --  remove and return the last process from a waiting queue list
 *------------------------------------------------------------------------
 */
int wgetlast(int lid, int tail)
{
	int	proc;			/* last process on the list	*/

	if ((proc=locktab[lid].wq[tail].qprev) < NPROC)
		return( wdequeue(lid, proc) );
	else
		return(EMPTY);
}

void printwq(int lid)
{

	struct lentry	*lptr;
	int head, next;	

	lptr = &locktab[lid];
	head = lptr->wqhead;
	kprintf("\nwq[%d]=",lid);
	for( next = lptr->wq[head].qnext ; next != lptr->wqtail ; next = lptr->wq[next].qnext ){
		kprintf("%d,",lptr->wq[next].qkey );
	}
	kprintf(".\n\n");
	return;
}
