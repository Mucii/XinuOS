/* xsh_search.c - xsh_search,countvowels */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
//#include <wordlist.h>
#include <stdlib.h>
/*------------------------------------------------------------------------
 * xsh_search - 
 *------------------------------------------------------------------------
 */
int searchpipe(pipid32,int *);
int vowels[5] = {0,0,0,0,0};
shellcmd xsh_search(int nargs, char *args[])
{
//kprintf("inside xsh_search \n\r");
if(nargs > 0) {
	fprintf(stderr,"Search is not a valid shell command. It has to be used with gen | search.\n\r ");
	return 0;
	}
return 0;
}       

int searchpipe(pipid32 pip, int *printsem)
{
    char mybuf[5];
    //int i;
    //int length = 0;
    int mylen;
    int rdtime = 0;
    while(1) 
    {  
       //kprintf("inside searchpipe pipid : %d \n\r",pip);
       //i++;     

       /*while (length < 5) {
            mylen = pipread(pip, &mybuf[length], 600-length);
            if (SYSERR == mylen) {
                 kprintf("[rd]: Sucks!\r\n");
                 return SYSERR;
            }
            length += mylen;
      }*/
      if ((mylen = pipread(pip,mybuf,5)) == SYSERR) {
           //kprintf("Reading failed \n\r");
           return SYSERR;      
      }     
           //kprintf("[reader]: string-- %s ,mybuf: %c%c%c, length : %d\r\n", mybuf,mybuf[0],mybuf[2],mybuf[4], mylen);
           //countvowels(mybuf,vowels);
           if((mybuf[0] == 'a') || (mybuf[0] == 'A')) vowels[0] += 1;
           if((mybuf[0] == 'e') || (mybuf[0] == 'E')) vowels[1] += 1;
           if((mybuf[0] == 'i') || (mybuf[0] == 'I')) vowels[2] += 1;
           if((mybuf[0] == 'o') || (mybuf[0] == 'O')) vowels[3] += 1;
           if((mybuf[0] == 'u') || (mybuf[0] == 'U')) vowels[4] += 1;
           if (((clktime%10) == 0) && (clktime != rdtime)) {
                wait(*printsem);
                kprintf("Number of words for vowel a : %d, e : %d , i : %d, o : %d ,u : %d \n\r,",vowels[0]/2,vowels[1]/2,vowels[2]/2,vowels[3]/2,vowels[4]/2);
                signal(*printsem);
                vowels[0] = 0;
                vowels[1] = 0;
                vowels[2] = 0;
                vowels[3] = 0;
                vowels[4] = 0;
                rdtime = clktime;  
           }
     }
     return OK;
}     
