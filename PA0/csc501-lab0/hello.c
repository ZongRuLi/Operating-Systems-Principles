#include <stdio.h>

long zfunction(long para);


int main(void) {
    printf("zfunction(0xaabbccdd)=%lx\n",zfunction(0xaabbccdd));
    return 0;
}


long zfunction(long para){
	return (para<<4) & 0xFC01FFF0;
}
