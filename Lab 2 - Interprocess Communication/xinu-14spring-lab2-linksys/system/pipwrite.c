/* pipwrite.c - pipwrite */

#include <xinu.h>

/*------------------------------------------------------------------------
 * pipwrite  --  Write len characters from an array into the pipe
 *------------------------------------------------------------------------
 */
syscall pipwrite(pipid32 pip, char *buf, uint32 len)
{
	intmask mask;			/* saved interrupt mask		*/
	//kprintf("inside pipwrite \n\r");
	mask = disable();
        uint32 numBytes=0;
	int32 i,j;
        uint32 wrflag=0;
        if (isbadpip(pip) || (buf == NULL) || (len < 0) ) {
		restore(mask);
		return SYSERR;
	}
        if ((piptab[pip].wrtend) != getpid()) {
		restore(mask);
		return SYSERR;
	}
        /*if((piptab[pip].nextpos == 0) && (nonempty(semtab[piptab[pip].supply].squeue)))
	{
		wrflag = 1;
	}*/
        //kprintf("Before writer while numBytes\n\r",numBytes);
        //pipedetails(pip);
        if (piptab[pip].pipstate == PIPE_CONNECTED) {
        while(numBytes < len){
                if((piptab[pip].nextpos == 0) && (nonempty(semtab[piptab[pip].supply].squeue)))
                {
                     //kprintf("reader is waiting until writer writes into the buffer\n\r");
                     wrflag = 1;
                }
                int remSpace = PIPE_SIZE - piptab[pip].nextpos;
                int toWrite = len - numBytes;
                int top = piptab[pip].nextpos;
                //kprintf("remSpace :%d, top(nxtpos) :%d ,toWrite :%d\n\r",remSpace,top,toWrite);
                if(toWrite > remSpace) {
                     for(i=top-1;i>=0;i--) {
                         piptab[pip].buffer[i + remSpace] = piptab[pip].buffer[i];
                     }
                     piptab[pip].nextpos = piptab[pip].nextpos + remSpace;
                     for(i=remSpace-1,j=numBytes;i>=0;i--,j++) {
                         (piptab[pip]).buffer[i] = buf[j];
                         numBytes++;
                     }
                //pipedetails(pip);
 	        if(wrflag == 1)
	        {
		     signal(piptab[pip].supply); 
                     wrflag = 0;
	        }
                //pipread could read some data if it gets scheduled after signal
                if(piptab[pip].nextpos == PIPE_SIZE) {
                     wait(piptab[pip].demand); 
                }
                else 
                {    
                  remSpace = PIPE_SIZE - piptab[pip].nextpos; 
                  if((piptab[pip].nextpos == 0) && (nonempty(semtab[piptab[pip].supply].squeue)))
	          {
	             //kprintf("writer is ..waiting until reader reads from the buffer\n\r");
          	     wrflag = 1;
	          }
                }
                //kprintf("back to pipwrite::numBytes : %d\n\r",numBytes);
                }    
                if(toWrite <= remSpace) {
                     //kprintf("toWrite (%d) < remSpace (%d)\n\r",toWrite,remSpace);
                     for(i=piptab[pip].nextpos-1;i>=0;i--) {
                         piptab[pip].buffer[i + toWrite] = piptab[pip].buffer[i];
                         //kprintf("[wr] buf[%d] :%c ,pipebuffer :%c  \t",i+toWrite,buf[i],piptab[pip].buffer[i]);
                     }
                     piptab[pip].nextpos = piptab[pip].nextpos + toWrite;
                     for(i=toWrite-1,j=numBytes;i>=0;i--,j++) {
                         piptab[pip].buffer[i] = buf[j];
                         //kprintf("[wr] buf[%d] :%c ,pipebuffer :%c , numBytes: %d \t",i,buf[i],piptab[pip].buffer[i],numBytes);
                         numBytes++;
                     }
                //pipedetails(pip); 
                }
                
        }
 	if(wrflag == 1)
	{
                //kprintf("[outside while] reader is waiting until writer writes into the buffer\n\r");
		signal(piptab[pip].supply);
	}
	restore(mask);
	return numBytes;
        }
        if (piptab[pip].pipstate == PIPE_DISCONNECTED) {
                piptab[pip].pipstate = PIPE_USED;
                piptab[pip].wrtend = -1;
                piptab[pip].nextpos = 0;
                restore(mask);
	        return SYSERR;
        }
        else {
                restore(mask);
	        return SYSERR;
        }
}
