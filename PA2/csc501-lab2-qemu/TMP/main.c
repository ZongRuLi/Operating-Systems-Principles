/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
	    failcount++;\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}

int	failcount;
/*--------------------------------Test 7--------------------------------*/
 
void test7 ()
{
	int	lck,	i,	temp;
	int	lcks[NLOCKS];

	kprintf("\nTest 7: lock allocation overflow test\n");
	for(i=0;i<NLOCKS;i++){
		lcks[i]  = lcreate ();
		assert (lcks[i] != SYSERR, "Test 7 failed");		
	}
	lck = lcreate();
	assert (lck == SYSERR, "Test 7 failed, expect no available locks!");		
	
	temp = lcks[0];
	ldelete (lcks[0]);
	lcks[0]= lcreate();
	assert( get_lid(temp) == get_lid(lcks[0]), "Test 7 failed, Can not re-create the same lock descriptor")

	for(i=0;i<NLOCKS;i++){
		ldelete (lcks[i]);
	}
	kprintf ("Test 7 ok\n");
}

/*--------------------------------Test 8--------------------------------*/

void reader8A (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void reader8B (char *msg, int lck)
{
	int return_msg;
	return_msg = lock (lck, READ, DEFAULT_LOCK_PRIO);
	assert (return_msg == SYSERR, "Test 8 failed, lock() after the lock is deleted expect return SYSERR!");		

	kprintf ("Test 8 ok\n");
}

void test8 ()
{
	int	lck;
	int	pid1;
	int	pid2;

        kprintf("\nTest 8: acquried deleted locks. Expected return SYSERR\n");
	
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 8 failed");		

	pid1 = create(reader8A, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader8B, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	
	sleep (5);
	ldelete (lck);
	resume(pid2);
}

/*--------------------------------Test 9--------------------------------*/

void reader9A (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void reader9B (char *msg, int lck)
{
	int return_msg;
	return_msg = lock (lck, READ, DEFAULT_LOCK_PRIO);
	assert (return_msg == SYSERR, "Test 9 failed, lock() after the lock is deleted expect return SYSERR!");		

	kprintf ("Test 9 ok\n");
}

void test9 ()
{
	int	lck;
	int	pid1;
	int	pid2;

        kprintf("\nTest 9: acquried deleted and renewed locks. Expected return SYSERR\n");
	
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 9 failed");		

	pid1 = create(reader9A, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader9B, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	
	sleep (5);
	ldelete (lck);
	
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 9 failed");		
	resume(pid2);
	
	sleep(3);
	ldelete(lck);
}
/*--------------------------------Test 10--------------------------------*/

void writer10 (char *msg, int lck)
{
	int return_msg;
	lock (lck, WRITE, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	return_msg = releaseall (1, lck);
	
	assert (return_msg == SYSERR, "Test 10 failed, release deleted lock expect return SYSERR!");		
	kprintf ("Test 10 okok\n");
}

void reader10 (char *msg, int lck)
{
	int return_msg;
	return_msg = lock (lck, READ, DEFAULT_LOCK_PRIO);
	assert (return_msg == DELETED, "Test 10 failed, wait on deleted lock expect return DELETED!");		
	kprintf ("Test 10 ok\n");
}

void test10 ()
{
	int	lck;
	int	pid1;
	int	pid2;

        kprintf("\nTest 10: wait on lock and lock been deleted. Expected return DELETED, "
		"release deleted lock, Expect return SYSERR.\n");
	
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 10 failed");		

	pid1 = create(writer10, 2000, 20, "writer 10", 2, "writer 10", lck);
	pid2 = create(reader10, 2000, 20, "reader 10", 2, "reader 10", lck);

        kprintf("-start writer 10, then sleep 1s. lock granted to writer 10\n");
	resume(pid1);
	sleep100 (1);
	
        kprintf("-start reader 10, then sleep 1s. reader 10 wait for the lock\n");
	resume(pid2);
	sleep100 (1);
	
        kprintf("-Main delete lock, Expect move reader 10 to ready list and return DELETED to reader 10\n");
	ldelete (lck);
	
	sleep (3);
}
/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void test1 ()
{
	int	lck;
	int	pid1;
	int	pid2;

	kprintf("\nTest 1: readers can share the rwlock\n");
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 1 failed");

	pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	resume(pid2);
	
	sleep (5);
	ldelete (lck);
	kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/
char output2[10];
int count2;
void reader2 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer2 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test2 ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		" lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

	rd1 = create(reader2, 2000, 20, "reader2A", 3, 'A', lck, 20);
	rd2 = create(reader2, 2000, 20, "reader2B", 3, 'B', lck, 30);
	rd3 = create(reader2, 2000, 20, "reader2D", 3, 'D', lck, 25);
	rd4 = create(reader2, 2000, 20, "reader2E", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 28);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep10 (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);


        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABCCDEED",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
	ldelete(lck);
}

/*----------------------------------Test 4---------------------------*/
char output4[10];
int count4;
void reader4 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output4[count4++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output4[count4++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer4 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output4[count4++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output4[count4++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test4 ()
{
        count4 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 4: wait on locks with same priority but write wait time less than 500 ms. Expected order of"
		" lock acquisition is: reader A, reader B, writer C\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 4 failed");

	rd1 = create(reader4, 2000, 20, "reader4A", 3, 'A', lck, 20);
	rd2 = create(reader4, 2000, 20, "reader4B", 3, 'B', lck, 30);
        wr1 = create(writer4, 2000, 20, "writer4", 3, 'C', lck, 30);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C & reader B, then sleep 1s. writer & reader waits for the lock\n");
        resume(wr1);
        resume(rd2);
        
	//kprintf("-Main sleep 1500 ms\n");
        sleep (15);
        kprintf("output=%s\n", output4);
        assert(mystrncmp(output4,"AABBCC",6)==0,"Test 4 FAILED\n");
        kprintf ("Test 4 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 5---------------------------*/
char output5[10];
int count5;
void reader5 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output5[count5++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (6);
        output5[count5++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer5 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output5[count5++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (6);
        output5[count5++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test5 ()
{
        count5 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 5: wait on locks with same priority, write wait time longer than 500 ms and longer than read wait time. Expected order of"
		" lock acquisition is: reader A, reader B, writer C\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 5 failed");

	rd1 = create(reader5, 2000, 20, "reader5A", 3, 'A', lck, 20);
	rd2 = create(reader5, 2000, 20, "reader5B", 3, 'B', lck, 30);
        wr1 = create(writer5, 2000, 20, "writer5", 3, 'C', lck, 30);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep100 (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep (3);
        kprintf("-start reader B, then sleep 1s. reader waits for the lock\n");
        resume (rd2);
        
        sleep (15);
        kprintf("output=%s\n", output5);
        assert(mystrncmp(output5,"AACCBB",6)==0,"Test 5 FAILED\n");
        kprintf ("Test 5 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 6---------------------------*/
char output6[10];
int count6;
void reader6 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output6[count6++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (6);
        output6[count6++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer6 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output6[count6++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (6);
        output6[count6++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test6 ()
{
        count6 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1, wr2;

        kprintf("\nTest 6: wait on locks with same priority, write wait time longer than 500 ms but less than read wait time. Expected order of"
		" lock acquisition is: writer D, reader B, writer C\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 6 failed");

	//rd1 = create(reader6, 2000, 20, "reader6A", 3, 'A', lck, 20);
	rd2 = create(reader6, 2000, 20, "reader6B", 3, 'B', lck, 30);
        wr1 = create(writer6, 2000, 20, "writer6C", 3, 'C', lck, 30);
        wr2 = create(writer6, 2000, 20, "writer6D", 3, 'D', lck, 30);
	
        kprintf("-start writer D, then sleep 1s. lock granted to writer D\n");
        resume(wr2);
        sleep100 (1);

        kprintf("-start reader B, then sleep 1s. reader waits for the lock\n");
        resume(rd2);
        sleep100 (5);
        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume (wr1);
        
        sleep (20);
        kprintf("output=%s\n", output6);
        assert(mystrncmp(output6,"DDBBCC",6)==0,"Test 6 FAILED\n");
        kprintf ("Test 6 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3A", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3B", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed, wr1 !=25");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 3 failed, wr1 !=30");
	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed, wr1 !=25 after kill\n");

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert (getprio(wr1) == 20, "Test 3 failed, wr1 !=20 after kill\n");

        sleep (8);
        kprintf ("Test 3 OK\n");

	ldelete(lck);
}
/*----------------------------------Test 11---------------------------*/
void reader11 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer11 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test11 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1, wr2;

        kprintf("\nTest 11: test the basic pinh, inherit to multiple lock holder.\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 11 failed");

        rd1 = create(reader11, 2000, 20, "reader11A", 2, "reader A", lck);
        rd2 = create(reader11, 2000, 20, "reader11B", 2, "reader B", lck);
        wr1 = create(writer11, 2000, 25, "writer11A", 2, "writer A", lck);
        wr2 = create(writer11, 2000, 30, "writer11B", 2, "writer B", lck);

        kprintf("-start reader A, B, then sleep 1s. lock granted to read (prio 20)\n");
        resume(rd1);
        resume(rd2);
        sleep (1);

        kprintf("-start writer A, then sleep 1s. writer A(prio 25) blocked on the lock\n");
        resume(wr1);
        sleep (1);
	assert (getprio(rd1) == 25, "Test 11 failed, rd1 !=25");
	assert (getprio(rd2) == 25, "Test 11 failed, rd2 !=25");

        kprintf("-start writer B, then sleep 1s. writer B(prio 30) blocked on the lock\n");
        resume (wr2);
	sleep (1);
	assert (getprio(rd1) == 30, "Test 11 failed, rd1 !=30");
	assert (getprio(rd2) == 30, "Test 11 failed, rd2 !=30");
	
	kprintf("-kill writer B, then sleep 1s\n");
	kill (wr2);
	sleep (1);
	assert (getprio(rd1) == 25, "Test 11 failed, rd1 !=25 after kill\n");
	assert (getprio(rd2) == 25, "Test 11 failed, rd2 !=25 after kill\n");

	kprintf("-kill writer A, then sleep 1s\n");
	kill (wr1);
	sleep(1);
	assert (getprio(rd1) == 20, "Test 11 failed, rd1 !=20 after kill\n");
	assert (getprio(rd2) == 20, "Test 11 failed, rd2 !=20 after kill\n");

        sleep (8);
        kprintf ("Test 11 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 12---------------------------*/
void reader12 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer12A (char *msg, int lck1, int lck2)
{
        kprintf ("  %s: to acquire lock1 \n", msg);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock1\n", msg);//sleep 10s\n", msg, lck1);
        
	kprintf ("  %s: to acquire lock2 \n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO); 
        kprintf ("  %s: acquired lock2, sleep 10s\n", msg);
        sleep (10);

	kprintf ("  %s: to release lock\n", msg);
        releaseall (2, lck1, lck2);
}
void writer12B (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test12 ()
{
        int     lck1, lck2;
        int     rd1, rd2;
        int     wr1, wr2;

        kprintf("\nTest 12: test the pinh, proc A hold lock 1, proc B hold lock 2.\n"
		       " And then proc A acquire lock 2 and wait on proc B.\n"
		       " And then proc C acquire lock 1 and wait on proc A.\n"
		       " relationship before kill: proc C -> proc A -> proc B\n");
        lck1  = lcreate ();
        lck2  = lcreate ();
        assert (lck1 != SYSERR, "Test 12 failed");
        assert (lck2 != SYSERR, "Test 12 failed");

        rd1 = create(reader12,  2000, 30, "proc C", 2, "reader C", lck1);
        //rd2 = create(reader12, 2000, 30, "reader12B", 2, "reader B", lck);
        wr1 = create(writer12A, 2000, 25, "proc A", 3, "writer A", lck1, lck2);
        wr2 = create(writer12B, 2000, 20, "proc B", 2, "writer B", lck2);

        kprintf("-start writer B, then sleep 1s. lock2 granted to write B (prio 20)\n");
        resume(wr2);
        sleep (1);

        kprintf("-start writer A, then sleep 1s. lock1 grant to wirte A (prio 25). writer A(prio 25) blocked on the lock2\n");
        resume(wr1);
        sleep (1);
	assert (getprio(wr2) == 25, "Test 12 failed, proc B !=25");

        kprintf("-start reader C, then sleep 1s. reader C (prio 30) blocked on the lock1\n");
        resume (rd1);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 12 failed, proc A !=30");
	assert (getprio(wr2) == 30, "Test 12 failed, proc B !=30");
	
	kprintf("-kill reader C, then sleep 1s\n");
	kill (rd1);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 12 failed, proc A !=25 after kill\n");
	assert (getprio(wr2) == 25, "Test 12 failed, proc B !=25 after kill\n");

	kprintf("-kill writer A, then sleep 1s\n");
	kill (wr1);
	sleep(1);
	assert (getprio(wr2) == 20, "Test 12 failed, proc B !=20 after kill\n");

        sleep (8);
        kprintf ("Test 12 OK\n");
	ldelete(lck1);
	ldelete(lck2);
}
/*----------------------------------Test 13---------------------------*/
void reader13 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer13 (char *msg, int lck1, int lck2)
{
        kprintf ("  %s: to acquire lock1 \n", msg);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock1\n", msg);//sleep 10s\n", msg, lck1);
        //sleep (3);
        
	kprintf ("  %s: to acquire lock2 \n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO); 
        kprintf ("  %s: acquired lock2, sleep 5s\n", msg);
        sleep (5);

	kprintf ("  %s: to release lock1, sleep 5s\n", msg);
        releaseall (1, lck1);
        sleep (5);

	kprintf ("  %s: to release lock2, sleep 5s\n", msg);
        releaseall (1, lck2);
	sleep (5);
}

void test13 ()
{
        int     lck1, lck2;
        int     rd1, rd2;
        int     wr1, wr2;

        kprintf("\nTest 13: test the pinh, proc A hold lock 1 and lock 2,\n"
		       " And then proc B acquire lock 1 and wait on proc A.\n"
		       " And then proc C acquire lock 2 and wait on proc A.\n"
		       " relationship before kill: proc B & proc C -> proc A\n"
		       " when proc A release lock 1, it should still inherit priority from proc C\n");
        lck1  = lcreate ();
        lck2  = lcreate ();
        assert (lck1 != SYSERR, "Test 12 failed");
        assert (lck2 != SYSERR, "Test 12 failed");

        rd1 = create(reader13, 2000, 30, "proc B", 2, "reader B", lck1);
        rd2 = create(reader13, 2000, 25, "proc C", 2, "reader C", lck2);
        wr1 = create(writer13, 2000, 20, "proc A", 3, "writer A", lck1, lck2);

        kprintf("-start writer A, then sleep 3s. lock 1 & lock2 granted to write A (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader C, then sleep 1s. reader C(prio 25) blocked on the lock2\n");
        resume(rd2);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 13 failed, proc A !=25");
        
        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock1\n");
	resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 30, "Test 13 failed, proc A !=30");

	kprintf("-sleep 5s, wait proc B to release lock1\n");
	sleep(5);
	assert (getprio(wr1) == 25, "Test 13 failed, proc A !=25 after release lock1\n");

	kprintf("-sleep 5s, wait proc B to release lock2\n");
	sleep(5);
	assert (getprio(wr1) == 20, "Test 13 failed, proc A !=20 after release all lock\n");

        kprintf ("Test 13 OK\n");
	ldelete(lck1);
	ldelete(lck2);
}
/*----------------------------------Test 14---------------------------*/
char output14[20];
int count14;
void reader14 (char msg, int lck)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        output14[count14++]=msg;
        kprintf ("  %c: acquired lock\n", msg);
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void writer14 (char msg, int lck)
{
        kprintf ("  %c: to acquire lock \n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        output14[count14++]=msg; 
        kprintf ("  %c: acquired lock, sleep 5s\n", msg);
	
	sleep (4);
	output14[count14++]=msg;
	kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}
void proc14(char msg)
{
	kprintf ("  %c: running \n",msg);
        output14[count14++]=msg;
}

void test14 ()
{
	count14=0;
        int     lck;
        int     rd1, rd2;
        int     wr1, wr2;
	int	pid, pid2, pid3;

        kprintf("\nTest 14: test the pinh, proc A hold lock 1,\n"
		       " Expect proc A got resched with higher priority when proc A is in ready queue.\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 14 failed");

        rd1 = create(reader14, 2000, 15, "reader C", 2, 'C', lck);
        wr1 = create(writer14, 2000, 5, "writer B", 2, 'B', lck);
        pid = create(proc14, 2000, 10, "proc A", 2, 'A');
        pid2 = create(proc14, 2000, 10, "proc A", 2, 'A');

        kprintf("-start writer A, then sleep 3s. lock 1 & lock2 granted to write A (prio 20)\n");
        resume(pid);
        resume(wr1);
	kprintf("-M sleep 2s, Expect: A -> B -> sleep -> M\n");
        sleep (2);

	resume(rd1);
        resume(pid2);
	kprintf("-wait B become ready\n");

	int	i;
	while( 1 ){
		for(i=0 ;i<1000; i++)
			;
		if( proctab[wr1].pstate == PRREADY )
			break;
	}
	kprintf("-proc B is ready, M sleep 3s\n");
	kprintf("-Expect: C (15, blocked, increase B's pprio) -> -B(5->15) -> C(15) -> A(10) \n");
	sleep (3);
        
	kprintf("output=%s\n", output14);
        assert(mystrncmp(output14,"ABBCA",5)==0,"Test 14 FAILED\n");
        kprintf ("Test 14 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 15---------------------------*/
char output15[20];
int count15;
void reader15 (char msg, int lck)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        output15[count15++]=msg;
        kprintf ("  %c: acquired lock\n", msg);
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void writer15 (char msg, int lck)
{
        kprintf ("  %c: to acquire lock \n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        output15[count15++]=msg; 
        kprintf ("  %c: acquired lock, sleep 5s\n", msg);
	
	sleep (4);
	output15[count15++]=msg;
	sleep (2);
        output15[count15++]=msg;
	kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}
void proc15(char msg)
{
	sleep (2);
	kprintf ("  %c: running \n",msg);
        output15[count15++]=msg;
	sleep (2);
        output15[count15++]=msg;
}

void test15 ()
{
	count15=0;
        int     lck;
        int     rd1, rd2;
        int     wr1, wr2;
	int	pid, pid2, pid3;

        kprintf("\nTest 15: test the pinh, proc A hold lock 1,\n"
		       " Expect proc A downgrade priority(due to kill()) when proc A is in ready queue.\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 15 failed");

        rd1 = create(reader15, 2000, 15, "reader C", 2, 'C', lck);
        wr1 = create(writer15, 2000, 5, "writer B", 2, 'B', lck);
        pid = create(proc15, 2000, 10, "proc A", 1, 'A');
        pid2 = create(proc15, 2000, 10, "proc A", 1, 'A');

        kprintf("-start writer A, then sleep 3s. lock 1 & lock2 granted to write A (prio 20)\n");
        resume(wr1);
	kprintf("-M sleep 2s, Expect: A -> B -> sleep -> M\n");
        sleep (1);

	kprintf("-Expect: C (15, blocked, increase B's pprio)\n");
	resume(rd1);
	resume(pid);
        sleep (1);

	kprintf("-wait B & A become ready\n");

	int	i;
	while( 1 ){
		for(i=0 ;i<1000; i++)
			;
		if( proctab[wr1].pstate == PRREADY && proctab[pid].pstate == PRREADY )
			break;
	}
	kprintf("-A & B are ready, C is blocked. M sleep 1s, Expect B(15) -> A(10) \n");
       	//"	-> -B(5->15) -> C(15) -> A(10) \n");
	assert (getprio(wr1) == 15, "Test 15 failed, proc A !=15");
	sleep (1);
	while( 1 ){
		for(i=0 ;i<1000; i++)
			;
		if( proctab[wr1].pstate == PRREADY && proctab[pid].pstate == PRREADY )
			break;
	}
	kprintf("-A & B are ready, kill C, proc B lower priority(15->5).\n");
	kill(rd1);
	assert (getprio(wr1) == 5, "Test 15 failed, proc A !=5");

	kprintf("-proc A, B is ready, C is blocked. M sleep 5s, Expect A(10) -> B(5) \n");
	sleep (5);
        
	kprintf("output=%s\n", output15);
        assert(mystrncmp(output15,"BBAAB",5)==0,"Test 15 FAILED\n");
        kprintf ("Test 15 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 16---------------------------*/
char output16[20];
int count16;
void reader16 (char msg, int lck)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        output16[count16++]=msg;
        kprintf ("  %c: acquired lock\n", msg);
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void writer16 (char msg, int lck)
{
        kprintf ("  %c: to acquire lock \n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        output16[count16++]=msg; 
        kprintf ("  %c: acquired lock, sleep 5s\n", msg);
	
	sleep (4);
	output16[count16++]=msg;
	sleep (2);
        output16[count16++]=msg;
	kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}
void proc16(char msg)
{
	sleep (2);
	kprintf ("  %c: running \n",msg);
        output16[count16++]=msg;
	sleep (2);
	kprintf ("  %c: running \n",msg);
        output16[count16++]=msg;
}

void test16 ()
{
	count16=0;
        int     lck;
        int     rd1, rd2;
        int     wr1, wr2;
	int	pid, pid2, pid3;

        kprintf("\nTest 16: test the pinh, proc A hold lock 1,\n"
		       " Expect proc A got resched with higher priority when proc A is in ready queue.\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 16 failed");

        rd1 = create(reader16, 2000, 15, "reader C", 2, 'C', lck);
        wr1 = create(writer16, 2000, 5, "writer B", 2, 'B', lck);
        pid = create(proc16, 2000, 10, "proc A", 1, 'A');
        pid2 = create(proc16, 2000, 10, "proc A", 1, 'A');

        kprintf("-start writer A, then sleep 3s. lock 1 & lock2 granted to write A (prio 20)\n");
        resume(wr1);
	kprintf("-M sleep 2s, Expect: A -> B -> sleep -> M\n");
        sleep (1);

	kprintf("-Expect: C (15, blocked, increase B's pprio)\n");
	resume(rd1);
	resume(pid);
        sleep (1);

	kprintf("-wait B & A become ready\n");

	int	i;
	while( 1 ){
		for(i=0 ;i<1000; i++)
			;
		if( proctab[wr1].pstate == PRREADY && proctab[pid].pstate == PRREADY )
			break;
	}
	kprintf("-A & B are ready, C is blocked. M sleep 1s, Expect B(15) -> A(10) \n");
       	//"	-> -B(5->15) -> C(15) -> A(10) \n");
	assert (getprio(wr1) == 15, "Test 16 failed, proc A !=15");
	sleep (1);
	while( 1 ){
		for(i=0 ;i<1000; i++)
			;
		if( proctab[wr1].pstate == PRREADY && proctab[pid].pstate == PRREADY )
			break;
	}
	kprintf("-A & B are ready, delete lock, proc B lower priority(15->5). Expect C(15) -> A(10) -> B(5) \n");
	ldelete(lck);
	assert (getprio(wr1) == 5, "Test 16 failed, proc A !=5");

	kprintf("-proc A, B is ready M sleep 5s, Expect C(15) -> A(10) -> B(5) \n");
	sleep (5);
        
	kprintf("output=%s\n", output16);
        assert(mystrncmp(output16,"BBACAB",5)==0,"Test 16 FAILED\n");
        kprintf ("Test 16 OK\n");
	ldelete(lck);
}
/*----------------------------------Test 17---------------------------*/
void reader17 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer17A (char *msg, int lck1, int lck2)
{
        kprintf ("  %s: to acquire lock1 \n", msg);
        lock (lck1, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock1\n", msg);//sleep 10s\n", msg, lck1);
        
	kprintf ("  %s: to acquire lock2 \n", msg);
        lock (lck2, WRITE, DEFAULT_LOCK_PRIO); 
        kprintf ("  %s: acquired lock2, sleep 10s\n", msg);
        sleep (10);

	kprintf ("  %s: to release lock\n", msg);
        releaseall (2, lck1, lck2);
}
void writer17B (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test17 ()
{
        int     lck1, lck2;
        int     rd1, rd2;
        int     wr1, wr2;

        kprintf("\nTest 17: test the pinh affect by chprio(), proc A hold lock 1, proc B hold lock 2.\n"
		       " And then proc A acquire lock 2 and wait on proc B.\n"
		       " And then proc C acquire lock 1 and wait on proc A.\n"
		       " relationship before kill: proc C -> proc A -> proc B\n");
        lck1  = lcreate ();
        lck2  = lcreate ();
        assert (lck1 != SYSERR, "Test 17 failed");
        assert (lck2 != SYSERR, "Test 17 failed");

        rd1 = create(reader12,  2000, 30, "proc C", 2, "reader C", lck1);
        //rd2 = create(reader12, 2000, 30, "reader12B", 2, "reader B", lck);
        wr1 = create(writer12A, 2000, 25, "proc A", 3, "writer A", lck1, lck2);
        wr2 = create(writer12B, 2000, 20, "proc B", 2, "writer B", lck2);

        kprintf("-start writer B, then sleep 1s. lock2 granted to write B (prio 20)\n");
        resume(wr2);
        sleep (1);

        kprintf("-start writer A, then sleep 1s. lock1 grant to wirte A (prio 25). writer A(prio 25) blocked on the lock2\n");
        resume(wr1);
        sleep (1);
	assert (getprio(wr2) == 25, "Test 17 failed, proc B !=25");

        kprintf("-start reader C, then sleep 1s. reader C (prio 30) blocked on the lock1\n");
        resume (rd1);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 17 failed, proc A !=30");
	assert (getprio(wr2) == 30, "Test 17 failed, proc B !=30");
        
	kprintf("-chprio reader C(30->15), then sleep 1s. Expect proc A(30->25), proc B(30->20)\n");
	chprio ( rd1, 15 );
	assert (getprio(wr1) == 25, "Test 17 failed, proc A !=25");
	assert (getprio(wr2) == 25, "Test 17 failed, proc B !=20");
	
        kprintf("-chprio reader C(15->30), then sleep 1s. Expect proc A(25->35), proc B(20->35)\n");
	chprio ( rd1, 35 );
	assert (getprio(wr1) == 35, "Test 17 failed, proc A !=35");
	assert (getprio(wr2) == 35, "Test 17 failed, proc B !=35");
        
	kprintf("-chprio writer A(25->18), then sleep 1s. Expect proc A & B remain 35 due to pinh from proc C(35)\n");
	chprio ( wr1, 18 );
	assert (getprio(wr1) == 35, "Test 17 failed, proc A !=25");
	assert (getprio(wr2) == 35, "Test 17 failed, proc B !=20");

	kprintf("-kill reader C, then sleep 1s, Expect proc A(35->18), proc B(35->25)\n");
	kill (rd1);
	sleep (1);
	assert (getprio(wr1) == 18, "Test 17 failed, proc A !=25 after kill\n");
	assert (getprio(wr2) == 20, "Test 17 failed, proc B !=25 after kill\n");

	kprintf("-kill writer A, then sleep 1s\n");
	kill (wr1);
	sleep(1);
	assert (getprio(wr2) == 20, "Test 17 failed, proc B !=20 after kill\n");

        sleep (8);
        kprintf ("Test 17 OK\n");
	ldelete(lck1);
	ldelete(lck2);
}
/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

int main( )
{
	failcount = 0;
        /* These test cases are only used for test purposes.
         * The provided results do not guarantee your correctness.
         * You need to read the PA2 instruction carefully.
         */
	// Test 7: lock allocation overflow test
	//test7();
        // Test 8: acquried deleted locks. Expected return SYSERR
	//test8();
	// Test 9: acquried deleted and renewed locks. Expected return SYSERR
	//test9();
	// Test 10: wait on lock and lock been deleted. wait Expected return DELETED, 
	// release deleted lock, Expect return SYSERR.
	//test10();
	//test1();
	//test2();
	// Test 4: wait on locks with same priority but write wait time less than 500 ms. 
	// Expected order of lock acquisition is: reader A, reader B, writer C
	//test4();
	// Test 5: wait on locks with same priority, write wait time longer than 500 ms and longer than read wait time. 
	// Expected order of lock acquisition is: reader A, reader B, writer C
	//test5();
	// Test 6: wait on locks with same priority, write wait time longer than 500 ms but less than read wait time. 
	// Expected order of lock acquisition is: writer D, reader B, writer C
	//test6();
	//test3();
	//test11();
	//test12();
	//test13();
	
	//test14();
	//test15();
	//test16();
	//test17();
	task1();

	if(failcount == 0)
		kprintf("\nALL TEST PASS!\n");
        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call exists the QEMU process.
         */
        shutdown();
}

