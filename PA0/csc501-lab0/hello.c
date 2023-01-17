#include <stdio.h>

long zfunction(long para);


int main(void) {
	long x=0xaabbccdd;
    printf("zfunction(0xaabbccdd)=%lx\n",zfunction(x));
    return 0;
}


long zfunction(long para){
	return (para<<4) & 0xFC01FFF0;
}
