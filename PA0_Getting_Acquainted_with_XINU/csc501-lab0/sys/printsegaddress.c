#include <stdio.h>

extern unsigned long _etext,_edata,_end;

void printsegaddress()
{
	unsigned long *petext,*pedata,*pend;
	petext = &_etext;
	pedata = &_edata;
	pend = &_end;

	kprintf("\n\nvoid printsegaddress()\n");
	kprintf("\nCurrent: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",petext,*petext,pedata,*pedata,pend,*pend);
	kprintf("\nPreceding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",petext-1,*(petext - 1),pedata-1,*(pedata-1),pend-1,*(pend-1));
	kprintf("\nAfter: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x",petext+1,*(petext+1),pedata+1,*(pedata+1),pend+1,*(pend+1));
}
