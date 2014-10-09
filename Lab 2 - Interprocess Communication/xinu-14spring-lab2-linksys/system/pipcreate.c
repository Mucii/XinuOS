/* pipcreate.c - pipcreate,newpip */

#include <xinu.h>

local	pipid32	newpip(void);

/*------------------------------------------------------------------------
 *  pipcreate  -  create a new pipe and return the ID to the caller
 *------------------------------------------------------------------------
 */
syscall pipcreate()                      /* initial semaphore count	*/
{
	intmask	mask;			/* saved interrupt mask		*/
	pipid32	pip;			/* semaphore ID to return	*/
	mask = disable();

	if ((pip=newpip()) == SYSERR) {
		restore(mask);
		return SYSERR;
	}
        piptab[pip].nextpos = 0;
        piptab[pip].pipstate = PIPE_USED;
        piptab[pip].owner = getpid();
	piptab[pip].supply = semcreate(0);               
        piptab[pip].demand = semcreate(0);  
        //pipedetails(pip);
	restore(mask);
	return pip;
}

/*------------------------------------------------------------------------
 *  newpipe  -  allocate an unused pipe and return its index
 *------------------------------------------------------------------------
 */

local	pipid32	newpip(void)
{
	static pipid32	nextpip = 0;	/* next pipe index to try	*/
	int32 	i;			/* iterate through # entries	*/
        pipid32 pip;
	for (i=0 ; i<NPIPE ; i++) {
	        pip = nextpip++;
                if (nextpip >= NPIPE)
                        nextpip = 0;
		if (piptab[pip].pipstate == PIPE_FREE) {
			piptab[pip].pipstate = PIPE_USED;
			return pip;
		}
	}
	return SYSERR;
}


void pipedetails(pipid32 pip)
{
	intmask mask = disable();
        int i;
	kprintf("PIPE DETAILS:::: pipid : %d  \t", pip);
	kprintf("pipstate %d\t",piptab[pip].pipstate);
	kprintf("pip ownerr: %d \t", piptab[pip].owner);
        kprintf("pip reader: %d \t", piptab[pip].rdend);
	kprintf("pip writer : %d \t", piptab[pip].wrtend);
        kprintf("pip nextpos: %d \t", piptab[pip].nextpos);
        kprintf("pip buffer : \t");
        for (i=0;i<piptab[pip].nextpos;i++) {
              kprintf("%c", (piptab[pip].buffer[i]));
        }
	kprintf("supply state: %d %c ",semtab[piptab[pip].supply].scount,semtab[piptab[pip].supply].sstate);
	kprintf("demand state : %d %c \n\r",semtab[piptab[pip].demand].scount,semtab[piptab[pip].demand].sstate);
	restore(mask);
}

