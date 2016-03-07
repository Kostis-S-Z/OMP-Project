#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

int countlines(FILE *file) {

    char c;
    int counter=0;

    c=fgetc(file);
    while (c != EOF) {
        if (c == '\n')
            counter++;
        c=fgetc(file);
    }

    rewind(file);//sends pointer at the beggining
    return counter;
}

int readFile(const char *fname, double ***coords) {
    FILE *fp;

    fp = fopen(fname,"r");//read only

    if (fp == NULL) {
        fprintf(stderr,"Error while opening file %s\n", fname);
        return 0;
    }

    int no_of_lines = countlines(fp);
    double ** temp;
    temp = (double **)malloc(no_of_lines*sizeof(double *));
    int k;
    for (k=0;k<no_of_lines;k++) {
        temp[k] = (double *)malloc(3*sizeof(double));
    }

    int i;
    for (i=0;i<no_of_lines;i++) {
        fscanf(fp,"%lf",&temp[i][0]);
        fscanf(fp,"%lf",&temp[i][1]);
        fscanf(fp,"%lf",&temp[i][2]);
    }
    *coords = temp;

    fclose(fp);

    return no_of_lines;
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
    int i;
    int inRange = 0;
    int finished = 0;

    if (MAX_COLLISIONS != -1)
        num = MAX_COLLISIONS;
    printf("NUM%d ", num);
    for (i=0; i<num && !finished; i++) {
        if (checkCollision(coords[i]))
                inRange++;

        if(MAX_TIME > -1)
            if (checkTime(start, MAX_TIME)) return 0; //exit program if time exceeded

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
