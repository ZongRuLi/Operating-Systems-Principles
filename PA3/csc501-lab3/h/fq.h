/* fq.h */

#ifndef _FQ_H_
#define _FQ_H_

#define NFQENT NFRAMES+2

struct fqent {
	int		qkey;		/* frame id to frm_tab */
	int		qnext;
	int		qprev;
};

extern int 	fqhead;
extern int 	fqtail;
extern int 	fqcurrpos;

extern struct fqent	fq[];

extern void init_fqueue();
extern void fq_push_back(int fid);
extern int fq_pop_front();
extern int fq_delete(int fid);
extern int fq_next_pos();
extern void fq_print();

#endif
