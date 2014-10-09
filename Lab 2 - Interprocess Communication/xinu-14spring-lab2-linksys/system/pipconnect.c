/* pipconnect.c - pipconnect */

#include <xinu.h>

/*------------------------------------------------------------------------
 * pipconnect  --  Connect a pipe with a reader and writer process
 *------------------------------------------------------------------------
 */
syscall pipconnect(pipid32 pip, pid32 writer, pid32 reader)
{
	intmask mask;			/* saved interrupt mask		*/
        //kprintf("inside pipconnect \n\r");	
	mask = disable();
	if (isbadpip(pip) || isbadpid(writer) || isbadpid(reader) || (reader == writer) ) {
		restore(mask);
		return SYSERR;
	}
        if ((piptab[pip].pipstate == PIPE_CONNECTED) || (piptab[pip].pipstate == PIPE_FREE)) {
		restore(mask);
		return SYSERR;
	}

        piptab[pip].pipstate = PIPE_CONNECTED;
        piptab[pip].rdend = reader;
        piptab[pip].wrtend = writer;
        // create process table entry for pipe state
        //pipedetails(pip);
	restore(mask);
	return OK;
}
