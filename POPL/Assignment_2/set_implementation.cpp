#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

using namespace std;

/* Variables for range of universe. */
int range_start = 0;
int range_end = 0;
int operation = 0;

/* Filename for input and output. */
string input_file_name1 = "";
string input_file_name2 = "";
string output_file_name = "";

/*
 * This class implements the mathematical set structure for integers.
 * @data-member bit_set: a boolean array, that stores the presence of a number in a set.
 * @data-member len: stores the length of range.
 * @method: contains(), add(), remove().
 */
class set
{
	vector<bool>bit_set;
	int len;

	public:
		set()	/* constructor for the class. */
		{
			/* Allocating memory for the boolean array. */
			len = range_end-range_start+1;

			/* Initializing the array. */
			for(int i=0; i<len; i++)
				bit_set.push_back(false);

            cout<<range_end<<" "<<range_start<<" "<<len<<endl;
            for(int i=0; i<len; i++)
                cout<<bit_set[i]<<" ";
            cout<<endl;

		}

		/*
		 * This function checks if an integer is present in the set.
		 * @param n: the integer to be checked.
		 * return bool: true if the integer is present, else false.
		 */
		bool contains(int n)
		{
			return bit_set[n-range_start];
		}

		/*
		 * This function adds an integer to the set.
		 * @param n: the integer to be added.
		 * return bool: true if the integer was added, false if the integer was already present.
		 */
		bool add(int n)
		{
			if(contains(n))
				return false;

			bit_set[n-range_start].flip();
			return true;
		}

		/*
		 * This function removes an integer to the set.
		 * @param n: the integer to be removed.
		 * return bool: true if the integer was removed, false if the integer was not present.
		 */
		bool remove(int n)
		{
			if(!contains(n))
				return false;

			bit_set[n-range_start].flip();
			return true;
		}

		/*
		 * This function takes the union of a set with the current set.
		 * @param b: the set with which the union operation is to be done.
		 * @return c: the set formed after the union operation.
		 */
		set set_union(set b)
		{
			set c;

			/* for each number in range check if it is present in b or in current set, then add. */
			for(int i=range_start; i<=range_end; i++)
			{
				if(b.contains(i) || contains(i))
					c.add(i);
			}

			return c;
		}

		/*
		 * This function takes the intersection of a set with the current set.
		 * @param b: the set with which the intersection operation is to be done.
		 * @return c: the set formed after the intersection operation.
		 */
		set set_intersection(set b)
		{
			set c;

			/* for each number in range check if it is present in b and and in the current set, then add. */
			for(int i=range_start; i<=range_end; i++)
			{
				if(b.contains(i) && contains(i))
					c.add(i);
			}

			return c;
		}

		/*
		 * This function takes the difference of a set with the current set (A-B).
		 * @param b: the set with which the difference operation is to be done (B).
		 * @return c: the set formed after the difference operation (A-B).
		 */
		set set_difference(set b)
		{
			set c;

			/* for each member, checking if it is present in current set, and not in set b. */
			for(int i=range_start; i<=range_end; i++)
				if(contains(i) && !b.contains(i))
					c.add(i);

			return c;
		}

		/*
		 * This function takes the set difference of two sets.
		 * @param b: the set with which the set difference operation is to be done.
		 * @return c: the set formed after the set difference operation.
		 */
		set set_set_difference(set b)
		{
			/*
			 * first the difference of b with current set is taken.
			 * then, the difference of current set with b is taken.
			 * then, the union of second with first is taken.
			 */
			return set_difference(b).set_union(b.set_difference(*this));
		}
};

/*
 * This function parses the range provided in the console to get the range values.
 * @param range: the range argument in the console.
 * @return void: it just sets the range global variables.
 */
void parse_range(char *range)
{
	/* len stores the length of range string. */
	int len = strlen(range);

	int i=0;
	/* parsing the start index. */
	while(i<len && range[i]!='-')
	{
		/* if it contains invalid characters. */
		if(!(range[i]>='0' && range[i]<='9'))
		{
			cout<<"Invalid range!";
			exit(0);
		}

		range_start = range_start*10 + range[i]-'0';
		i++;
	}

	i++;

	/* parsing the end of the range. */
	while(i<len && range[i]!='-')
	{
		/* if it contains invalid characters. */
		if(!(range[i]>='0' && range[i]<='9'))
		{
			cout<<"Invalid range!";
			exit(0);
		}

		range_end = range_end*10 + range[i]-'0';
		i++;
	}

	/*
	 * Error in case of invalid ranges such as,
	 * 		When end index is smaller than start index. (3000-1000)
	 *		When no start or end range is provided. (-1000, 1000-)
	 */
	if(range_end<range_start)
	{
		cout<<"Invalid range!";
		exit(0);
	}
}


