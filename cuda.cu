#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

#define BLOCKSIZE 1024
#define LOW 12
#define HIGH 30

__global__ void cu_examine(double*, int, int*);

extern "C" int checkInRange(double *coords, int num)  
{
	int answer;
	int memory = num*sizeof(double);
	double *d_coords;
	int * counter;
	int error = 0;
	error = cudaMalloc((void**)&d_coords, memory);
	error = cudaMalloc((void**)&counter, sizeof(int));
	error = cudaMemcpy(d_coords, coords, memory, cudaMemcpyHostToDevice);
	error = cudaMemset(counter, 0, sizeof(int));

	dim3 dimblock(BLOCKSIZE);
	dim3 dimgrid((int)(ceil((double)num/3/BLOCKSIZE)));

	cu_examine<<<dimgrid, dimblock>>>(d_coords, num, counter);

	error = cudaMemcpy(&answer, counter, sizeof(int), cudaMemcpyDeviceToHost);

	if(error != cudaSuccess)
		printf("Something went wrong!\n");	
	
	return answer;
}

__global__ void cu_examine(double* coords, int num, int *counter) 
{
	int x = 3* (blockIdx.x * blockDim.x + threadIdx.x);
	if( x >= num ) return;
	if ( (coords[x] > LOW) && (coords[x] < HIGH) && (coords[x+1] > LOW) && (coords[x+1] < HIGH) && (coords[x+2] > LOW) && (coords[x+2] < HIGH))
	{
		atomicAdd(counter,1);
	}
	
}
