/* framequeue.c - initfqueue, push_back, pop_front, delete */
#include<kernel.h>
#include<fq.h>
#include<paging.h>
#include <Debug.h>

int 	fqhead;
int 	fqtail;
int		fqcurrpos; // current node position where SC start to check pt_acc

struct fqent	fq[NFRAMES+2];

void init_fqueue()
{
	int i;

	fqhead = NFRAMES;
	fqtail = NFRAMES+1;
	fqcurrpos = -1;
	
	fq[fqhead].qnext = fqtail;
	fq[fqhead].qprev = EMPTY;
	
	fq[fqtail].qnext = EMPTY;
	fq[fqtail].qprev = fqhead;

	for(i=0;i<NFRAMES;i++)
	{
		fq[i].qnext = EMPTY;	
		fq[i].qnext = EMPTY;	
	}
}

void fq_push_back(int fid)
{
	int prev = fq[fqtail].qprev;

	if( fq[fid].qprev != EMPTY && fq[fid].qnext != EMPTY ){
		lDebug(DBG_ERR,"[ERROR][push_back] frame queue (%d) node is not empty!!", fid);
		return;
	} 
	if( fq[fqhead].qnext == fqtail ) fqcurrpos = fid;
	
	fq[prev].qnext = fid;

	fq[fid].qprev = prev;
	fq[fid].qnext = fqtail;

	fq[fqtail].qprev = fid;

}

int fq_pop_front()
{
	int head = fq[fqhead].qnext;
	int second = fq[head].qnext;

	fq[fqhead].qnext = second;
	fq[second].qprev = fqhead;

	fq[head].qnext = EMPTY;
	fq[head].qnext = EMPTY;

	return head; 
}

int fq_delete(int fid)
{
	int prev = fq[fid].qprev;
	int	next = fq[fid].qnext;

	fq[prev].qnext = next;
	fq[next].qprev = prev;

	fq[fid].qnext = EMPTY;
	fq[fid].qnext = EMPTY;

	return fid;
}

int fq_next_pos()
{
	int next = fq[fqcurrpos].qnext;
	
	if( next == EMPTY )
	{
		lDebug(DBG_ERR,"[ERROR][push_back] fq current position (%d) node is empty!! Can't find next node!!", fqcurrpos);
		return;
	}

	if( next == fqtail ) next = fq[fqhead].qnext;

	return (fqcurrpos = next);
}

void fq_print()
{
	int next;
	kprintf("fqhead->");
	for(next=fq[fqhead].qnext; next!= fqtail ; next=fq[next].qnext)
	{
		kprintf("f(%d)->", next);
	}
	kprintf("fqtail\n");
}
