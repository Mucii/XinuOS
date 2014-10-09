Part 1 - BLocking Message Send

Files modified/added 	- /system/sendb.c, /system/receiveb.c, ./include/process.h, ./system/initialize.c, ./include/queue.h

A fifo buffer (implemented using an int array of size 3 inside process block) is used to store the messages. 
If the receiver process' buffer is full, the sender has to wait and moves to PR_SND state, adds itself to the 
receiver's blocked processes queue and calls resched. If the receiver process' buffer is empty, receiveb is 
put in a blocking state and calls resched(). On receiving a message, the receiver checks to see if it has 
any processes waiting in its queue, and picks out a process based on FIFO policy.   
Modifications to procent are :

        

Part 2 - Asynchronous Message Receive

Files added	 - ./system/registerreceive.c (new), ./system/send.c, ./system/sendb.c, ./include/process.h, ./system/initialize.c

Two entries are added to the procent table (int (* cbfunc)(void), uint32 *buf) for the callback function (in process.h)

The two pointers are maintained in the process stack of the process which calls registerrecv. Here, the kernal executes the 
user-supplied code.       
In order to register a callback function, a process calls registerrecv with the address of the callback function and the 
buffer to store the message. Before sending a message to a receiver, sendb and send checks to see if the callback function
(cbfunc) is not null. If a valid callback function exists, it is executed. 

In order to have a "safe" implementation, the callback function should not be executed in the sender process' context. A new process
can be created which only executes the callback function. Thus the callback function is executed in the context of the newly created
process and not the sender's context. 


Part 3 - Garbage collection Support

Files added/modified	: ./system/freememgb.c, ./system/getmemgb.c, ./include/process.h, ./system/kill.c, ,./system/initialize.c

A linked list similar to memblk (struct memblkgb which has uint32 mlength and struct memblkgb *mnext as entries) is used to store the blocks 
that are allocated from the heap memory. When a memory block of n bytes is requested, nbytes is rounded off to the nearest byte and the freelist
is searched for availabe memory. If a block is obtained, it is added to the memblkgb of the current process. Freememgb iterates through the memblkgb structure maintained by all the processes to find a block
matching the specified starting location and the number of bytes.

A pointer to the head of the (   struct memblkgb   *prmemlistgb; ) the memblkgb is stored in PCB so that we can keep track of the allocated 
memory for every process. Memblkgb is arranged in the increasing order of the address. 
When a process is killed, all the allocated heap memory of that process is returned to the freelist by traversing through each allocated block
for the corresponding process and freeing them. The stack region is not freed up by the garbage collector because when a process is killed, 
freestk is called to free up the alloted stack region for a process.

The user can only free an allocated block as a whole and cannot split it up for freeing. If the user has allocated 200 bytes of memory starting 
from x, and tries to free memory less than 200 bytes, freememgb returns SYSERR.
