/* lab1.c - pschprio, tschprio */
#include<xinu.h>
/* pschprio, tschprio - change the priority of PS and TS processes */
pri16 pschprio (pid32 pid,pri16 prio)
{
    struct procent *prptr;
    intmask mask;
    mask = disable();
    kprintf("Changing PSpriority of PS process %d to %d \r\n",pid,prio);
    prptr = &proctab[pid];
    prptr->prPsPrio = prio
    kprintf("Maxkey - prio = %d \r\n",(MAXKEY - prio)); 
    chprio (pid, (MAXKEY - prio));
    restore(mask);
    return prio;
}
pri16 tschprio (pid32 pid,pri16 prio)
{
    intmask mask;
    mask = disable();
    kprintf("Changing priority of TS process %d to %d \r\n",pid,prio);
    chprio (pid, prio);
    restore(mask);
    return prio;
} 
