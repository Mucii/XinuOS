/* chgprio.c - chprio */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  chgprio  -  Change the scheduling group priority
 *------------------------------------------------------------------------
 */
pri16	chgprio(
	  int		group,		/* group num of group to change	*/
	  pri16		newprio		/* new priority			*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	pri16	oldprio;		/* priority to return		*/

	mask = disable();
	if (isbadgroup(group)) {
		restore(mask);
		kprintf("Enter a valid group \n\r");
		return (pri16) SYSERR;
	}
	if ((int)newprio <= 0) {
	        restore(mask);
		kprintf("group priority should be greater than 0 and less than 0x7FFF \n\r");
		return (pri16) SYSERR;
	}
	if (group == PROPORTIONALSHARE) {
	        oldprio = initPsPrio;
		initPsPrio = newprio;
		kprintf("new Initial PSgroup priority %d \n\r",initPsPrio);
		restore (mask);
		return oldprio;
	}
	else { 
	        oldprio = initTsPrio;
		initTsPrio = newprio;
		kprintf("new Initial TSgroup priority %d \n\r",initTsPrio);
		restore (mask);
		return oldprio;
        }
}
