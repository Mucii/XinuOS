/* receiveb.c - receiveb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receiveb  -  wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receiveb(void)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	umsg32	msg;			/* message to return		*/
        int32 i;
	mask = disable();
	prptr = &proctab[currpid];
        //kprintf("receving\n\r");
        while (prptr->prmsglen == 0)  {
                //kprintf("Mailbox empty\n\r");
		prptr->prstate = PR_RECVB;
		resched();		/* block until message arrives	*/
	}
        //printbuf(currpid);
        msg = prptr->prsndmsg[0];       /* retrieve message		*/
        //kprintf("Received message %d, len: %d\n\r",prptr->prsndmsg[0],prptr->prmsglen);
        for(i=1;i<=(prptr->prmsglen-1);i++)  {
              prptr->prsndmsg[i-1] = prptr->prsndmsg[i];
              //kprintf("::%d::",prptr->prsndmsg[i-1]);
        }
        prptr->prmsglen = prptr->prmsglen - 1;
        if(nonempty(prptr->prsndq))  {

              //kprintf("\n\r releasing %d from queue;;lastid %d\n\r", firstid(prptr->prsndq), lastid(prptr->prsndq));
              ready(dequeue(prptr->prsndq), RESCHED_YES);
        }
	restore(mask);
	return msg;
}
