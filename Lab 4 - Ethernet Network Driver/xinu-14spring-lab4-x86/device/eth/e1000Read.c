/* e1000Read.c - e1000Read */

#include <xinu.h>

/*------------------------------------------------------------------------
 * e1000Read - read a packet from an E1000E device
 *------------------------------------------------------------------------
 */
devcall	e1000Read(
	struct	dentry	*devptr,	/* entry in device switch table	*/
	void	*buf,			/* buffer to hold packet	*/
	uint32	len			/* length of buffer		*/
	)
{
        struct  ether   *ethptr; 
	struct  e1000_rx_desc *rxdescptr;/* ptr to receive ring descriptor       */
        char    *pktptr;                /* ptr used during packet copy  */
        uint32  head;                   /* head of ring buffer          */
        uint32  status;                 /* status of entry              */
        uint32  length;                 /* packet length                */
        int32   retval;
        uint32  rdt;
        ethptr = &ethertab[devptr->dvminor];
        /* Wait for a packet to arrive */
        //kprintf("1-inside e1000Read, isem %d\n\r",ethptr->isem);
        if ((ETH_STATE_UP != ethptr->state) || (len < ETH_HDR_LEN)) {
                return SYSERR;
        }
        wait(ethptr->isem);
        //kprintf("2 - inside e1000Read\n\r");
        /* Find out where to pick up the packet */

        head = ethptr->rxHead;
        rxdescptr = (struct e1000_rx_desc *)ethptr->rxRing + head;
        status = rxdescptr->status;

        if (!(status & E1000_RXD_STAT_DD)) {   
                kprintf("error in e1000Read packet\n");
                retval = SYSERR;
        } else {                                
                pktptr = (char *)((uint32)(rxdescptr->buffer_addr &
                                           ADDR_BIT_MASK));
                length = rxdescptr->length;
                memcpy(buf, pktptr, length);
                retval = length;
        }
        /* Clear up the descriptor and the buffer */

        rxdescptr->length = 0;
        rxdescptr->csum = 0;
        rxdescptr->status = 0;
        rxdescptr->errors = 0;
        rxdescptr->special = 0;
        memset((char *)((uint32)(rxdescptr->buffer_addr & ADDR_BIT_MASK)), 
                        '\0', ETH_BUF_SIZE); 

        /* Add newly reclaimed descriptor to the ring */

        if (ethptr->rxHead % E1000_RING_BOUNDARY == 0) {
                rdt = e1000_io_readl(ethptr->iobase, E1000_RDT(0));
                rdt = (rdt + E1000_RING_BOUNDARY) % ethptr->rxRingSize;
                e1000_io_writel(ethptr->iobase, E1000_RDT(0), rdt);
        }

        /* Advance the head pointing to the next ring descriptor which  */
        /*      will be ready to be picked up                           */
        ethptr->rxHead = (ethptr->rxHead + 1) % ethptr->rxRingSize;
        //kprintf("inside e1000Read\n\r");
        return retval;
}
