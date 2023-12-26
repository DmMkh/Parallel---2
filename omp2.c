#include <stdio.h>
#include <math.h>

int main(int argc, char* argv[])
{
	int N = atoi(argv[1]);
	int i;
	double sum = 0;
#pragma omp parallel for reduction (+: sum)
	for (i = 1; i <= N; i++)
		sum += (double) 1 / (double) i;

	printf("summ is %f\n", sum); 
	return 0;
}