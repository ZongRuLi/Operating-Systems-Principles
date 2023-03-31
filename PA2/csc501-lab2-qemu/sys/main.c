#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>

#include <stdio.h>

#include <q.h>

#define DEFAULT_LOCK_PRIO 20
#define NLOCKS 50
#define assert(x,error) if(!(x)){ \
            totalfailed += 1;\
            kprintf(error);\
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

int totalpassed =0;
int totalfailed =0;

char output8[6];
int count8;
void reader8 (char i, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock, ctr1000 = %d ms\n", i, ctr1000);
        lock (lck, READ, lprio);
        output8[count8++]=i;
        kprintf ("  %c: acquired lock, sleep 3s\n", i);
        sleep (3);
        kprintf ("  %c: to release lock\n", i);
        output8[count8++]=i;
        releaseall (1, lck);
        
}

void writer8 (char i, int lck, int lprio)
{
        kprintf ("  %c: to acquire lock, ctr1000 = %d ms\n", i, ctr1000);
        lock (lck, WRITE, lprio);
        output8[count8++]=i;
        kprintf ("  %c: acquired lock, sleep 4s\n", i);
        sleep (4);
        kprintf ("  %c: to release lock\n", i);
        output8[count8++]=i;
        releaseall (1, lck);
        
}

void test8 ()
{
        int     lck;
        int     rd1;
        int     wr1,wr2;

        count8 = 0;
        kprintf("\nTest 8: wait on locks with equal priority, waiting time diff > 0.5s. Expected order of \n"
        "lock acquisition is: writer C, writer B, reader A\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 8 FAILED\n");

        rd1 = create(reader8, 2000, 20, "reader8", 3, 'A', lck, 30);
        wr1 = create(writer8, 2000, 20, "writer8", 3, 'B', lck, 30);
        wr2 = create(reader8, 2000, 20, "reader8", 3, 'C', lck, 25);
    
        kprintf("-start reader C, then sleep 1s. lock granted to reader A\n");
        resume(wr2);
	kprintf("-sleep 1s\n");
	sleep(1);

        kprintf("-start writer B, then sleep 2s. writer waits for the lock\n");
        resume (wr1);
	kprintf("-sleep 2s\n");
        sleep(2);
        kprintf("-start reader A, then sleep 2s. reader waits for the lock\n");
        resume (rd1);


	kprintf("-sleep 10s\n");
        sleep (10);
        ldelete (lck);
        kill(rd1);kill(wr2);kill(wr1);
        kprintf("Output is %s\n",output8);
        assert(strncmp(output8,"CCBBAA",6)==0,"Test 8 FAILED\n");
        kprintf ("Test 8 PASSED\n");
        totalpassed += 1;

}
/*----------------------------------Test 8 reference---------------------------*/
void reader8_reference (char i, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock, ctr1000 = %d ms\n", i, ctr1000);
        lock (lck, READ, lprio);
        output8[count8++]=i;
        kprintf ("  %c: acquired lock, sleep 0.7s\n", i);
        sleep (7);// increase sleep time 3 -> 7
        kprintf ("  %c: to release lock\n", i);
        output8[count8++]=i;
        releaseall (1, lck);
        
}
void writer8_reference (char i, int lck, int lprio)
{
        kprintf ("  %c: to acquire lock, ctr1000 = %d ms\n", i, ctr1000);
        lock (lck, WRITE, lprio);
        output8[count8++]=i;
        kprintf ("  %c: acquired lock, sleep 0.4s\n", i);
        sleep (4);
        kprintf ("  %c: to release lock\n", i);
        output8[count8++]=i;
        releaseall (1, lck);
        
}

void test8_reference ()
{
        int     lck;
        int     rd1;
        int     wr1,wr2;

        count8 = 0;
        kprintf("\nTest 8 reference: wait on locks with equal priority, waiting time diff > 0.5s. Expected order of \n"
        "lock acquisition is: writer C, writer B, reader A\n");
        lck  = lcreate ();
        assert (lck != SYSERR,"Test 8 FAILED\n");

        rd1 = create(reader8_reference, 2000, 20, "reader8", 3, 'A', lck, 30);
        wr1 = create(writer8_reference, 2000, 20, "writer8", 3, 'B', lck, 30);
        wr2 = create(reader8_reference, 2000, 20, "reader8", 3, 'C', lck, 25);
    
        kprintf("-start reader C, then sleep 0.1s. lock granted to reader A\n");
        resume(wr2);
	kprintf("-sleep 0.1s\n");
	sleep(1);

        kprintf("-start writer B, then sleep 5.01s. writer waits for the lock\n");
        resume (wr1);
	kprintf("-sleep 0.51s\n");
        sleep10 (51);
        kprintf("-start writer A, then sleep 1.2s. writer waits for the lock\n");
        resume (rd1);


	kprintf("-sleep 1.2s\n");
        sleep (12); // increase sleep time 10 -> 12
	kprintf("-ctr1000 = %d\n", ctr1000);

        ldelete (lck);
        kill(rd1);kill(wr2);kill(wr1);
        kprintf("Output is %s\n",output8);
        assert(strncmp(output8,"CCBBAA",6)==0,"Test 8 FAILED\n");
        kprintf ("Test 8 PASSED\n");
        totalpassed += 1;

}



int main( )
{

    test8();
    test8_reference();
    //kprintf("\n--------------SUMMARY------------\n");
    //kprintf("PASSED: %d, FAILED: %d, TOTAL: %d\n",totalpassed,totalfailed,totalpassed+totalfailed);
    //kprintf("Lost %d points\n", totalfailed*5);
//     kprintf("Score = %d\n",60-totalfailed*5);
        
    /* The hook to shutdown QEMU for process-like execution of XINU.
     * This API call exists the QEMU process.
     */
    shutdown();
}




