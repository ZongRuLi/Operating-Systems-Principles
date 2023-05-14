/* lcreate.c - create a lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
SYSCALL lcreate()
{
	STATWORD ps;
	int lid, ldes, lid_is_good;

	disable(ps);

       	lid = find_available_lock();	
	lid_is_good = !isbadlock(lid);

	if( lid_is_good ) {
		ldes = generate_lock_descriptor( lid );
		locktab[lid].lstate = FREE;
		restore(ps);
		return( ldes );
	}

	restore(ps);
	return(SYSERR);
}

int find_available_lock(){
	int	lid;

	// TBD: different iterate order, prevent to pick the same lock id.
	for( lid=0 ; lid < NLOCKS ; lid++ ){
		if( isuninitlock(lid) || isdeletedlock(lid) )
			return lid;
	}
	return -1;
}

int generate_locktoken(){
	locktoken++;
	return locktoken;
}

int generate_lock_descriptor(int lid){
	int ldes, token;
	token = generate_locktoken();
	locktab[lid].ltoken = token;
	ldes = (token << LIDBITS ) | lid;
	return ldes;
}
