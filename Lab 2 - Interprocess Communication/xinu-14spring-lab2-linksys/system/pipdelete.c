/* pipdelete.c - pipdelete */

#include <xinu.h>

/*------------------------------------------------------------------------
 * pipdelete  --  Delete a pipe by releasing its table entry
 *------------------------------------------------------------------------
 */
syscall	pipdelete(
	  pipid32		pip	/* ID of the pipe to delete	*/
	)
{
	intmask mask;			/* saved interrupt mask		*/
	struct	pipentry *pipptr;	/* ptr to pipaphore table entry	*/
        kprintf("inside pipdelete \n\r");
	mask = disable();
	if (isbadpip(pip)) {
		restore(mask);
		return SYSERR;
	}
	pipptr = &piptab[pip];
	if (pipptr->owner != getpid()) {
 		restore(mask);
		return SYSERR;
	}
	/*if (pipptr->pipstate == S_FREE) {
		restore(mask);
		return SYSERR;
	}*/
	pipptr->nextpos = -1; 
        pipptr->pipstate = PIPE_FREE;
        pipptr->rdend = -1;
        pipptr->wrtend = -1;
        semdelete(pipptr->supply);
        semdelete(pipptr->demand);
	restore(mask);
	return OK;
}
