/* sendb.c - sendb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  sendb  -  pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	sendb(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* contents of message		*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
        struct	procent *ptcurr;	
	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}
	prptr = &proctab[pid];
	if (prptr->prstate == PR_FREE) {
		restore(mask);
		return SYSERR;
	}
        ptcurr = &proctab[currpid];
        if(prptr->cbfunc != NULL)  {
                *(prptr->abuf) = msg;
                (*prptr->cbfunc)();
                //kprintf("Executed callback function \n\r");
                restore(mask);
                return OK;
        }
        //take care of race conditions
        while(prptr->prmsglen == 3) 
        {
                //kprintf("Mailbox full: %d,%d,%d\n\r",prptr->prsndmsg[0],prptr->prsndmsg[1],prptr->prsndmsg[2]);
                ptcurr->prstate = PRSND;
                //kprintf("adding %d to queue \n\r", currpid);
                //kprintf("firstid %d lastid %d\n\r", firstid(prptr->prsndq), lastid(prptr->prsndq));
                enqueue(currpid,prptr->prsndq);
                resched();
        }
        //kprintf("send msg:%d ,len: %d \n\r",msg,prptr->prmsglen);
	prptr->prsndmsg[prptr->prmsglen] = msg;	/* deliver message		*/
        prptr->prmsglen = prptr->prmsglen + 1;
        //kprintf("buffer[%d]:%d\n\r",prptr->prmsglen-1,prptr->prsndmsg[prptr->prmsglen-1]);
	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECVB) {
		ready(pid, RESCHED_YES);
	} 
        else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid, RESCHED_YES);
	}
	restore(mask);		/* restore interrupts */
	return OK;
}
