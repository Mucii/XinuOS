/* pipe.h - isbadpip */

#ifndef	NPIPE
#define	NPIPE		10	/* number of pipes, if not defined	*/
#endif

#ifndef	PIPE_SIZE
#define	PIPE_SIZE	256	/* pipe size, if not defined	*/
#endif

/* Pipe state definitions */

#define	PIPE_FREE	0		/* pipe table entry is available*/
#define	PIPE_USED	1		/* pipe table entry is in use	*/

#define	PIPE_CONNECTED	2		/* pipe is connected    	*/
#define	PIPE_DISCONNECTED	3		/* pipe is disconnected 	*/

/* Pipe table entry */
struct	pipentry	{
	uint32 pipstate;	
	pid32 owner;
	pid32 rdend;
	pid32 wrtend;
	sid32 supply;
	sid32 demand;
	int32 nextpos;
	char buffer[PIPE_SIZE];
};

extern void pipedetails(pipid32 pip);
extern	struct	pipentry piptab[];
#define	isbadpip(pip)	((int32)(pip) < 0 || (pip) >= NPIPE)
