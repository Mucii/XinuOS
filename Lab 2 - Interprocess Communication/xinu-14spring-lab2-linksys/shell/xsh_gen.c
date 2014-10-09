/* xsh_gen.c - xsh_gen,writepip */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wordlist.h>
#include "shprototypes.h"
/*------------------------------------------------------------------------
 * xsh_gen - generate words from wordlist.h and write it into a pipe
 *------------------------------------------------------------------------
 */
extern void searchpipe(pipid32);
//extern char *words[];
void writepip(pipid32);
sid32 printsem;

shellcmd xsh_gen(int nargs, char *args[])
{
	pipid32 pip;
        pid32 rdpid,wrtpid;
        /* For argument '--help', emit help about the 'gen' command	*/
	if (nargs == 2 && (strncmp(args[1], "--help", 7) == 0)) {
		printf("Use: %s\n\n", args[0]);
		printf("Description:\n");
		printf("\tGenerates a word from wordlist and prints the count of words starting with vowel\n");
		printf("Options:\n");
		printf("\t | search\t it will print the number of words generated in 5 sec and count the number of words starting with vowels read from the pipe\n");
		return 0;
	}

	if (nargs < 3) {
		fprintf(stderr, "%s: too few arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}
        else {
                if ((nargs == 3) && (strncmp(args[1], "|", 1) == 0) && (strncmp(args[2], "search", 6) == 0)) {
                     //kprintf("correct usage \n\r");
                     if((pip = pipcreate()) == SYSERR) {
                         kprintf("pipe creation unsuccessful \n\r");
                         return 0;
                     }
                     printsem = semcreate(1);
                     rdpid = create(searchpipe, SHELL_CMDSTK, SHELL_CMDPRIO,"search", 2, pip, &printsem);
                     wrtpid = getpid();
                     if(pipconnect(pip,wrtpid,rdpid) == SYSERR) { 
                         kprintf("pip connection failed \n\r");
                         return 0;
                     }
                     else {
                         //kprintf("before resuming reader and writing\n\r");
                         resume(rdpid);
                         writepip(pip);
                         //resume(rdpid);
                         kprintf("after writing\n\r");
                     }
                }
        }
	return 0;
}
void writepip(pipid32 pip) {
        int index = 0;
        int wdCount = 0, numBytes;
        //char genWord[5];
        char *genWord; 
        int wrttime = 0,t=0;
        while(1) {
            index = rand() % 2048;
            /*if(index == 2048) {
                index = 0;
            }
            if(index < 2048) {
                genWord = (char*)(words[index]);
            }*/
            genWord = (char*)(words[index]);
            if((numBytes = pipwrite(pip,(char*)words[index]+0 ,5)) == SYSERR) {
                kprintf("pipewrite failed \n\r");
                return;
                } 
            wdCount = wdCount + 1;
            //index = index + 1;
            //kprintf("clktime : %d, wdcount : %d ,numBytes : %d\n\r",clktime, wdCount,numBytes);
            if (((((t=clktime)) % 5) == 0) && (clktime != wrttime)) {
                wait(printsem);
                kprintf("Number of words generated : %d \n\r",wdCount);
                signal(printsem); 
                wdCount = 0;
                wrttime = t;          
            }
            //kprintf("generated word: %c%c%c%c%c ,index :%d\n\r",genWord[0],genWord[1],genWord[2],genWord[3],genWord[4],index);
        }                
}
