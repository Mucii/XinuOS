                   Lab 2 - Interprocess Communication using BOunded-Buffers in XInu

I have created the following files for lab 2
The reading and writing mechanism uses 2 semaphores - supply and demand. If the pipe is empty, we will wait on the demand semaphore. If the pipe is full, 
will wait on the supply semaphore. The pipe buffer is a FIFO char array (Writing happens at the front of the array whereas reading is from the rear end). 
It uses a variable nextpos to keep track of the top of the array. Nextpos is set to 0 to reset a pipe buffer or when the pipe buffer is created or deleted.  
1) pipcreate.c
   Iterate through NPIPE number of pipes and return the pip id of the 1st pipe in PIPE_FREE state. Demand and supply semaphore are created using semcreate with
   values of 0.
2) pipdelete.c
-  check if the pip id is valid and if the owner calls this method or not.
3) pipread.c
   If the pipe was full before we entered this syscall with the writer process waiting in the semaphore queue, we will signal demand once the reading 
   operation is over. 
   The pipread returns the number of bytes written if the pipe is connected and the
   calling process is the reader of the pipe and the pipe is connected. If the pipe is empty, wait for the supply (i.e till the writer writes into the pipe).
   If the number of bytes to be read is more than pipe buffer, the reader will empty the pipe and return the number of bytes read. 
   If the writer end is disconnected, return SYSERR if the pipe is empty. If the pipe contains some data, read it and return the number of bytes read
   If the number of bytes to be read exceeds the buffer, return the number of buffer contents and clean up the pipe. 
   Reading is done from the top of the array. 
   
4) pipwrite.c
   If the pipe was empty before we entered this syscall with the reader process waiting in the semaphore queue, we will signal supply once the reading 
   operation is over.
   The writer waits till the contents of the buffer are written to the pipe. If the data to be written is more than the available space in the buffer, fill 
   the pipe and wait till the reader reads something from the pipe.  It will return the number of bytes written to the pipe, if the pipe is connected and the 
   current process is the writer process. The buffer is shifted to the right by the length of the content (SIZE - nextpos if length to be written exceeds the 
   pipe) and the data is written from the beginning of the array.  
   If the reader end is disconnected, clean up the pipe and return SYSERR. 
   
5) pipconnect.c
   The pipe should be in PIPE_USED state for it be to connected. The reader and writer process should be valid and should not be the same. The pip id should be
   valid as well. If the pipe is in FREE or CONNECTED state, it returns SYSERR.
   
6) pipdisconnect.c
   It returns SYSERR if 
     1) pipe state is PIPE_FREE or PIPE_DISCONNECTED
	 2) invalid pip id
	 3) if the current process is not reader or writer process
	 If the reader calls disconnect, reset the reader end of the pipe, put pipe to PIPE_DISCONNECTED state and if the writer is waiting for demand, signal demand.
	 If the writer calls disconnect, reset the writer end of the pipe, put pipe to PIPE_DISCONNECTED state and if the reader is waiting for supply, signal supply.

7) pipe.h - pipe has 4 states 1) PIPE_FREE 2) PIPE_USED 3) PIPE_CONNECTED 4) PIPE_DISCONNECTED.

A pipe maintains a variable for owner, reader, writer end, char array of size 256 and the next availabe free position in the array (nextpos) and two semaphore
demand and supply.


   Modified kill syscall.
   Before killing a process, iterate through the npipe
   1) If a pipe owner is killed, call pipdelete
  2) If pipe rader/writer is killed, call pipdisconnect.


The following files were created for the gen | search shell command implementation

 gen and search shell commands were added in shell.c
 A printsem semaphore is created to make sure that only one of the processes print at a given time on the console. 
1) xsh_gen.c
    gen | search generates words at random from a file wordlist.h. A writer process writes the words into a pipe and a reader process reads continuously from the pipe. 
    Gen prints the number of words generated every 5 seconds. It waits for printsem sempahore before printing and signals printsem after printing. 
    It creates a pipe and a search process passing the pip id as parameter. 
    The pipe is connected with gen as writer and search as reader process.  
2) xsh_search.c 
    searchpipe(pip) functions reads a word from the pipe and calls the countvowels method which returns the number of words starting with all vowels ( an array is maintained for this
    purpose. It waits for printsem sempahore before printing and signals printsem after printing. 
