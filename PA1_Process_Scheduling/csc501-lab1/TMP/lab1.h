/* lab1.h - define schedule macro and schedclass function */

#ifndef _LAB1_H_
#define _LAB1_H_

#define AGESCHED 1 
#define LINUXSCHED 2

extern void setschedclass (int sched_class);

extern int getschedclass();

#endif
