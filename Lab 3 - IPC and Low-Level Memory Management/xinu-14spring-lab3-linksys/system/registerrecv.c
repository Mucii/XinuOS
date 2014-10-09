/* registerrecv.c - registerrecv */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  registerrecv  -  wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
syscall	registerrecv(umsg32 *abuf, int (*cbfunc)(void))
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/

	mask = disable();
	if ((abuf == NULL) || (cbfunc == NULL)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[currpid];
        //kprintf("registering pid %d with %d function", currpid, *cbfunc);
	prptr->abuf = abuf;
        prptr->cbfunc = cbfunc;
	restore(mask);		/* restore interrupts */
	return OK;
}
