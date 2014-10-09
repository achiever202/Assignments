#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#define max_blocks 10000000
#define ll long long unsigned

using namespace std;

int *buffer = NULL;
ll next_id = 0;
int total_size;
int current_index;


/*
 * This class maintains the registry of all the allocated blocks in the buffer.
 * @data memory_index: it stores the index of the allocated block in buffer.
 * @data block_size: it stores the size of the allocated block.
 * @data reference_count: it stores the count of references to the block.
 */
class Registry
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
		Registry(int index, int size)
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

class MyInt
{
	public:
		ll id;

		void update_id(ll id)
		{
			this->id = id;
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
	total_size = size;
	current_index = 0;

	/* If memory could not be allocated. */
	if(buffer==NULL)
	{
		cout<<"ERROR: Could not allocated memory. Aborting.\n";
		exit(0);
	}
}

ll allocate_from_buffer(int size)
{
	return 1;
}

/*
 * This function allocates memory, inserts a registry element in the map and returns the id.
 * @param size: number of blocks of memory to be allocated.
 * @return ll: the id of the allocated registry object.
 * If not enough memory to allocate, it returns -1.
 */
ll allocate_registry(int size)
{
	/* if the object can be directly allocated without compaction. */
	if(total_size-current_index>=size)
	{
		/* Allocating memory from buffer without compaction. */
		ll id = allocate_from_buffer(size);
		return id;
	}

	/* compacting the memory. */
	//compact_memory();

	/* if memory can be allocated after compaction. */
	if(total_size-current_index>=size)
	{
		/* Allocating memory and returning the id. */
		ll id = allocate_from_buffer(size);
		return id;
	}

	/* If not enough memory to allocate, return -1. */
	return -1;
}

/*
 * This function implements the functionality of the new operator in C++.
 * @param size: the number of blocks to be allocated.
 * @return MyInt: the MyInt variable allocated.
 */
MyInt my_new(int size)
{
	/* temporary variable to allocate memory. */
	MyInt temp;

	/* Allocating a new Registry element. */
	ll id = allocate_registry(size);

	/* If could not allocate memory, id of the registry is -1. */
	if(id<0)
	{
		cout<<"No memory left in the buffer. Could not allocate memory using my_new().\n";
		exit(0);
	}
	/* Updating the id of the allocated MyInt. */
	else 
		temp.update_id(id);

	/* returning the object. */
	return temp;
}

int main()
{
	return 0;
}
