/* lab1.h */

/* variables for group scheduling */

#define INITPSPRIO 10    /* initial PS group priority */
#define INITTSPRIO 10    /* initial TS group priority */
#define isbadgroup(x)   (((int)(x) != 1)&&((int)(x) != 2)) 
extern pri16 psGpPrio;   /* PS Group Priority      */
extern pri16 tsGpPrio;   /* TS Group Priority      */
extern pri16 initPsPrio; /* PS Group Initial Priority */
extern pri16 initTsPrio; /* TS Group Initial Priority */
