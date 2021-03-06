#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <algorithm>

#define max_blocks 10000000
#define ll long long int
#define pr pair<ll, Registry>

using namespace std;

int *buffer = NULL;
ll next_id = 0;
int total_size;
int current_index;
int dummy_memory;

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
};

map<ll, Registry> registry_map;

/*
 * This class defines objects that store the id of the allocated registry element. 
 * @data id: id of the registry element.
 */
class MyInt
{
	public:
		ll id;

		/* simple constructor for the object. */
		MyInt()
		{
			id = -1;
		}

		/* copy constructor for the object. */
		MyInt(const MyInt &b)
		{
			/* updating the id of the object. */
			this->id = b.id;

			/* increasing the reference count of the object being copied. */
			map<ll, Registry>::iterator it = registry_map.find(b.id);
			if(it!=registry_map.end())
				it->second.reference_count++;
		}

		/* overloading the assignment operator for the objects. */
		MyInt& operator=(MyInt const &b)
		{
			/* updating the reference count for the lvalue. */
			map<ll, Registry>::iterator it = registry_map.find(this->id);
			if(it!=registry_map.end())
			{
				it->second.reference_count--;
			}

			/* updating the reference count for the rvalue. */
			it = registry_map.find(b.id);
			if(it!=registry_map.end())
			{
				it->second.reference_count++;
				this->id = b.id;
			}
			else
			{
				this->id = -1;
				cout<<"ERROR: Right side of assignment does Not point to a valid memory location.\n";
			}
			return *this;
		}

		/* overloading the [] operator for objects for access. */
		int& operator[](const int &index)
		{
			/* indexing the memory allocated. */
			map<ll, Registry>::iterator it = registry_map.find(this->id);

			if(it!=registry_map.end())
			{	
				/* if out of bounds access. */
				int memory_index = it->second.memory_index;
				if(index>=it->second.block_size || index<0)
				{
					cout<<"ERROR: Memory out of bound being accessed. Prone to segmentation faults and erraneous results.\n";
					return dummy_memory;
				}

				/* adding the offset to the base. */
				memory_index += index;
				return buffer[memory_index];
			}
			else
			{
				cout<<"ERROR: Invalid memory address.\n";
				return dummy_memory; 
			}
		}

		void update_id(int id)
		{
			map<ll, Registry>::iterator it = registry_map.find(this->id);
			if(it!=registry_map.end())
			{
				it->second.reference_count--;
			}
			else
			{
				//cout<<"ERROR: The previous id not valid.\n";
			}

			it = registry_map.find(id);
			if(it!=registry_map.end())
			{
				this->id = id;
				it->second.reference_count++;
			}
			else
			{
				this->id = -1;
				//cout<<"ERROR: The new id not valid.\n";
			}
		}

		/* Destructor for the objects. */
		~MyInt()
		{
			map<ll, Registry>::iterator it = registry_map.find(this->id);

			if(it!=registry_map.end())
				it->second.reference_count--;
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
	/* Creating new Registry element allocated from current_index and of given size. */
	Registry new_registry_element(current_index, size);

	/* Updating the current index. */
	current_index = current_index+size;

	/* Inserting the new registry element in the map. */
	registry_map.insert(pr(next_id, new_registry_element));

	/* Updating the next id to be allcated. */
	next_id++;

	/* returning the id of the allocated registry element. */
	return next_id-1;
}

/* Comparison function to sort the vector of registry elements. */
bool comp(pr i, pr j)
{
	/* return true if the memory index of the first element is smaller. */
	return i.second.memory_index<j.second.memory_index;
}

/* This function does memory compaction. */
void compact_memory()
{
	/* Creating a vector to store the current registry elements in the map. */
	vector <pr> registry_vector;

	/* Iterating over the map, and pushing elements into the vector. */
	map<ll, Registry>::iterator it = registry_map.begin();
	while(it!=registry_map.end())
	{
		if(it->second.reference_count>0)
			registry_vector.push_back(make_pair(it->first, it->second));
		it++;
	}

	/* Sorting the vector by the memory index. */
	sort(registry_vector.begin(), registry_vector.end(), comp);

	/* Compacting memory and moving memory blocks. */
	ll cur_index = 0;
	for(ll i=0; i<registry_vector.size(); i++)
	{
		/* If the cur_index is the one that is occupied. */
		if(registry_vector[i].second.memory_index==cur_index)
		{
			cur_index += registry_vector[i].second.block_size;
		}

		/* If the memory block can be moved. */
		else if(cur_index<registry_vector[i].second.memory_index)
		{
			/* Copying the data until the block size. */
			for(ll j=0; j<registry_vector[i].second.block_size; j++)
			{
				buffer[cur_index+j] = buffer[registry_vector[i].second.memory_index + j];
			}

			/* Updating the memory index of the registry element. */
			registry_vector[i].second.memory_index = cur_index;

			/* Updating the current index to be allocated. */
			cur_index += registry_vector[i].second.block_size;
		}
	}

	/* Updating the current index of the buffer to be allocated. */
	current_index = cur_index;

	/* Erasing all the elements in the map. */
	registry_map.erase(registry_map.begin(), registry_map.end());

	/* Inserting all the elements back into the map. */
	for(ll i=0; i<registry_vector.size(); i++)
	{
		registry_map.insert(registry_vector[i]);
	}
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
	compact_memory();

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
		return temp;
	}

	/* Updating the id of the allocated MyInt. */
	else 
		temp.id = id;

	/* returning the object. */
	return temp;
}

