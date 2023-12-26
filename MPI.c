#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#include <mpi.h>

#define ISIZE 2000
#define JSIZE 2000
#define len 1999

int comm_size, my_rank;

int div_begin(int rank)
{
	int segment_len = (len) / comm_size;
	return rank * segment_len;
}

int div_end(int rank)
{
	if (rank == comm_size - 1)
		return len;
	else return div_begin(rank+1);
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	double *a = malloc(ISIZE * JSIZE * sizeof(*a));
	int i, j;
	FILE *ff;
	for (i = 0; i < ISIZE; i++)
	{
		for (j = 0; j < JSIZE; j++)
		{
			a[i * ISIZE + j] = 10 * i + j;
		}
	}
	// начало измерения времени
	double start = MPI_Wtime();

	int begin = div_begin(my_rank);
	int end = div_end(my_rank);

	for (i = 1; i < ISIZE; i++)
	{
		for (j = begin; j < end; j++)
		{
			a[i * ISIZE + j] = sin(2 * a[(i - 1) * ISIZE + j + 1]);
		}

		if (my_rank != 0)
			MPI_Send(&a[i * ISIZE + begin], 1, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD);
	
		if (my_rank < comm_size - 1)
			MPI_Recv(&a[i * ISIZE + div_begin(my_rank+1)], 1, MPI_DOUBLE, my_rank + 1,
				 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	

	begin = div_begin(my_rank);
	end = div_end(my_rank);

	for (i = 1; i < ISIZE; i++)
	{
		if (my_rank != 0)
		{
			MPI_Send(&a[i * ISIZE + begin], end - begin, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		}

		if (my_rank == 0)
		for (int rank = 1; rank < comm_size; rank++)
		{
			begin = div_begin(rank);
			end = div_end(rank);
			MPI_Recv(&a[i * ISIZE + begin], end - begin, MPI_DOUBLE, 
				rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		}
	
	}


	// окончание измерения времени
	if (my_rank == 0)
	{
		double finish = MPI_Wtime();
		printf("Time: %lg\n", finish - start);
	}
	if (my_rank == 0)
	{
		ff = fopen("result.txt", "w");
		for (i = 0; i < ISIZE; i++)
		{
			for (j = 0; j < JSIZE; j++)
			{
				fprintf(ff, "%f ", a[i * ISIZE + j]);
			}
			fprintf(ff, "\n");
		}
		fflush(ff);
		fclose(ff);
	}

	MPI_Finalize();
}
