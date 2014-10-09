Lab 4 - A device Driver for an Intel PRO/100 Ethernet Adapter

I have implemented the following functions in the device driver. 

devcall	ethInit (struct dentry *devptr)

1. Initailize the ether tab.
2. Check if the PCI device can be found using (0x100F,0x8086) as device and vendor id. If not, return SYSERR. 
3. If the device is found on the PCI bus, the devtab of ETHER0 is initialized with the funtion pointers of e1000 
4. Start up the ethernet device.

e1000_io_writel and e1000_io_readl  are implemented in e1000.h using inX and outX

_82545EMInit(struct ether *ethptr) is called from ethInit. 

It reads the IO base address, flash base address and the
PCI line number from the PCI table using the device info. The ring structure pointers are then initialized to 0. 
ethptr->osem is initailized with ethptr->txRingSize. The transmit and receive rings are aligned on a 16-byte boundary and the
buffers are allocated on the 64-bit cache-line. THe newly allocated transmit and receive
buffers are inserted into the e1000_tx_desc and e1000_rx_desc ring respectively.
The packet buffer is set to a value of E1000_PBA_48K. 

_82545EM_reset_hw(struct ether *ethptr)
Write 0xffffffff to E1000_IMC to mask off all interrupts, write 0 to E1000_RCTL and E1000_TCTL registers
to disable transmit and receive units. E1000_CTRL register is set with a E1000_CTRL_RST to issue a global reset. 

_82545EM_init_hw(struct	ether *ethptr)

E1000_RAL(i), E1000_RAH(i), Multicast HASH table (i=1 to E1000_82545EM_RAR_ENTRIES) are zeroed out. 
Read E1000_CTRL register and set it with the appropriate value to set the link up, disable force speed and duplex. 
The physical register M88E1000_PHY_SPEC_CTRL is read and is modified to enable the autox mode, disable polarity reversal.
Wait for auto-negotiation to complete. E1000_PHY_AUTONEG_ADV is written a value so as to Setup autoneg, flow control advertisement 
and perform autonegotiation. A for loop is implemented which waits and checks the E1000_MII_SR_LINK_STATUS 
and E1000_MII_SR_AUTONEG_COMPLETE, break if they are both true. 


local void _82545EM_configure_rx(struct ether *ethptr):

1. Disable receiver (~E1000_RCTL_EN) while configuring.
2. Enable receiver, accept broadcast packets, no loopback, and free buffer threshold is set to 1/2 RDLEN. The E1000_RCTL register value is read and modified 
to not store bad packets, do not pass MAC control frame, disable long packet receive and CRC strip. The receiver buffer size is set to 2048. 
3. Setup the hardware Rx Head and Tail Descriptor Pointers. E1000_RDBAL(0) points to the ethptr->rxRing. (only 32 bits need to be used here). 
E1000_RDLEN(0) is set to ring size - E1000_RDSIZE * ethptr->rxRingSize. 
4. Disable Receive Checksum Offload for IPv4, TCP and UDP.
5. Set up the RCTL register and enable receiver.
Write the modified control register value to E1000_RCTL. 

devcall	e1000Read(struct dentry	*devptr ,void	*buf, uint32 len):
1. Wait for a packet to arrive by waiting for isem. 
2. Find out where to pick up the packet. (using the head of the receive ring buffer, copy the received buffer to system memory)
3. Verify the package and pick up the packet.
4. Clear up the descriptor and the buffer. 
5. Add newly reclaimed descriptor to the ring. (advancing E1000_RDT(0))
6. Advance the head pointing to the next ring descriptor which will be ready to be picked up. (advance ethptr->rxHead)

local void _82545EM_configure_tx(struct ether *ethptr):
1. Set the transmit descriptor write-back policy. I have set it for E1000_TXDCTL(0) and E1000_TXDCTL(1) queues. The writeback threshold is reset 
with E1000_TXDCTL_GRAN  granularity. 
2. Program the Transmit Control Register. It is set to re-transmit on late, pad short packets and appropriate collision parameters are set. 
3. Set the default values for the Tx Inter Packet Gap timer. 
4. Set the Tx Interrupt Delay register with required bit shifts.
5. Setup the HW Tx Head and Tail descriptor pointers. E1000_TDBAL(0) points to the ethptr->txRing. (only 32 bits need to be used here). 
E1000_TDLEN(0) is set to ring size - E1000_TDSIZE * ethptr->txRingSize. 
6. Set the Transmit Control Register.
Write the modified control register value to E1000_TCTL. 

devcall	e1000Write(struct dentry	*devptr ,void	*buf, uint32 len):
1. Verify Ethernet interface is up and arguments are valid. (The ethernet interface should be up and the length of the buffer should be greater than ETH_MAX_PKT_LEN
and less than ETH_HDR_LEN.
2. Check the length, If padding of short packet is enabled, the value in TX descriptor length feild should be not less than 17 bytes
3. Wait for a free ring slot. (wait for ethptr->osem)
4. Find the tail of the ring to insert packet and get the transmit descriptor pointer. 
5. Copy packet to transmit ring buffer. 
6. Insert transmitting command and length. 
7. Add descriptor by advancing the tail pointer. (advancing E1000_TDT(0))
8. Advance the ring tail pointing to the next available ring descriptor. (advancing ethptr->txTail)

