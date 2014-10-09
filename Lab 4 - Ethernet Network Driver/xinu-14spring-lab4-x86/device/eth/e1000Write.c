/* e1000Write.c - e1000Write */

#include <xinu.h>

/*------------------------------------------------------------------------
 * e1000Write - write a packet to an E1000E device
 *------------------------------------------------------------------------
 */
devcall	e1000Write(
	struct	dentry	*devptr, 	/* entry in device switch table	*/
	void	*buf,			/* buffer that holds a packet	*/
	uint32	len			/* length of buffer		*/
	)
{
	struct  ether   *ethptr;        
        uint32 tail;
        uint32 tdt;
        struct e1000_tx_desc *txdescptr;   /* ptr to transmit ring descriptor       */
        char    *pktptr;
        ethptr = &ethertab[devptr->dvminor];

        if ((ethptr->state != ETH_STATE_UP)
                        || (len < ETH_HDR_LEN)
                        || (len > ETH_MAX_PKT_LEN) ) {
                return SYSERR;
        }
        if(len < 17) {
                kprintf("length should be more than 16 bytes \n\r");
        }
        /* Wait for a free ring slot */
        wait(ethptr->osem);
        //kprintf("before sending a packet \n\r");
        tail = ethptr->txTail;
        txdescptr = (struct e1000_tx_desc *)ethptr->txRing + tail;

        /* Copy packet to transmit ring buffer */
        
        pktptr = (char *)((uint32)txdescptr->buffer_addr & ADDR_BIT_MASK);
        memcpy(pktptr, buf, len);

        /* Insert transmitting command and length */
        
        txdescptr->lower.data &= E1000_TXD_CMD_DEXT; 
        txdescptr->lower.data = E1000_TXD_CMD_IDE |
                              E1000_TXD_CMD_RS | 
                              E1000_TXD_CMD_IFCS |
                              E1000_TXD_CMD_EOP |
                              len;
        txdescptr->upper.data = 0;

        /* Add descriptor by advancing the tail pointer */
        
        tdt = e1000_io_readl(ethptr->iobase, E1000_TDT(0));
        tdt = (tdt + 1) % ethptr->txRingSize;
        e1000_io_writel(ethptr->iobase, E1000_TDT(0), tdt);

        /* Advance the ring tail pointing to the next available ring    */
        /*      descriptor                                              */
        
        ethptr->txTail = (ethptr->txTail + 1) % ethptr->txRingSize;

        return len;
}
