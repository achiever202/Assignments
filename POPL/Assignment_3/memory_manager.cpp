#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define max_blocks 10000000
#define ll long long unsigned

using namespace std;

int *buffer = NULL;

/*
 * This class maintains the registry of all the allocated blocks in the buffer.
 * @data memory_index: it stores the index of the allocated block in buffer.
 * @data block_size: it stores the size of the allocated block.
 * @data reference_count: it stores the count of references to the block.
 */
class registry
{
	public:
		int memory_index;
		int block_size;
		int reference_count;

		/*
		 * Constructor for the objects.
		 * @param inde: the index of the memory block allocated.
		 * @param size: the size of the block allocated.
		 * It sets the reference count as 1.
		 */
		registry(int index, int size)
		{
			memory_index = index;
			block_size = size;
			reference_count = 1;
		}

		/*
		 * ++ operator overloaded for the objects of this class.
		 * It increments the reference count of the block.
		 */
		void operator++()
		{
			this->reference_count = this->reference_count + 1;
		}

		/*
		 * -- operator overloaded for the objects of this class.
		 * It decrements the reference count of the block.
		 */
		void operator--()
		{
			this->reference_count = this->reference_count - 1;
		}
};

/* 
 * This function creates a memory buffer which is used for allocation in the program.
 * @param size: the number of blocks to be allocated.
 */
void create_buffer(int size)
{
	/* If size is more than maximum blocks, reporting error and aborting. */
	if(size>max_blocks)
	{
		cout<<"ERROR: Memory too large to be allocated. Aborting.\n";
		exit(0);
	}

	/* Allocating memory to the buffer. */
	buffer = (int*)malloc(sizeof(int)*size);

	/* If memory could not be allocated. */
	if(buffer==NULL)
	{
		cout<<"ERROR: Could not allocated memory. Aborting.\n";
		exit(0);
	}
}

int main()
{
	return 0;
}
