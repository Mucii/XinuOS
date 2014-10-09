/* resched.c - resched */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* ptr to table entry for old process	*/
	struct procent *ptnew;	/* ptr to table entry for new process	*/
	int pscount=0;          /* count of PS processes in ready queue */
	int tscount=0;          /* count of TS processes in ready queue */
	int schedGp=1;          /* which group to schedule next         */  
	pri16 MAX = 0x7FFF;
        pid32 pidsched;         /* pid of the next process              */
	struct procent *ptsched;/* prtable entry of the next process    */
	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */ 
	/* Get the count of TS and PS processes in the ready queue    */
	ptold = &proctab[currpid];
	pid32 curr = firstid(readylist);
	while (queuetab[curr].qkey > 0)
	{
	     struct procent *prcurr = &proctab[curr];
	     if (prcurr->prGroup == PROPORTIONALSHARE) { 
	                pscount+=1;
	     }
	     if (prcurr->prGroup == TSSCHED) {
	                tscount+=1;
	     }
	     curr = (queuetab[curr]).qnext;
	}     
	/* Update the current priority and group priorities after determining the group of the current process. Null process is igonred here */             
	if ((ptold->prGroup == PROPORTIONALSHARE) && (currpid > 0)) {
	     psGpPrio = initPsPrio;
	     tsGpPrio = tsGpPrio+tscount;
	     psGpPrio = psGpPrio+pscount;
             int currtick = clktime*CLKTICKS_PER_SEC + clkticks;
             pri16 prio = ptold->prPsPrio + (pri16)((currtick - ptold->prStartTime)*100/ptold->prPsRate);
	     ptold->prPsPrio = prio;
	     ptold->prprio = MAX - ptold->prPsPrio;
	}

	if (ptold->prGroup == TSSCHED) {
             tsGpPrio = initTsPrio;
	     tsGpPrio = tsGpPrio+tscount;
	     psGpPrio = psGpPrio+pscount;
             pri16 prio = ptold->prprio + 1;
	     if((preempt != QUANTUM) || ((ptold->prstate != PR_FREE) && (ptold->prstate != PR_CURR)) )
	     {  
	        if(prio >= 0){ 
		   ptold->prprio = prio;
		}
		else {
		   prio = INITPRIO;
		   ptold->prprio = prio;
		}   
	     }
	}
	if (psGpPrio >= tsGpPrio) {
	     schedGp = PROPORTIONALSHARE;
	}
	else { 
	     schedGp = TSSCHED;
        }

        /* cases in which processes in only 1 group or only null process is present */
	if ((pscount == 0) && (tscount > 0)) { 
	     schedGp = TSSCHED;
	}
	if ((tscount == 0) && (pscount > 0)) {
	     schedGp = PROPORTIONALSHARE;
	}
	if (((ptold->prGroup == TSSCHED) && (ptold->prstate == PR_CURR))&&(pscount == 0)) {
	     schedGp = TSSCHED;
	}

	curr = firstid(readylist);
	pidsched = curr;
	ptsched = &proctab[curr];
        /* Find the first process from the ready queue belonging to the next scheduling group */
	while ( (queuetab[curr].qkey > 0) && (((&proctab[curr])->prGroup)!=schedGp) )
	{
	     curr = (queuetab[curr]).qnext;
	     pidsched = curr;
	     ptsched = &proctab[curr];
	}
	//kprintf("**out while curr : %d prname : %s, prgroup : %d ,prprio : %d, pidsched : %d\n\r",curr,ptsched->prname,ptsched->prGroup,ptsched->prprio,pidsched);
        	
	if (ptold->prstate == PR_CURR) {  /* process remains running */
		//if (ptold->prprio > firstkey(readylist)) {
		if ((ptold->prGroup == schedGp) && (ptold->prprio > ptsched->prprio)) {
			return;
		}
                  
		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */
        //currpid = dequeue(readylist);
	/* Get the process to be scheduled next                  */
	if ((!isbadpid(pidsched)) && (pidsched < NPROC))
	{
	   currpid = getitem(pidsched);
	}
	else {
	   pidsched = 0;
	   dequeue(readylist);
	}
	ptnew = &proctab[pidsched];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* reset time slice for process	*/
	/* lab1 +1 */ 
	ptnew->prStartTime = (uint32)((clktime*CLKTICKS_PER_SEC)+clkticks);
	/* If the process is scheduled for the first time */
	if ((ptnew->prPsPrio == 0) && (ptnew->prGroup == PROPORTIONALSHARE)) {
	    ptnew->prPsPrio = clktime*CLKTICKS_PER_SEC + clkticks;
	    ptnew->prprio = MAX - ptnew->prPsPrio;
	 }
	 /* If the new process is scheduled after blocking */
	 if ((ptnew->isBlocked) && (ptnew->prGroup == PROPORTIONALSHARE)) {
	    //kprintf("Scheduling a blocked PS Process %s :maxkey-prio = %d \r\n",ptnew->prname, (MAX - ptnew->prPsPrio));
	    ptnew->prprio = MAX - ptnew->prPsPrio;
	    ptnew->isBlocked = 0;
	 }

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	/* Old process returns here when resumed */

	return;
}

