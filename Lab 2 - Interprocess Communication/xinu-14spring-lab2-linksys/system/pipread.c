/* pipread.c - pipread */

#include <xinu.h>
#include <stdio.h>
/*------------------------------------------------------------------------
 * pipread  --  Read len characters from the pipe into an array
 *------------------------------------------------------------------------
 */
syscall pipread(pipid32 pip, char *buf, uint32 len)
{
	intmask mask;			/* saved interrupt mask		*/
	//kprintf("inside pipread!!\n\r"); 
	mask = disable();
        uint32 numBytes=0;
	uint32 i;
        uint32 j=0;
        uint32 rdflag=0;
        int top;
        if (isbadpip(pip) || (buf == NULL) || (len < 0) ) {
                kprintf("error checking failed , isbadpip : %d, len : %d,buf string-- %s\r\n", isbadpip(pip),len,buf);
		restore(mask);
		return SYSERR;
	}
        if ((piptab[pip].rdend) != getpid()) {
 		kprintf("rdend : %d, getpid : %d \n\r",piptab[pip].rdend, getpid());
                restore(mask);
		return SYSERR;
	}
        //pipedetails(pip);
        //Check to see if the pipe was full and writer is in blocking state
        //kprintf("[rd]numBytes : %d, len: %d \n\r",numBytes,len);
        top = piptab[pip].nextpos;
        if(piptab[pip].pipstate == PIPE_CONNECTED) {
        if(top == 0) 
        {
              wait(piptab[pip].supply); 
        }
        if((top == PIPE_SIZE) && (nonempty(semtab[piptab[pip].demand].squeue)))
	{
	      //kprintf("reader is waiting until writer writes into the buffer\n\r");
              rdflag = 1;
	}        
        if(len <= top) {
              for(i=top-1,j=0; j<len;i--,j++) {
                   buf[j] = piptab[pip].buffer[i];
                   piptab[pip].nextpos--;
                   ++numBytes;
                   //kprintf("[rd] buf[i] :%c ,pipebuffer :%c , numBytes: %d nextpos: %d \t",buf[j],piptab[pip].buffer[i],numBytes, piptab[pip].nextpos);
              }
        //kprintf(" READ %d chards, buf ;%s \n\r",numBytes,buf); 
        }
        else {   
                   for(i=top-1,j=0; j<top;i--,j++) {
                         buf[j] = piptab[pip].buffer[i];
                         piptab[pip].nextpos--;
                         ++numBytes;
                   }
                   //kprintf("reader num read :%d\n\r",numBytes);
        }
        //pipedetails(pip);
        if(rdflag == 1)
        {
	          signal(piptab[pip].demand);
        }       
        restore(mask);
        return numBytes;
        }
        
        if(piptab[pip].pipstate == PIPE_DISCONNECTED) {
                if (top == 0) {
		       piptab[pip].pipstate = PIPE_USED;
                       piptab[pip].rdend = -1;
                       restore(mask);
		       return SYSERR;
		}
	        //return syserr if there are no characters to read from a pipe.
		if(len <= top) {
                   for(i=top-1,j=0; j<len;i--,j++) {
                        buf[j] = piptab[pip].buffer[i];
                        piptab[pip].nextpos--;
                        ++numBytes;
                   }
                }
                else {   
                   for(i=top-1,j=0; j<top;i--,j++) {
                         buf[j] = piptab[pip].buffer[i];
                         piptab[pip].nextpos--;
                         ++numBytes;
                         piptab[pip].pipstate = PIPE_USED;
                         piptab[pip].rdend = -1;
                         piptab[pip].nextpos = 0;
                   }
                }
                restore(mask);
                return numBytes;
        }

        else {
                restore(mask);
	        return SYSERR;
        }
}
                
