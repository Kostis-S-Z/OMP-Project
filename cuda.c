#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int countlines(FILE *file);
int readFile(const char *fname, double **coords);
void printResults(double totalTime, double checkingTime, int inRange, int numOfCollisions);
double calcTime(struct timespec start,struct timespec end);
void freeCoords(double **coords);

extern int checkInRange(double *coords, int num);

int main(int argc, char *argv[])
{
	const char* FILENAME;
	if (argc == 2)
	{
		FILENAME = argv[1];
		if (FILENAME == NULL)
		{
        		fprintf(stderr, "No filename given\n");
        		return 1;
    		}
	}
	else
	{
		fprintf(stderr, "Incorrect number of parameters\n");
        	return 1;
	}

	struct timespec start, cStart, end;
    	double *coords = NULL;
    	int inRange;
    	double secs, cSecs;

    	// Read file and write content in coords
    	clock_gettime(CLOCK_MONOTONIC,  &start);
    	int noOfLines = readFile(FILENAME, &coords);
    	if (!noOfLines)
	{
        	fprintf(stderr, "No data given\n");
        	return 1;
    	}

    	clock_gettime(CLOCK_MONOTONIC,  &cStart);

    	// Find the number of coordinates that are in range
    	inRange = checkInRange(coords, 3*noOfLines);

    	clock_gettime(CLOCK_MONOTONIC,  &end);

    	secs = calcTime(start, end);
    	cSecs = calcTime(cStart, end);
    	printResults(secs, cSecs, inRange, noOfLines);
    	freeCoords(&coords);
    	return 0;
}

/*
 Counts the number of lines of the file
 */
int countlines(FILE *file)
{
	char c;
    	int counter=0;
    	c=fgetc(file);
    	while (c != EOF)
	{
		if (c == '\n')
            	counter++;
        	c=fgetc(file);
    	}

    	rewind(file);//sends pointer at the beggining
    	return counter;
}

/*
 Puts the content of the files the coords array
 */
int readFile(const char *fname, double **coords)
{
	FILE *fp;
	fp = fopen(fname,"r");//read only
	double* temp;
    	if (fp == NULL)
 	{
        	fprintf(stderr,"Error while opening file %s\n", fname);
        	return 0;
    	}

    	int no_of_lines = countlines(fp);
	int memory = 3*no_of_lines*sizeof(double);
    	temp = (double *)malloc(memory);
    	int i;
    	for (i=0; i < no_of_lines*3 ;i++)
	{
		fscanf(fp,"%lf",&(temp[i]));
    	}

    	fclose(fp);
	*coords = temp;
    	return no_of_lines;
}

/*
 Prints the results
*/
void printResults(double totalTime, double checkingTime,
        int inRange, int numOfCollisions) {
    printf("Number of coordinates: %d\n", numOfCollisions);
    printf("Number of coordinates in range: %d\n", inRange);
    printf("In range ratio: %f\n", numOfCollisions/(double)inRange);
    printf("Processing ratio: %f coordinates/sec \n", numOfCollisions/totalTime);
    printf("Total time: %f secs \n", totalTime);
    printf("Time without I/O: %f secs \n", checkingTime);
}

/**
 Calculates the number of seconds elapsed
 */
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

/*
 Frees array
*/
void freeCoords(double **coords)
{
	free(*coords);
}




