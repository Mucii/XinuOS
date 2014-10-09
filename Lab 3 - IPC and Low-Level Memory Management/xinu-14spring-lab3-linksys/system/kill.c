/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	int32	i;			/* index into descriptors	*/
        uint32 len=0;
        struct memblkgb *currgb, *nextgb;
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

        /* free up the allocated heap memory for the process  */
        nextgb = prptr->prmemlistgb;
        kprintf("memlength before %d \n\r",memlist.mlength);
        while(nextgb!= NULL)
	{
		currgb = nextgb;
		nextgb = currgb->mnext;
		//blkaddr = (char*)currgb+ sizeof(struct memblkgb); 
		freememgb((char *)currgb,currgb->mlength);
		kprintf("kill currgb length %d\n\r",currgb->mlength);
                len += currgb->mlength;
	}  
        kprintf("Freed up %d space for the process %d, memlength:%d\n\r", len,pid,memlist.mlength);

	if (--prcount <= 1) {		/* last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);
        kprintf("memlength after freestk %d \n\r",memlist.mlength);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* fall through */

	case PR_READY:
		getitem(pid);		/* remove from queue */
		/* fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
