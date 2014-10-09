/* getmemgb.c - getmemgb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getmemgb  -  Allocate heap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char  	*getmemgb(
	  uint32	nbytes		/* size of memory requested	*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	memblk	*prev, *curr, *leftover;
         
        struct  memblkgb *prevgb, *nextgb, *blockgb, *tempgb; 
        struct procent* prptr = &proctab[currpid];
        uint32 validblk = 0;
	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	//nbytes = (uint32) roundmb(nbytes);	/* use memblk multiples	*/
        nbytes = (uint32) roundmb(nbytes);// + sizeof(struct memblkgb);

	prev = &memlist;
	curr = memlist.mnext;
	while (curr != NULL) {			/* search free list	*/

		if (curr->mlength == nbytes) {	/* block is exact match	*/
			prev->mnext = curr->mnext;
			memlist.mlength -= nbytes;
                        validblk = 1;
			//restore(mask);
			//return (char *)(curr);
                        break;
		} else if (curr->mlength > nbytes) { /* split big block	*/
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			memlist.mlength -= nbytes;
                        validblk = 1;
			//restore(mask);
			//return (char *)(curr);
                        //kprintf("validblk \n\r");
                        break;
		} else {			/* move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
        kprintf("curr length %d, nbytes :%d\n\r", curr->mlength,nbytes);
        
        if(curr != NULL)
        {
                blockgb = (struct  memblkgb *)curr;
                //kprintf("curr %x, blockgb %x, nbytes %d\n\r", curr,blockgb,nbytes );
                if (prptr->prmemlistgb == NULL || (blockgb < prptr->prmemlistgb)) {
                        tempgb = (struct  memblkgb *)blockgb;
                        tempgb->mnext = prptr->prmemlistgb; 
                        tempgb->mlength = nbytes;
                        prptr->prmemlistgb = tempgb;
                        usedspace += nbytes;
                        kprintf("adding to the head \n\r");
                        restore(mask);
		        return (char *)(curr);
                        
                }
                else {
                prevgb = prptr->prmemlistgb;	/* walk along the process' alloted list	*/
	        nextgb = (prptr->prmemlistgb)->mnext;
                uint32 topgb;
	        while ((nextgb != NULL) && (nextgb < blockgb)) {
		        prevgb = nextgb;
		        nextgb = nextgb->mnext;
	        }

	        //if (prevgb == prptr->prmemlistgb) {		/* compute top of previous block*/
		//        topgb = (uint32) NULL;
	        //} else {
		        topgb = (uint32) prevgb + prevgb->mlength;
	        //}

	        /* Insure new block does not overlap previous or next blocks	*/

        	if (((prevgb != prptr->prmemlistgb) && (uint32) blockgb < topgb)
        	    || ((nextgb != NULL)	&& (uint32) blockgb+nbytes>(uint32)nextgb)) {
		        restore(mask);
		        return (char *)SYSERR;
	        }
	        usedspace += nbytes;
                //kprintf("prevgb %x prevgb->length %d blockgb %x topgb %x blockgb->nxt %x nbytes %d",prevgb, prevgb->mlength,blockgb, topgb, blockgb+nbytes,nbytes);
                tempgb = blockgb;
                tempgb->mnext = nextgb;
	        tempgb->mlength = nbytes;
		prevgb->mnext = tempgb;
	       /* Coalesce with next block if adjacent */

	       /*if (((uint32) blockgb + blockgb->mlength) == (uint32) nextgb) {
		       blockgb->mlength += nextgb->mlength;
		       blockgb->mnext = nextgb->mnext;
	       }*/
               }
               if(validblk == 1) {
                       //kprintf("getmem address %x",curr);
                       restore(mask);
		       return (char *)(curr);
               }
         }
	 restore(mask);
	 return (char *)SYSERR;
}
