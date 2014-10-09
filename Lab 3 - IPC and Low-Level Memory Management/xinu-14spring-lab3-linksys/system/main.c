/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
/************************************************************************/
/* */
/*  main - main program for testing Xinu */
/* */
/************************************************************************/
/*void sender(pid32);
void receiver(void);
//int myrecvhandler(void);

void sender(pid32 receiver) {
uint32 i;
for(i=0+(currpid*20); i<(15 + (currpid*20)); i++) {
if( sendb(receiver, i) == SYSERR ) {
kprintf("Fail to send msg %d!\r\n", i);
} else {
kprintf("Send msg %d to receiver from %d!\r\n", i,currpid);
}
}
return;
}

void receiver() {
int i;
uint32 msg;
for(i=0; i<30; i++) {
msg = receiveb();
if( msg == SYSERR ) {
kprintf("Fail to recieve msg!\r\n");
} else {
kprintf("Recieve msg %d from sender!\r\n", msg);
}
sleep(1);
}
return;
}*/
 
/*int main(int argc, char **argv) {
pid32 spid, rpid;
rpid = create(receiver, 2014, 20, "receiver", NULL);
spid = create(sender, 2048, 20, "sender", 1, rpid);

resume(spid);
resume(rpid);
while(1) {
sleep(100);
}
return OK;
}*/

/*uint32 recvbuf;
int myrecvhandler(void) {
kprintf("\nhello!! msg received = %d\n", recvbuf);
return(OK);
}
 
void sender(pid32 receiver) {
sleep(3);
if( send(receiver, 20) == SYSERR ) {
kprintf("Fail to send msg 20!\r\n");
} else {
kprintf("Send msg 20 to receiver!\r\n");
}
return;
}
void receiver() {
if (registerrecv(&recvbuf, &myrecvhandler) != OK) {
kprintf("recv handler registration failed\n");
return;
}
while(1) {
sleep(1);
}
return;
}*/
 
/*int main(int argc, char **argv) {
pid32 spid, rpid;
rpid = create(receiver, 2014, 20, "receiver", NULL);
spid = create(sender, 2048, 20, "sender", 1, rpid);
resume(rpid);
resume(spid);

while(1) {
sleep(100);
}
return OK;
}*/
char *x1;
void prmemlist(pid32 pid)
{
	struct procent *prptr = &proctab[pid];
	struct memblkgb *curr = NULL;
	kprintf("<><><>\n\r");
	if(prptr->prstate != PR_FREE)
	{
		kprintf("head %x\n\r",(prptr->prmemlistgb));
		curr = prptr->prmemlistgb;
		while(curr != NULL)
		{
			kprintf("%x with length %d & nxt %x\n\r", curr, curr->mlength,curr->mnext);
	                curr = curr->mnext;
		}
	}
	kprintf("<><><>\n\r");
}

int memallocgb(void)
{
	int i;
	int *x;
        kprintf("[function] memlist.length = %d, alloted : %d\n\r",memlist.mlength,usedspace);	
	for(i=0;i<4;i++)
	{
		x =(int*) getmemgb(50*sizeof(int));
		//yield();

	}
         prmemlist(currpid);
        /* if(freememgb((char*)x1,100)== SYSERR)
	{
		kprintf("ERROR 1\n\r");
	}
	 freememgb((char *)x,200);
         prmemlist(currpid);*/
	return 0;
}

int main(int argc, char **argv)
{
        //char *x2, *x3;
	/*for(i =0;i<30;i++)
		x = (int*)getmemgb(sizeof(int));*/
	//kprintf("memlist.length = %d, alloted : %d\n\r",memlist.mlength,usedspace);	
        if((x1 = getmemgb(100)) == (char *)SYSERR)
	{
		kprintf("ERROR @@\n\r");
	}
        /*if((x2 = getmemgb(150)) == (char *)SYSERR)
	{
		kprintf("ERROR @@\n\r");
	}
        if((x3 = getmemgb(250)) == (char *)SYSERR)
	{
		kprintf("ERROR @@\n\r");
	}
	kprintf("memlist.length = %d, alloted : %d\n\r",memlist.mlength,usedspace);	
	kprintf("prmemlisthead->length %d\n\r",proctab[currpid].prmemlistgb->mlength);
        prmemlist(currpid);*/
        /*if(freememgb((char*)x2,150)== SYSERR)
	{
		kprintf("ERROR 2\n\r");
	}
	if(freememgb((char*)x1,100)== SYSERR)
	{
		kprintf("ERROR 1\n\r");
	}
        kprintf("memlist.length = %d, alloted : %d\n\r",memlist.mlength,usedspace);	
        if((x4 = getmemgb(300)) == (char *)SYSERR)
	{
		kprintf("ERROR @@\n\r");
	}
        prmemlist(currpid);
	if(freememgb((char*)x3,250)== SYSERR)
	{
		kprintf("ERROR 3\n\r");
	}*/
        kprintf("memlist.length = %d, alloted : %d\n\r",memlist.mlength,usedspace);	
        resume(create(memallocgb,1024,20,"memalloc1",0));
        resume(create(memallocgb,1024,20,"memalloc2",0));
        //kprintf("memlist.length = %d\n\r",memlist.mlength);
	//kprintf("allocated = %d\n\r",usedspace);
        /*kprintf("memlist.length = %d, alloted : %d\n\r",memlist.mlength,usedspace);
        pid32 spid, rpid, spid2;
        rpid = create(receiver, 2014, 20, "receiver", NULL);
        spid2 = create(sender, 2014, 20, "sender1", 1, rpid);
        spid = create(sender, 2048, 20, "sender2", 1, rpid);
        resume(rpid);
        resume(spid2);
        resume(spid);

while(1) {
sleep(100);
}*/
//sleepms(100);
return OK;
}
