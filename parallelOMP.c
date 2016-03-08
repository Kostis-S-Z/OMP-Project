#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#define LOW 12
#define HIGH 30


int countlines(FILE *file);
int readFile(const char *fname, double ***coords);
int checkCollision(double xyz[3]);
int checkInRange(double **coords, int num,
                    const int MAX_TIME,
                    const int MAX_COLLISIONS,
                    struct timespec start);
int checkTime(struct timespec start,const int MAX_TIME);
void printResults(double totalTime, double checkingTime, int inRange, int numOfCollisions);
double calcTime(struct timespec start,struct timespec end);
void freeCoords(double ***coords, int no_of_lines);

int main(int argc, char *argv[]) {
    // Program arguments and checking
    if (argc != 6) {
        fprintf(stderr, "Incorrect number of parameters\n");
        return 1;
    }
    const int MAX_COLLISIONS = atoi(argv[1]);
    const int MAX_TIME = atoi(argv[2]);
    const char* FILENAME = argv[3];
    if (FILENAME == NULL) {
        fprintf(stderr, "No filename given\n");
        return 1;
    }
    const int MAX_THREADS = atoi(argv[4]);
    if(MAX_THREADS > 0)
    	omp_set_num_threads(MAX_THREADS);
    const int MAX_PROCESSES = atoi(argv[5]);

    struct timespec start, cStart, end;
    double **coords = NULL;
    int inRange;

    // Read file and write content in coords
    clock_gettime(CLOCK_MONOTONIC,  &start);
    int noOfLines = readFile(FILENAME, &coords);
    if (!noOfLines) {
        fprintf(stderr, "No data given\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC,  &cStart);

    // Find the number of coordinates that are in range
    inRange = checkInRange(coords, noOfLines, MAX_TIME, MAX_COLLISIONS, start);

    clock_gettime(CLOCK_MONOTONIC,  &end);

    double secs = calcTime(start, end);
    double cSecs = calcTime(cStart, end);
    printResults(secs, cSecs, inRange, noOfLines);
    freeCoords(&coords, noOfLines);
    return 0;
}


int readFile(const char *fname, double ***coords) {
    FILE *fp;

    fp = fopen(fname,"r");//read only

    if (fp == NULL) {
        fprintf(stderr,"Error while opening file %s\n", fname);
        return 0;
    }

    double ** temp;
    temp = (double **)malloc(1000*sizeof(double *));

    int i=0;
    while (!feof(fp)) {
        temp[i] = (double *)malloc(3*sizeof(double));
        fscanf(fp,"%lf",&temp[i][0]);
        fscanf(fp,"%lf",&temp[i][1]);
        fscanf(fp,"%lf",&temp[i][2]);
        if (i%1000 == 0)
            temp = (double **)realloc(temp,(i+1000)*sizeof(double *));
        i++;
    }
    *coords = temp;

    fclose(fp);

    return i--;
}

int checkCollision(double xyz[3]) {
    int i, inRange = 1;
    for (i=0;i<3 && inRange;i++)
        if (xyz[i] < LOW || xyz[i] > HIGH)
            inRange = 0;


  return inRange;
}

int checkInRange(double **coords, int num,
                    const int MAX_TIME,
                    const int MAX_COLLISIONS,
                    struct timespec start) {

    int inRange = 0;
    int i;
    int finished;


    if (MAX_COLLISIONS != -1)
        num = MAX_COLLISIONS;
    #pragma omp parallel private(i,finished) shared(inRange)
    {
    finished = 0;
    #pragma omp for reduction(+:inRange)
    for (i=0; i<num ; i++) {
        if ( finished ) i = num;
        if (checkCollision(coords[i]))
                inRange++;

        if(MAX_TIME > -1)
            if (checkTime(start, MAX_TIME)) finished = 1; //exit program if time exceeded
    }
    }

    return inRange;
}

void printResults(double totalTime, double checkingTime,
        int inRange, int numOfCollisions) {
    printf("Number of coordinates: %d\n", numOfCollisions);
    printf("Number of coordinates in range: %d\n", inRange);
    printf("In range ratio: %f\n", numOfCollisions/(double)inRange);
    printf("Processing ratio: %f coordinates/sec \n", numOfCollisions/totalTime);
    printf("Total time: %f secs \n", totalTime);
    printf("Time without I/O: %f secs \n", checkingTime);
}

double calcTime(struct timespec start, struct timespec end) {
    const int DAS_NANO_SECONDS_IN_SEC = 1000000000;
    long timeElapsed_s = end.tv_sec - start.tv_sec;
    long timeElapsed_n = end.tv_nsec - start.tv_nsec;
    if ( timeElapsed_n < 0 ) {
        timeElapsed_n = DAS_NANO_SECONDS_IN_SEC + timeElapsed_n;
        timeElapsed_s--;
    }

    double secs = timeElapsed_s + timeElapsed_n/1000000000.0;

    return secs;
}

int checkTime(struct timespec start, const int MAX_TIME){
		struct timespec current;
		clock_gettime(CLOCK_MONOTONIC,&current);
        if (current.tv_sec-start.tv_sec >= MAX_TIME){
        	printf("maximum time exceeded! \n");
        	printf("%ld \n", current.tv_sec-start.tv_sec);
        	return 1;
        }

        return 0;
}

void freeCoords(double ***coords, int no_of_lines) {
    int j=0;
    for (j=0;j<no_of_lines;j++) {
        free((*coords)[j]);
    }
    free(*coords);
}
