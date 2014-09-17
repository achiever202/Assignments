/*
 * This is an API for implementing PID manager.
 * @function allocate_map: allocates memory to PID manager.
 * @function allocate_pid(): allocate a PID.
 * @function release_pid(): release a PID.
 */


#include <stdio.h>
#include <stdlib.h>

/* minimum and maximum PIDs */
#define MIN_PID 300
#define MAX_PID 5000

/*
 * pid_library is the pointer to an integer array that acts as a PID manager.
 * len stores the length of range of PIDs. 
 */
int *pid_library;
int len = MAX_PID - MIN_PID + 1;

/*
 * This function allocates the map that acts as PID manager.
 * It allocates an array of length "len" pointed by pid_library and initializes the array.
 * All the PIDs are initialized as unallocated.
 * @param none: It does not take any arguments.
 * @return int: It returns 1 if the allocation and initialization is successful, else returns -1.
 */
int allocate_map()
{
	/* Allocating memory. */
	pid_library = malloc(len*sizeof(int));

	/* error if could not allocate memory. */
	if(pid_library==NULL)
	{
		printf("ERROR: Could not allocated memory to PID manager.\n");
		return -1;
	}

	/* initializing the array to mark all PIDs as unallocated. */
	int index;
	for(index=0; index<len; index++)
		*(pid_library+index) = 0;

	/* returns 1 after the successful initialization. */
	return 1;
}


/*
 * This function allocates a PID, if one is available.
 * @param none: It does not take any arguments.
 * @return int: returns the PID allocated if available, else returns -1.
 */
int allocate_pid(void)
{
	int index;

	/* if the PID manager is NULL, return error. */
	if(pid_library==NULL)
	{
		printf("Error: No  PID manager allocated.\n");
		return -1;
	}

	/* traversing the array for any available PID. */
	for(index=0; index<len; index++)
	{
		if(*(pid_library+index)==0)
		{
			*(pid_library+index) = 1;
			return index + MIN_PID;
		}
	}

	return -1;
}


/*
 * This function releases a PID.
 * @param pid: the PID to be released.
 * @return int: returns the PID allocated if available, else returns -1.
 */
void release_pid(int pid)
{
	/* if the PID manager is NULL, report error. */
	if(pid_library==NULL)
	{
		printf("Error: No  PID manager allocated.\n");
		return;
	}

	/* if the PID is out of range, report error. */
	if(pid<MIN_PID || pid>MAX_PID)
	{
		printf("Error: PID out of range (%d-%d).\n", MIN_PID, MAX_PID);
		return;
	}

	/* deallocating PID. */
	*(pid_library + pid - MIN_PID) = 0;
}