/*
 * This function parses the argument for the desired operation to perform.
 * @param op: command line argument.
 * @return void: does not return anything.
 */
void parse_operation(char *op)
{
	int len = strlen(op);

	/* changing the upper case characters to lower case to support case-insensitiveness. */
	for(int i=0; i<len; i++)
	{
		if(!(op[i]>='a' && op[i]<='z'))
			op[i] = op[i]^' ';
	}

	/* checking for the desired operation. */
	if(strcmp(op, "union")==0)
		operation = 1;
	else if(strcmp(op, "intersection")==0)
		operation = 2;
	else if(strcmp(op, "difference")==0)
		operation = 3;
	else if(strcmp(op, "symmetric difference")==0)
		operation = 2;
	else
	{
		/* No valid operation found. */
		cout<<"Invalid Operation!";
		exit(0);
	}
}


/*
 * This function parses the arguments passed through the console.
 * @param args: number of arguments passed.
 * @param argc: argument list passed as pointer to array of characters.
 * @return void: it parses the arguments and sets the global variables, doesnt return anything.
 */
void parse_arguments(int args, char **argc)
{
	/* Error if more than five arguments. */
	if(args>6)
	{
		cout<<"Invalid arguments.!";
		exit(0);
	}

	for(int i=1; i<args; i++)
	{
		/* Parsing the range argument. */
		if(i==1)
			parse_range(argc[1]);

		/* Parsing the operation argument. */
		if(i==2)
			parse_operation(argc[2]);

		/* First input file. */
		if(i==3)
			input_file_name1 = (string)argc[3];

		/* Second input file. */
		if(i==4)
			input_file_name2 = (string)argc[4];

		/* Output file name. */
		if(i==5)
			output_file_name = (string)argc[5];
	}
}

/*
 * This function creates a set from the numbers in a file.
 * @param filename: name of the file to be read.
 * @param s: pointer to the set which is to be created.
 * @return void: it just creates a set and doesn't return anything.
 */
void create_set(string filename, set *s)
{
	int num;

	/* Opening the given file. */
	FILE *input = fopen(filename.c_str(), "r");

	/* Error while opening file. */
	if(input==NULL)
	{
		cout<<"ERROR: could not open file "<<filename<<".";
		exit(0);
	}

	/* Reading numbers from file. */
	while(fscanf(input, "%d", &num)!=EOF)
	{
		/* If numbers out of range. */
		if(num<range_start || num>range_end)
		{
			cout<<"ERROR: number not in the specified range.";
			exit(0);
		}

		/* Adding the numbers to the set. */
		if(!s->add(num))
		{
			/* If duplicate number found. */
			cout<<"ERROR: Duplicate number found in file "<<filename<<"."<<endl;
			cout<<"The number duplicated is: "<<num<<endl;
			exit(0);
		}
	}

	/* Closing the file. */
	fclose(input);
}


/*
 * This function writes the contents of a set to a file.
 * @param filename: name of the file to be written to.
 * @param s: set that has to be written to file.
 * @return void: doesnt return anything.
 */
void write_set(string filename, set s)
{
	int num;

	/* Opening the given file. */
	FILE *output = fopen(filename.c_str(), "w");

	/* Error while opening file. */
	if(output==NULL)
	{
		cout<<"ERROR: could not open file "<<filename<<".";
		exit(0);
	}

	/* if the set contains a number in the range, write to file. */
	for(int i=range_start; i<=range_end; i++)
		if(s.contains(i))
			fprintf(output, "%d\n", i);

	/* closing the file. */
	fclose(output);
}

void print_set(set s)
{
	for(int i=range_start; i<=range_end; i++)
		if(s.contains(i))
			cout<<i<<endl;
}

int main(int args, char **argc)
{
	/* Parse command line arguments. */
    parse_arguments(args, argc);

    set a, b;
	int len = range_end-range_start+1;

    /* creating sets from file. */
    create_set(input_file_name1, &a);
    create_set(input_file_name2, &b);

    set c = a.set_union(b);
    write_set(output_file_name, c);

    return 0;
}
