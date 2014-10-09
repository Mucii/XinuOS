/* 82545EMInit.c - _82545EMInit */

#include <xinu.h>

local 	status 	_82545EM_init_hw(struct ether *);
local 	void 	_82545EM_reset_hw(struct ether *);
local 	void 	_82545EM_configure_rx(struct ether *);
local 	void 	_82545EM_configure_tx(struct ether *);
status 	_82545EM_read_phy_reg(struct ether *, uint32, uint16 *);
status 	_82545EM_write_phy_reg(struct ether *, uint32, uint16);


/*------------------------------------------------------------------------
 * _82545EMInit - initialize Intel 82545EM Ethernet NIC
 *------------------------------------------------------------------------
 */
status 	_82545EMInit(
	struct 	ether *ethptr
	)
{
        uint32 iobase1;
        uint16 cmdreg;
        byte irq;
	/* Read PCI configuration information */
	/* Read I/O base address */
        pci_bios_read_config_dword(ethptr->pcidev, E1000_PCI_IOBASE, &iobase1);
      
        //kprintf("iobase %x \n\r",iobase1);
        ethptr->iobase = iobase1;
        ethptr->iobase &= ~1;
        pci_bios_read_config_dword(ethptr->pcidev, E1000_PCI_FLASHBASE,
                        (uint32 *)&ethptr->flashbase);

        /* Read memory base address */

        pci_bios_read_config_dword(ethptr->pcidev, E1000_PCI_MEMBASE,
                        (uint32 *)&ethptr->membase);

	/* Read interrupt line number */
        pci_bios_read_config_byte (ethptr->pcidev, E1000_PCI_IRQ,
                        (byte *)&irq);
        ethptr->dev->dvirq = irq; 
        //kprintf("flashbase %x , membase %x, irq %x\n\r",ethptr->flashbase, ethptr->membase, ethptr->dev->dvirq );
	/* Enable PCI bus master, I/O port access */
        pci_bios_read_config_word(ethptr->pcidev, E1000_PCI_COMMAND, 
                        &cmdreg);
        //kprintf("command register value %x \n\r",cmdreg);
        cmdreg |= E1000_PCI_CMD_MASK;
        pci_bios_write_config_word(ethptr->pcidev, E1000_PCI_COMMAND, 
                        cmdreg);

	/* Read the MAC address (from dhcp.c)*/
	/*uint32 i;
	uint32 rar_low = e1000_io_readl(ethptr->iobase, E1000_RAL(0));
	uint32 rar_high = e1000_io_readl(ethptr->iobase, E1000_RAH(0));

	for (i = 0; i < ETH_ADDR_LEN; i++) 
		ethptr->devAddress[i] = (byte)(rar_low >> (i*8));
	for (i = 0; i < ETH_ADDR_LEN; i++)
		ethptr->devAddress[i + 4] = (byte)(rar_high >> (i*8));

	kprintf("MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n",
			0xff&ethptr->devAddress[0],
			0xff&ethptr->devAddress[1],
			0xff&ethptr->devAddress[2],
			0xff&ethptr->devAddress[3],
			0xff&ethptr->devAddress[4],
			0xff&ethptr->devAddress[5]);*/
	/* Initialize structure pointers */

        ethptr->rxRingSize = E1000_RX_RING_SIZE;
        ethptr->txRingSize = E1000_TX_RING_SIZE;
        ethptr->isem = semcreate(0);
        ethptr->osem = semcreate(ethptr->txRingSize);

	/* Rings must be aligned on a 16-byte boundary */
	ethptr->rxRing = (void *)getmem((ethptr->rxRingSize + 1) * E1000_RDSIZE);
        ethptr->txRing = (void *)getmem((ethptr->txRingSize + 1) * E1000_TDSIZE);
        //kprintf("rxring"); 
        ethptr->rxRing = (void *)(((uint32)ethptr->rxRing + 0xf) & ~0xf);
        ethptr->txRing = (void *)(((uint32)ethptr->txRing + 0xf) & ~0xf);
	
	/* Buffers are highly recommended to be allocated on cache-line */
	/* 	size (64-byte for E8400) 				*/
	     
        ethptr->rxBufs = (void *)getmem((ethptr->rxRingSize + 1) * ETH_BUF_SIZE);
        ethptr->txBufs = (void *)getmem((ethptr->txRingSize + 1) * ETH_BUF_SIZE);
        ethptr->rxBufs = (void *)(((uint32)ethptr->rxBufs + 0x3f) & ~0x3f);
        ethptr->txBufs = (void *)(((uint32)ethptr->txBufs + 0x3f) & ~0x3f);


	/* Set buffer pointers and rings to zero */
	memset(ethptr->rxBufs, '\0', ethptr->rxRingSize * ETH_BUF_SIZE);
        memset(ethptr->txBufs, '\0', ethptr->txRingSize * ETH_BUF_SIZE);
        memset(ethptr->rxRing, '\0', E1000_RDSIZE * ethptr->rxRingSize);
        memset(ethptr->txRing, '\0', E1000_TDSIZE * ethptr->txRingSize);


	/* Insert the buffer into descriptor ring */
	struct e1000_rx_desc *rxringptr;
        uint32 rxbufptr;
        uint32 i;
        rxringptr = (struct e1000_rx_desc *)ethptr->rxRing;
        rxbufptr = (uint32)ethptr->rxBufs;
        for (i = 0; i < ethptr->rxRingSize; i++) {
                rxringptr->buffer_addr = (uint64)rxbufptr;
                rxringptr++;
                rxbufptr += ETH_BUF_SIZE;
        }

	struct e1000_tx_desc *txringptr;
        uint32 txbufptr;
        txringptr = (struct e1000_rx_desc *)ethptr->txRing;
        txbufptr = (uint32)ethptr->txBufs;
        for (i = 0; i < ethptr->txRingSize; i++) {
                txringptr->buffer_addr = (uint64)txbufptr;
                txringptr++;
                txbufptr += ETH_BUF_SIZE;
        }

	/* Reset packet buffer allocation to default */

        e1000_io_writel(ethptr->iobase, E1000_PBA, E1000_PBA_48K);

	/* Reset the NIC to bring it into a known state and initialize it */

	_82545EM_reset_hw(ethptr);

	/* Initialize the hardware */

	if (_82545EM_init_hw(ethptr) != OK)
		return SYSERR;

	/* Configure the NIC */

	e1000_io_writel(ethptr->iobase, E1000_AIT, 0);

	/* Configure the RX */

	_82545EM_configure_rx(ethptr);
	
	/* Configure the TX */

	_82545EM_configure_tx(ethptr);

	/* Register the interrupt and enable interrupt */
	
	set_evec(ethptr->dev->dvirq + IRQBASE, (uint32)e1000Dispatch);
	e1000IrqEnable(ethptr);

	return OK;
}

