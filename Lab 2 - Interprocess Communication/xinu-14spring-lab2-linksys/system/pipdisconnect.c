/* pipdisconnect.c - pipdisconnect */

#include <xinu.h>

/*------------------------------------------------------------------------i
 * pipdisconnect  --  Disconnect a pipe with a reader and writer process
 *------------------------------------------------------------------------
 */
syscall pipdisconnect(pipid32 pip)
{
	intmask mask;			/* saved interrupt mask		*/
	mask = disable();
	if (isbadpip(pip)) {
		restore(mask);
		return SYSERR;
	}
        if ((piptab[pip].pipstate == PIPE_FREE) || (piptab[pip].pipstate == PIPE_DISCONNECTED)) {
		restore(mask);
		return SYSERR;
	}
        if ((getpid() != piptab[pip].rdend) || (getpid() != piptab[pip].wrtend)) {
		restore(mask);
		return SYSERR;
	}
        if (getpid() == piptab[pip].rdend) {
                piptab[pip].rdend = -1;
		piptab[pip].pipstate = PIPE_DISCONNECTED;
                if((piptab[pip].nextpos == PIPE_SIZE) && (nonempty(semtab[piptab[pip].demand].squeue)))
		{
		    signal(piptab[pip].demand);
		}
                semreset(piptab[pip].supply, 0);
                semreset(piptab[pip].demand, 0);
                restore(mask);
	        return OK;
        }
        if (getpid() == piptab[pip].wrtend) {
                piptab[pip].wrtend = -1;
                piptab[pip].pipstate = PIPE_DISCONNECTED;
                if((piptab[pip].nextpos == 0) && (nonempty(semtab[piptab[pip].supply].squeue)))
		{
		    signal(piptab[pip].supply);
		}
                semreset(piptab[pip].supply, 0);
                semreset(piptab[pip].demand, 0);
                restore(mask);
	        return OK;

        }
        piptab[pip].pipstate = PIPE_USED;
        piptab[pip].rdend = -1;
        piptab[pip].wrtend = -1;
        semreset(piptab[pip].supply, 0);
        semreset(piptab[pip].demand, 0);
	restore(mask);
	return OK;
}
