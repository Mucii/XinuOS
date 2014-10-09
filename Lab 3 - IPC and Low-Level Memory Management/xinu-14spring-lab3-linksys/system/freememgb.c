/* freememgb.c - freememgb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  freememgb  -  Free a memory block, returning the block to the free list
 *------------------------------------------------------------------------
 */
syscall	freememgb(
	  char		*blkaddr,	/* pointer to memory block	*/
	  uint32	nbytes		/* size of block in bytes	*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	uint32	top,i,flag=0;
        struct memblk *next, *prev, *block;
        struct procent *prptr;// = &proctab[currpid];
        struct memblkgb *prevgb, *currgb, *blockgb;
	mask = disable();
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) minheap)
			  || ((uint32) blkaddr > (uint32) maxheap)) {
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes); //+ sizeof(struct memblkgb *);	/* use memblk multiples	*/
	block = (struct memblk *)blkaddr;
         
	prev = &memlist;			/* walk along free list	*/
	next = memlist.mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == &memlist) {		/* compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Insure new block does not overlap previous or next blocks	*/

	if (((prev != &memlist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	memlist.mlength += nbytes;
        kprintf("freemem %x, next %x, nbytes %d\n\r",block,next,nbytes);
        struct memblkgb *curr = NULL;

        /*if(prptr->prmemlistgb == NULL) {
                kprintf("empty list \n\r");
      	        restore(mask);
	        return SYSERR;
        }*/
        for (i = 0; i < NPROC; i++) {
        prptr = &proctab[i];
        if((prptr->prstate == PR_FREE)||(prptr->prmemlistgb == NULL)) {
                continue;
        }
        prevgb = (prptr->prmemlistgb);
	currgb = (prptr->prmemlistgb)->mnext;
        blockgb = (struct memblkgb *)blkaddr;
        if((blockgb == prptr->prmemlistgb)&&(blockgb->mlength == nbytes)) {
                prptr->prmemlistgb = (prptr->prmemlistgb)->mnext;
                usedspace-=nbytes;
                kprintf("freed up head %x\n\r",blockgb);
                flag = 1;
                //restore(mask);
                //return OK;
        }  
        if((blockgb != prptr->prmemlistgb) && (flag == 0)){   
	while (currgb != NULL) {			/* search alloted list	*/

                kprintf("currgb->mlength %d, currgb %x, blockgb %x \n\r",currgb->mlength,currgb,blockgb);
                if((currgb->mlength == nbytes) && (currgb == blockgb)) {
                        prevgb->mnext = currgb->mnext;
                        usedspace-=nbytes;
                        kprintf("freed up %d, usedspace : %d\n\r",currgb->mlength,usedspace);
                        flag = 1;
                        break;
                        //restore(mask);
                        //return OK;
                }
                else {			               /* move to next block	*/
			prevgb = currgb;
			currgb = currgb->mnext;
		}
	}
        }
        if(flag == 1) {
                break;
        }
        } //end of for
        if((prptr->prmemlistgb != NULL) && (currgb == NULL)) {
                 kprintf("currgb null\n\r");
	         restore(mask);
	         return SYSERR;
        }
       /* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
        }else {			/* link into list as new node	*/
                
		/*kprintf("<1> <1> <1> blokgb %x\n\r",blockgb);
	        if(prptr->prstate != PR_FREE)
	        {
		kprintf("head %x\n\r",(prptr->prmemlistgb));
		curr = prptr->prmemlistgb;
		while(curr != NULL)
		{
			kprintf("%x with length %d & nxt %x\n\r", curr, curr->mlength,curr->mnext);
	                curr = curr->mnext;
		}
	        }
	        kprintf("<1> <1> <1>\n\r");*/ 

		block->mnext = next;
                
		kprintf("<1a> <1a> <1a>\n\r");
	        if(prptr->prstate != PR_FREE)
	        {
		kprintf("head %x\n\r",(prptr->prmemlistgb));
		curr = prptr->prmemlistgb;
		while(curr != NULL)
		{
			kprintf("%x with length %d & nxt %x\n\r", curr, curr->mlength,curr->mnext);
	                curr = curr->mnext;
		}
	        }
	        kprintf("<1> <1> <1>\n\r");

		block->mlength = nbytes;
                
		/*kprintf("<1b> <1b> <1b>\n\r");
	        if(prptr->prstate != PR_FREE)
	        {
		kprintf("head %x\n\r",(prptr->prmemlistgb));
		curr = prptr->prmemlistgb;
		while(curr != NULL)
		{
			kprintf("%x with length %d & nxt %x\n\r", curr, curr->mlength,curr->mnext);
	                curr = curr->mnext;
		}
	        }
	        kprintf("<1> <1> <1>\n\r");*/ 

		prev->mnext = block;	 

        kprintf("<2> <2> <2>\n\r");
	if(prptr->prstate != PR_FREE)
	{
		kprintf("head %x\n\r",(prptr->prmemlistgb));
		curr = prptr->prmemlistgb;
		while(curr != NULL)
		{
			kprintf("%x with length %d & nxt %x\n\r", curr, curr->mlength,curr->mnext);
	                curr = curr->mnext;
		}
	}
	kprintf("<2> <2> <2>\n\r");
	}
       
	
	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
        /*if(currgb == NULL) {
                 kprintf("currgb null\n\r");
	         restore(mask);
	         return SYSERR;
        }*/
               	 restore(mask);
	         return OK;
}