/*------------------------------------------------------------------------
 * _82545EM_reset_hw - Reset the hardware 
 *------------------------------------------------------------------------
 */
local void _82545EM_reset_hw(
	struct 	ether *ethptr
	)
{

	/* Masking off all interrupts */

        e1000_io_writel(ethptr->iobase, E1000_IMC, 0xffffffff);

	/* Disable the Transmit and Receive units. */
        e1000_io_writel(ethptr->iobase, E1000_RCTL, 0);
        e1000_io_writel(ethptr->iobase, E1000_TCTL, 0);


	/* Issuing a global reset by setting CTRL register with E1000_CTRL_RST*/
        uint32 control;
        control = e1000_io_readl(ethptr->iobase, E1000_CTRL); 
        control |= E1000_CTRL_RST;
        e1000_io_writel(ethptr->iobase, E1000_CTRL, control);
 
        e1000_io_writel(ethptr->iobase, E1000_CTRL, control);
	
    /* Delay slightly to let hardware process */
        MDELAY(20);

    /* Masking off all interrupts again*/
        e1000_io_writel(ethptr->iobase, E1000_IMC, 0xffffffff);

}

/*------------------------------------------------------------------------
 * _82545EM_init_hw - Initialize the hardware
 *------------------------------------------------------------------------
 */
