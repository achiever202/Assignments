/* 
 * This file implements the library function, set_priority() for changing the tickets for a process in lottery scheduling.
 * It takes as input, the tickets to be added or removed from the current process.
 * It then creates a message variable, that is used to send data between processes.
 * It then calls the system call implemented in the PM server, with this message pointer.
 */

#include <lib.h>
#include <unistd.h>
int set_priority(int tickets)
{
	/* creating the message variable and putting the number of tickets in the structure. */
	message m;
	m.m1_i1=tickets;

	/* system call for changing tickets with the message pointer. */
	return _syscall(PM_PROC_NR, SET_TICKET, &m);
}
