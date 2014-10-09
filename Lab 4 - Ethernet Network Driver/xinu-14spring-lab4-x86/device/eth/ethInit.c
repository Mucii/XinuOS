/* ethInit.c - ethInit */

#include <xinu.h>

struct	ether	ethertab[Neth];		/* Ethernet control blocks 	*/

/*------------------------------------------------------------------------
 * ethInit - Initialize Ethernet device structures
 *------------------------------------------------------------------------
 */
devcall	ethInit (
	  struct dentry *devptr
	)
{
	struct	ether 	*ethptr;
	int32	dinfo;			/* device information*/
        uint32 value, value1,value2, confw;
        int busNum, deviceNum, functionNum;

	ethptr = &ethertab[devptr->dvminor];
	memset(ethptr, '\0', sizeof(struct ether));
        ethptr->dev = devptr;
        ethptr->csr = devptr->dvcsr;
        ethptr->state = ETH_STATE_DOWN;
        //ethptr->mtu = ETH_MTU;
        ethptr->addrLen = ETH_ADDR_LEN;
        ethptr->rxHead = 0;
        ethptr->rxTail = 0;
        ethptr->txHead = 0;
        ethptr->txTail = 0;
        //kprintf("pcidev %x config word %d, value %x \n\r",ethptr->pcidev,confw,value);
    /*If find the device on PCI bus, intialize the devtab of ETHER0 with the funtion pointers of e1000
     If not found, return SYSERR*/
        if( (dinfo = find_pci_device(0x100F, 0x8086, 0)) == SYSERR) {
               kprintf("No valid device found in the PCI table \n\r");
               return SYSERR;
        }
        //kprintf("Found device info %d from PCI\n\r",dinfo);
        ethptr->pcidev = dinfo;	
        ethptr->type = NIC_TYPE_82545EM; 
        _82545EMInit(ethptr);
	/* Set the state of ethptr to be UP */
        ethptr->state = ETH_STATE_UP;
	return OK;
}