local status _82545EM_init_hw(
	struct 	ether *ethptr
	)
{
        uint32 i;
        uint32 control;
        uint16 phydata;
        uint16 phyctrl;
        uint16 phystatus;
	/* Setup the receive address */
	/* Zero out the other receive addresses */
       for (i = 1; i < E1000_82545EM_RAR_ENTRIES; i++) {
                e1000_io_writel(ethptr->iobase, E1000_RAL(i), 0);
                e1000_io_flush(ethptr->iobase);
                e1000_io_writel(ethptr->iobase, E1000_RAH(i), 0);
                e1000_io_flush(ethptr->iobase);
        }


	/* Zero out the Multicast HASH table */

        for (i = 0; i < E1000_82545EM_MTA_ENTRIES; i++)
                e1000_io_writel(ethptr->iobase, E1000_MTA + (i << 2), 0);

	/* Configure copper link settings */
        //kprintf("Setting up the link...\n\r");
        control = e1000_io_readl(ethptr->iobase, E1000_CTRL);
        control |= E1000_CTRL_SLU;
        control &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);

	/* Commit the changes.*/
        e1000_io_writel(ethptr->iobase, E1000_CTRL, control);
        if (_82545EM_read_phy_reg(ethptr,M88E1000_PHY_SPEC_CTRL, &phydata) != OK)
                 return SYSERR;
        phydata |= M88E1000_PSCR_AUTO_X_MODE; 
        phydata &= ~M88E1000_PSCR_POLARITY_REVERSAL;
        phydata |= E1000_BM_PSCR_ENABLE_DOWNSHIFT;
 
       if (_82545EM_write_phy_reg(ethptr,M88E1000_PHY_SPEC_CTRL, phydata) != OK)
                 return SYSERR;

       if (_82545EM_read_phy_reg(ethptr,M88E1000_PHY_SPEC_CTRL, &phyctrl) != OK)
                 return SYSERR;               

       phyctrl |= E1000_MII_CR_RESET;

       if (_82545EM_write_phy_reg(ethptr, E1000_PHY_CONTROL , phyctrl) != OK)
                 return SYSERR;
    /* Do a slightly delay for the hardware to proceed the commit */
       DELAY(5);

	/* Setup autoneg and flow control advertisement and perform 	*/
	/* 	autonegotiation. 					*/
       uint16 phyautonegadv, phy1000tctrl;
       if (_82545EM_read_phy_reg(ethptr, E1000_PHY_AUTONEG_ADV, &phyautonegadv) != OK)
                 return SYSERR;

       if (_82545EM_read_phy_reg(ethptr, E1000_PHY_1000T_CTRL, &phy1000tctrl) != OK)
                 return SYSERR;

       phyautonegadv |= (E1000_NWAY_AR_100TX_FD_CAPS |
                         E1000_NWAY_AR_100TX_HD_CAPS |
                         E1000_NWAY_AR_10T_FD_CAPS   |
                         E1000_NWAY_AR_10T_HD_CAPS);
            
       phy1000tctrl &= ~E1000_CR_1000T_HD_CAPS;
       phy1000tctrl |= E1000_CR_1000T_FD_CAPS;

       phyautonegadv &= ~(E1000_NWAY_AR_ASM_DIR | E1000_NWAY_AR_PAUSE);

       if (_82545EM_write_phy_reg(ethptr, E1000_PHY_AUTONEG_ADV,phyautonegadv) != OK)
                return SYSERR;

       if (_82545EM_write_phy_reg(ethptr, E1000_PHY_1000T_CTRL, phy1000tctrl) != OK)
                return SYSERR;

       if (_82545EM_read_phy_reg(ethptr, E1000_PHY_CONTROL, &phyctrl) != OK)
                return SYSERR;

	/* Restart auto-negotiation. */
       phyctrl |= (E1000_MII_CR_AUTO_NEG_EN | E1000_MII_CR_RESTART_AUTO_NEG);

       if (_82545EM_write_phy_reg(ethptr, E1000_PHY_CONTROL, phyctrl) != OK)
                return SYSERR;

       //kprintf("before autoneg loop \n\r");
	/* Wait for auto-negotiation to complete 
       Implement a loop here to check the E1000_MII_SR_LINK_STATUS and E1000_MII_SR_AUTONEG_COMPLETE, break if they are both ture
       You should also delay for a while in each loop so it won't take too much CPU time */
       for (;;) {
                if (_82545EM_read_phy_reg(ethptr, E1000_PHY_STATUS, &phystatus) != OK)
                        DELAY(10);
                    
                if (_82545EM_read_phy_reg(ethptr, E1000_PHY_STATUS, &phystatus) != OK)
                        return SYSERR;
                
                if ( (phystatus & E1000_MII_SR_LINK_STATUS) && (phystatus &E1000_MII_SR_AUTONEG_COMPLETE) )
                        break;
                
                MDELAY(100);
       }
       //kprintf("after autoneg loop \n\r");

    /* Update device control according receive flow control and transmit flow control*/
        control = e1000_io_readl(ethptr->iobase, E1000_CTRL);
        control &= (~(E1000_CTRL_TFCE | E1000_CTRL_RFCE));
        e1000_io_writel(ethptr->iobase, E1000_CTRL, control);
	return OK;
}

