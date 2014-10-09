/* chprio.c - chprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chprio  -  Change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
pri16	chprio(
	  pid32		pid,		/* ID of process to change	*/
	  pri16		newprio		/* new priority			*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	pri16	oldprio;		/* priority to return		*/

	mask = disable();
	if (newprio <= 0) {
	        restore(mask);
		return (pri16) SYSERR;
	}
	if (isbadpid(pid)) {
		restore(mask);
		return (pri16) SYSERR;
	}
	prptr = &proctab[pid];
	if(prptr->prGroup == PROPORTIONALSHARE) {
	   oldprio = prptr->prPsRate;
	   prptr->prPsRate = newprio;
	   restore(mask);
	   return oldprio;
	}
	oldprio = prptr->prprio;
	prptr->prprio = newprio;
	restore(mask);
	return oldprio;
}
