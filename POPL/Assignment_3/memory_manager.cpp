#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

class free_node
{
	public:
		void *free_block;
		int size;
		free_node *next;
}*free_list = NULL;

class node
{
	public:
		int num;
		node *next;
};

/*
 * cur_heap_head stores the head of the heap pointer.
 * cur_heap_length stores the current size of the continuos heap in bytes.
 */
void *cur_heap_head = NULL;
int cur_heap_length;

void create_buffer()
{
	cout<<"Enter the size of the buffer: ";
	cin>>cur_heap_length;

	cur_heap_head = malloc(cur_heap_length);
	if(cur_heap_head==NULL)
	{
		cout<<"ERROR: Could not create the heap.";
		exit(0);
	}
}

void delete_element(node **list_head)
{
	cout<<"Deleting element.\n";
	cur_heap_head = *list_head;
	cur_heap_length += sizeof(node);

	*list_head = (*list_head)->next;
}

void* my_new(int size, node **list_head)
{
	if(cur_heap_length<size)
	{
		cout<<"ERROR: Not enough size on heap.\n";
		delete_element(list_head);
	}

	void *temp = cur_heap_head;
	cur_heap_head += size;
	cur_heap_length -= size;
	return temp;
}

void insert_element(int num, node **list_head)
{
	node *temp = (node*)my_new(sizeof(node), list_head);
	if(temp==NULL)
		return;

	temp->num = num;
	temp->next = *list_head;
	*list_head = temp;
}

void show(node *list_head)
{
	node *temp = list_head;
	while(temp!=NULL)
	{
		cout<<temp->num<<" ";
		temp = temp->next;
	}
	cout<<endl;
}

int main()
{
	create_buffer();

	node *list_head = NULL;
	int n = 1;
	while(n)
	{
		cin>>n;
		insert_element(n, &list_head);
		show(list_head);
	}
	return 0;
}