/*------------------------------------------------------------------------
 * _82545EM_configure_rx - Configure Receive Unit after Reset
 *------------------------------------------------------------------------
 */
local void _82545EM_configure_rx(
	struct 	ether *ethptr
	)
{
        uint32 rctl;
        rctl = e1000_io_readl(ethptr->iobase, E1000_RCTL);
	/* Disable receiver while configuring. */
        rctl &= ~E1000_RCTL_EN;
        e1000_io_writel(ethptr->iobase, E1000_RCTL, rctl);
	/* Enable receiver, accept broadcast packets, no loopback, and 	*/
	/* 	free buffer threshold is set to 1/2 RDLEN. 		*/
        rctl |= E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_LBM_NO | E1000_RCTL_RDMTS_HALF;

	/* Do not store bad packets, do not pass MAC control frame, 	*/
	/* 	disable long packet receive and CRC strip 		*/
     
	rctl &= ~(E1000_RCTL_SBP | E1000_RCTL_LPE | E1000_RCTL_SECRC | E1000_RCTL_PMCF);
	
	/* Setup buffer sizes */
        rctl &= ~(E1000_RCTL_BSEX | E1000_RCTL_SZ_4096 | E1000_RCTL_FLXBUF_MASK);
        rctl |= E1000_RCTL_SZ_2048;

	/* Set the Receive Delay Timer Register, let driver be notified */
	/* 	immediately each time a new packet has been stored in 	*/
	/* 	memory 							*/

        e1000_io_writel(ethptr->iobase, E1000_RDTR, E1000_RDTR_DEFAULT);
        e1000_io_writel(ethptr->iobase, E1000_RADV, E1000_RADV_DEFAULT);

	/* Set up interrupt rate to be default. Notice that it is a the rate is not just E1000_ITR_DEFAULT which is the frequency, 
       it is 1000000000 / (E1000_ITR_DEFAULT * 256) */

        e1000_io_writel(ethptr->iobase, E1000_ITR, 1000000000 / (E1000_ITR_DEFAULT * 256));

	/* Setup the HW Rx Head and Tail Descriptor Pointers, the Base 	*/
	/* 	and Length of the Rx Descriptor Ring 			*/

        e1000_io_writel(ethptr->iobase, E1000_RDBAL(0), (uint32)ethptr->rxRing);
        e1000_io_writel(ethptr->iobase, E1000_RDBAH(0), 0);
        e1000_io_writel(ethptr->iobase, E1000_RDLEN(0), E1000_RDSIZE * ethptr->rxRingSize);
        e1000_io_writel(ethptr->iobase, E1000_RDH(0), 0);
        e1000_io_writel(ethptr->iobase, E1000_RDT(0), ethptr->rxRingSize - E1000_RING_BOUNDARY);

	/* Disable Receive Checksum Offload for IPv4, TCP and UDP. */
        uint32 rxcsum;
        rxcsum = e1000_io_readl(ethptr->iobase, E1000_RXCSUM);
        rxcsum &= ~E1000_RXCSUM_TUOFL;
        e1000_io_writel(ethptr->iobase, E1000_RXCSUM, rxcsum);

	/* Enable receiver. */
        e1000_io_writel(ethptr->iobase, E1000_RCTL, rctl);
}

/*------------------------------------------------------------------------
 * _82545EM_configure_tx - Configure Transmit Unit after Reset
 *------------------------------------------------------------------------
 */