/*
 * This function implements the functionality of the delete operator. 
 * @param num: the MyInt element to be deleted.
 */
void my_delete(MyInt *num)
{
	/* Finding the registry element associated with the MyInt element in the map. */
	map<ll, Registry>::iterator it = registry_map.find(num->id);

	/* Checking if the element exists. */
	if(it!=registry_map.end())
	{
		/* Decreasing the reference count of the registry element. */
		it->second.reference_count--;
	}
	else
	{
		/* Element not found in the map. */
		cout<<"ERROR: No reference to any valid element found.\n";
	}

	num->id = -1;
}

/* for debugging. */
void show_registry()
{
	cout<<"Dump:\n";

	map<ll, Registry>::iterator it = registry_map.begin();
	while(it!=registry_map.end())
	{
		cout<<it->first<<" "<<it->second.memory_index<<" "<<it->second.block_size<<" "<<it->second.reference_count<<" "<<buffer[it->second.memory_index]<<endl;
		it++;
	}
}

/*
 * This class is used to implement linked list of integers.
 * The object contains the head of the linked list.
 */
class list
{
		/* head of the linked list. */
		MyInt head;

	public:

		/* constructor for the object, initialize the id of head to -1 to point to NULL. */
		list()
		{
			head.update_id(-1);
		}

		/*
		 * This function is used to insert an element into the list.
		 * @param num: the number to be inserted.
		 */
		void list_insert(int num)
		{	
			/* if the list is currently empty. */
			if(head.id==-1)
			{
				/* allocate a node. */
				head = my_new(2);
				
				/* if memory could not be allocated. */
				if(head.id==-1)
					return;

				/* storing the number, and updating the next pointer. */
				head[0] = num;
				head[1] = -1;
			}
			else
			{
				/* inserting the integer at the front of the list. */
				MyInt temp;
				temp = head;
					
				/* allocating a node. */
				head = my_new(2);

				/* if memory could not be allocated. */
				if(head.id==-1)
				{
					head.update_id(temp.id);
					return;
				}

				/* storing the number, and updating the next node. */
				head[0] = num;
				head[1] = temp.id;

				/* updating the refernce count. */
				map<ll, Registry>::iterator it = registry_map.find(temp.id);
				if(it!=registry_map.end())
					it->second.reference_count++;
			}
		}

		/*
		 * This function deletes and element from the list.
		 * @param num: the number to be deleted.
		 */
		void list_delete(int num)
		{
			/* if the list is empty. */
			if(head.id==-1)
			{
				cout<<"ERROR: Element not found.\n";
				return;
			}

			/* if the first element is the number to be deleted. */
			if(head[0]==num)
			{
				/* delete the element and update the head of the list. */
				MyInt temp = head;
				head.update_id(head[1]);

				/* update the reference count. */
				map<ll, Registry>::iterator it = registry_map.find(temp[1]);
				if(it!=registry_map.end())
					it->second.reference_count--;

				/* deallocate memory. */
				my_delete(&temp);
			}
			else
			{
				/* traversing the list to find the element. */
				MyInt temp;
				temp = head;

				MyInt temp2;
				temp2.update_id(temp[1]);

				while(temp2.id!=-1)
				{
					if(temp2[0]==num)
					{
						/* updating the pointers, and reference count. */
						temp[1] = temp2[1];

						map<ll, Registry>::iterator it = registry_map.find(temp2.id);
						if(it!=registry_map.end())
							it->second.reference_count--;

						/* deallocating memory. */
						my_delete(&temp2);
						return;
					}

					/* updating the pointers. */
					temp.update_id(temp[1]);
					temp2.update_id(temp2[1]);
				}

				/* if element not found. */
				cout<<"ERROR: Element not found.\n";
			}
		}

		/* This function displays the contents of the list. */
		void list_show()
		{
			/* traversing the list. */
			MyInt temp = head;
			while(temp.id!=-1)
			{
				cout<<temp[0]<<" ";
				temp.update_id(temp[1]);
			}
			cout<<endl;
		}

		/* destructor for the linked list. */
		/*~list()
		{
			cout<<"in\n";
			MyInt temp;
			while(head.id!=-1)
			{
				cout<<"dest"<<" "<<head.id<<endl;
				temp = head;
				cout<<head[1]<<endl;
				head.update_id(head[1]);
				my_delete(&temp);
			}
			cout<<"out\n";
		}*/
};



int main()
{
	string filename;
	cout<<"Enter the filename: ";
	cin>>filename;

	char input[100];
	int memory_size, num;
	list interger_list;
	char ch;

	FILE *ptr = fopen(filename.c_str(), "r");
	if(ptr==NULL)
	{
		cout<<"ERROR: Could not open the file. Please enter the correct filename.\n";
		cout<<"ERROR: Aborting Program.\n";
		return 0;
	}

	while(fscanf(ptr, "%s", input)!=EOF)
	{
		if(strcmp(input, "memSize")==0)
		{
			fscanf(ptr, "%d", &memory_size);
			create_buffer(memory_size);
		}
		else if(strcmp(input, "insert")==0)
		{
			fscanf(ptr, "%d", &num);
			interger_list.list_insert(num);
		}
		else if(strcmp(input, "delete")==0)
		{
			fscanf(ptr, "%d", &num);
			interger_list.list_delete(num);
		}
		else if(strcmp(input, "show")==0)
		{
			interger_list.list_show();
		}
		else
		{
			cout<<"ERROR: Not a valid operation.\n";
		}
	}


	delete[](buffer);
	return 0;
}
