# This function updates the memoization matrix.
# It takes three arguments, the start index, the end index, and a middle index.
# It checks for the cost of multiplying matrices obtained after multiplying matrices between these indices.
def update_dp_matrix(i, j, k):
	# using the global lists.
	global dp_matrix, matrices

	# checking if the current product is not obtained, or can be updated.
 	if (dp_matrix[i][j] == -1) or (dp_matrix[i][k]+dp_matrix[k+1][j]+(matrices[i]*matrices[k+1]*matrices[j+1])<dp_matrix[i][j]):
 		dp_matrix[i][j] = dp_matrix[i][k]+dp_matrix[k+1][j]+(matrices[i]*matrices[k+1]*matrices[j+1])


# This function recursively iterates over the matrices for multiplying 'length' number of matrices.
# It takes three arguments, lenght, start_index, end_index.
# It then calculates the minimum cost of multiplying matrices from start_index to end_index uptil now. 
def second_loop(length, start_index, end_index):
	map(lambda k: update_dp_matrix(start_index, end_index, start_index+k), range(length-1))


# This function calculates the minimum cost of multiplying all 'length' number of consecutive matrices.
# It takes two arguments, the 'length' consecutive matrices to be multiplied, and the number of matrices.
def first_loop(length, num_of_matrices):
	map(lambda j: second_loop(length, j, j+length-1), range(num_of_matrices-length+1))


# This function calculates the minimum cost of multiplying n matrices.
def solve(num_of_matrices):
	global dp_matrix
	map(lambda i: first_loop(i, num_of_matrices), range(2, num_of_matrices+1))
	return dp_matrix[0][num_of_matrices-1]


# This function initializes a single row of dp_matrix.
# It initializes the cost of multiplying a series of matrices as -1.
# It initializes the cost of multiplying single matrix as 0.
def init_row(i, num_of_matrices):
	return map(lambda i: -1, range(i)) + [0] + map(lambda i: -1, range(i+1, num_of_matrices))


# This function initializes the dp_matrix.
# It recursively call the init_row() to initialize each row.
def init_dp_matrix(num_of_matrices):
	return map(lambda i: init_row(i, num_of_matrices), range(num_of_matrices))


# This function takes the dimensions of matrix as input.
def input(num_of_matrices):
	return map(lambda i:int(raw_input()), range(num_of_matrices+1))


# The list which stores the dimensions of the matrix.
matrices = input(int(raw_input()))


# The memoization matrix.
dp_matrix = init_dp_matrix(len(matrices)-1)


# This function calculates and prints the result.
print solve(len(matrices)-1)