local void _82545EM_configure_tx(
	struct 	ether *ethptr
	)
{

	/* Set the transmit descriptor write-back policy for both queues */
        uint32 txdctl;
        txdctl = e1000_io_readl(ethptr->iobase, E1000_TXDCTL(0));
        txdctl &= ~E1000_TXDCTL_WTHRESH;
        txdctl |= E1000_TXDCTL_GRAN;
        e1000_io_writel(ethptr->iobase, E1000_TXDCTL(0), txdctl);
        txdctl = e1000_io_readl(ethptr->iobase, E1000_TXDCTL(1));
        txdctl &= ~E1000_TXDCTL_WTHRESH;
        txdctl |= E1000_TXDCTL_GRAN;
        e1000_io_writel(ethptr->iobase, E1000_TXDCTL(1), txdctl);


	/* Program the Transmit Control Register */
	uint32 tctl = e1000_io_readl(ethptr->iobase, E1000_TCTL);
        tctl &= ~E1000_TCTL_CT;
        tctl |= E1000_TCTL_RTLC |
                E1000_TCTL_EN |
                E1000_TCTL_PSP |
                (E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT);
        tctl &= ~E1000_TCTL_COLD;
        tctl |= E1000_COLLISION_DISTANCE << E1000_COLD_SHIFT;



	/* Set the default values for the Tx Inter Packet Gap timer */
	
        uint32 tipg = E1000_TIPG_IPGT_COPPER_DEFAULT; 
        tipg |= E1000_TIPG_IPGR1_DEFAULT << E1000_TIPG_IPGR1_SHIFT;
        tipg |= E1000_TIPG_IPGR2_DEFAULT << E1000_TIPG_IPGR2_SHIFT;
        e1000_io_writel(ethptr->iobase, E1000_TIPG, tipg);

	/* Set the Tx Interrupt Delay register */
	
        e1000_io_writel(ethptr->iobase, E1000_TIDV, E1000_TIDV_DEFAULT);
        e1000_io_writel(ethptr->iobase, E1000_TADV, E1000_TADV_DEFAULT);

	/* Setup the HW Tx Head and Tail descriptor pointers */
	
        e1000_io_writel(ethptr->iobase, E1000_TDBAL(0), (uint32)ethptr->txRing);
        e1000_io_writel(ethptr->iobase, E1000_TDBAH(0), 0);
        e1000_io_writel(ethptr->iobase, E1000_TDLEN(0), E1000_TDSIZE * ethptr->txRingSize);
        e1000_io_writel(ethptr->iobase, E1000_TDH(0), 0);
        e1000_io_writel(ethptr->iobase, E1000_TDT(0), 0);


    /* Enable transmit but setting TCTL*/
        e1000_io_writel(ethptr->iobase, E1000_TCTL, tctl);
}

/*------------------------------------------------------------------------
 * _82545EM_read_phy_reg - Read MDI control register
 *------------------------------------------------------------------------
 */
status _82545EM_read_phy_reg(
	struct 	ether *ethptr,
	uint32 	offset,
	uint16 	*data
	)
{
	uint32 i, mdic = 0;

	if (offset > E1000_MAX_PHY_REG_ADDRESS) {
		return SYSERR;
	}

	mdic = ((offset << E1000_MDIC_REG_SHIFT) |
		(E1000_82545EM_MDIC_PHY_ADDR << E1000_MDIC_PHY_SHIFT) |
		(E1000_MDIC_OP_READ));

	e1000_io_writel(ethptr->iobase, E1000_MDIC, mdic);

	for (i = 0; i < (E1000_GEN_POLL_TIMEOUT * 3); i++) {
		DELAY(50);
		mdic = e1000_io_readl(ethptr->iobase, E1000_MDIC);
		if (mdic & E1000_MDIC_READY)
			break;
	}
	if (!(mdic & E1000_MDIC_READY)) {
		return SYSERR;
	}
	if (mdic & E1000_MDIC_ERROR) {
		return SYSERR;
	}
	*data = (uint16) mdic;

	return OK;
}

/*------------------------------------------------------------------------
 *  _82545EM_write_phy_reg - Write MDI control register
 *------------------------------------------------------------------------
 */
status _82545EM_write_phy_reg(
	struct 	ether *ethptr,
	uint32 	offset,
	uint16 	data
	)
{
	uint32 i, mdic = 0;

	if (offset > E1000_MAX_PHY_REG_ADDRESS) {
		return SYSERR;
	}

	mdic = ( ((uint32)data) |
		 (offset << E1000_MDIC_REG_SHIFT) |
		 (E1000_82545EM_MDIC_PHY_ADDR << E1000_MDIC_PHY_SHIFT) |
		 (E1000_MDIC_OP_WRITE) );

	e1000_io_writel(ethptr->iobase, E1000_MDIC, mdic);

	for (i = 0; i < (E1000_GEN_POLL_TIMEOUT * 3); i++) {
		DELAY(50);
		mdic = e1000_io_readl(ethptr->iobase, E1000_MDIC);
		if (mdic & E1000_MDIC_READY)
			break;
	}
	if (!(mdic & E1000_MDIC_READY)) {
		return SYSERR;
	}
	if (mdic & E1000_MDIC_ERROR) {
		return SYSERR;
	}

	return OK;
}
