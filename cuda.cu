#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

#define BLOCKSIZE 1024

__global__ void cu_examine(double*,const int, int*);
double calcTime(struct timespec start,struct timespec end);

extern "C" int checkInRange(double *coords, int num)  
{
	int answer;
	int memory = num*sizeof(double);
	int * counter;
	int error = 0;
	double *d_coords;	

	error = cudaMalloc((void**)&d_coords, memory);
	error = cudaMalloc((void**)&counter, sizeof(int));
	error = cudaMemcpy(d_coords, coords, memory, cudaMemcpyHostToDevice);
	error = cudaMemset(counter, 0, sizeof(int));
	struct timespec start, end;	

	dim3 dimblock(BLOCKSIZE);
	dim3 dimgrid((int)(ceil((double)num/3/BLOCKSIZE)));

	clock_gettime(CLOCK_MONOTONIC, &start);
	cu_examine<<<dimgrid, dimblock>>>(d_coords, num, counter);
	cudaDeviceSynchronize();
	clock_gettime(CLOCK_MONOTONIC, &end);

	error = cudaMemcpy(&answer, counter, sizeof(int), cudaMemcpyDeviceToHost);

	if(error != cudaSuccess)
		printf("Something went wrong!\n");
	printf("kernel exec time: %f\n",calcTime(start, end));	
		
	cudaDeviceReset();
	return answer;
}

__global__ void cu_examine(double* coords, const int border, int *counter) 
{
	const int low = 12;
	const int high = 30;
	const int tx = threadIdx.x;
	const int bx = blockIdx.x;
	const int bDim = blockDim.x;
	const int b = border;

	const int x = 3* (bx * bDim + tx);
	__shared__ double s_coords[3*BLOCKSIZE];

	s_coords[tx] = coords[x];
	s_coords[tx+1] = coords[x+1];
	s_coords[tx+2] = coords[x+2];

	if( x >= b ) return;

	if ((s_coords[tx] >= low) && (s_coords[tx] <= high) && (s_coords[tx+1] >= low) && (s_coords[tx+1] <= high) && (s_coords[tx+2] >= low) && (s_coords[tx+2] <= high))
	{
		atomicAdd(counter,1);
	}	
}







double calcTime(struct timespec start, struct timespec end)
{
	const int DAS_NANO_SECONDS_IN_SEC = 1000000000;
    	long timeElapsed_s = end.tv_sec - start.tv_sec;
    	long timeElapsed_n = end.tv_nsec - start.tv_nsec;
    	if ( timeElapsed_n < 0 )
	{
        	timeElapsed_n = DAS_NANO_SECONDS_IN_SEC + timeElapsed_n;
        	timeElapsed_s--;
    	}
    	double secs = timeElapsed_s + timeElapsed_n/1000000000.0;
    	return secs;
}
