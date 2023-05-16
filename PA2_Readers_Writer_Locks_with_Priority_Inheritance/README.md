# PA2: Readers/Writer Locks with Priority Inheritance

## 1. 1. Introduction
In PA2, you are going to implement readers/writer locks as described in this handout. Additionally, you will implement a
priority inheritance mechanism to prevent the priority inversion problem when using locks.<br>
Please download and untar a fresh version of the XINU for QEMU source at (csc501-lab2-qemu.tgz (https://moodlecourses2223.wolfware.ncsu.edu/pluginfile.php/1237269/mod_assign/introattachment/0/csc501-lab2-qemu.tgz?
forcedownload=1)).<br>
Readers/writer locks are used to synchronize access to a shared data structure. A lock can be acquired for read or write
operations. A lock acquired for reading can be shared by other readers, but a lock acquired for writing must be exclusive.<br>

You have been provided with the standard semaphore implementation for XINU. Make sure you read and understand the
XINU semaphore system (wait.c, signal.c, screate.c, sdelete.c, etc.) and use that as a basis for your locks. You should NOT
modify the standard semaphore implementation, since semaphores are used in the rest of the kernel, e.g., in device
drivers. Your task is to extend the XINU semaphore to implement the readers/writer lock semantics. Although, the standard
semaphores implemented in XINU are quite useful, there are some issues with the XINU semaphores which we will try to
fix in this assignment.

XINU semaphores do not distinguish between read accesses, which can co-exist, and write accesses, which must be
exclusive. Another problem with XINU’s semaphores occurs when a semaphore is deleted at a time when it has processes waiting in
its queue. In such situation, sdelete awakens all the waiting processes by moving them from the semaphore queue to the
ready list. As a result, a process that is waiting for some event to occur will be awakened, even though the event has not
yet occurred. You need to fix this problem in this PA.<br>

Yet another problem that occurs due to the interactions between process synchronization and process scheduling is
priority inversion. Priority inversion occurs when a higher priority thread is blocked waiting on a lock (or a semaphore) held
by a lower priority thread. This can lead to erroneous system behavior, especially in real time systems.<br>

There are many solutions in the literature to solve the problem of priority inversion. In this lab, you will implement one such
solution discussed in our lectures: priority inheritance protocol for locks. Other solutions, for example, can be found
onWikipedia (https://en.wikipedia.org/wiki/Priority_inversion).

## 2. Interfaces to Implement
Basic Locks<br>

For this lab you must implement the entire readers/writer lock system. This includes code or functions to:<br>
\> initialize locks (call a function linit() from the sysinit() function in initialize.c)
\> create and destroy a lock (lcreate and ldelete)
\> acquire a lock and release multiple locks (lock and releaseall)
Please create files called linit.c, lcreate.c, ldelete.c, lock.c and releaseall.c that contain these functions. Use a header file
called lock.h for your definitions, including the constants DELETED, READ andWRITE. The functions have to be
implemented as explained next:
\> Create a lock: int lcreate (void) – Creates a lock and returns a lock descriptor that can be used in further calls to refer to
this lock. This call should return SYSERR if there are no available entries in the lock table. The number of locks allowed is
NLOCKS, which you should define in lock.h to be 50.
\> Destroy a lock: int ldelete (int lockdescriptor) – Deletes the lock identified by the descriptor lockdescriptor. (see “Lock
Deletion” below)
\> Acquisition of a lock for read/write: int lock (int ldes1, int type, int priority) –  This call is explained below (“Wait on locks
with Priority”).
\> Simultaneous release of multiple locks: int releaseall (int numlocks, int ldes1,…, int ldesN)
