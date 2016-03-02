#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LOW 12
#define HIGH 30

int countlines(FILE *file);
int readFile(const char *fname, double ***coords);
int checkCollision(double xyz[3]);
int checkTime(struct timespec start,const int MAX_TIME);
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

    struct timespec start, end;
    double **coords = NULL;
    int i;
    int numOfCollisions = 0;
    int finished = 0;

    // Read file and write content in coords
    int noOfLines = readFile(FILENAME, &coords);
    if (!noOfLines) {
        fprintf(stderr, "No data given\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC,  &start);    
    // Check points
    for (i=0; i<noOfLines && !finished; i++) {
        if (checkCollision(coords[i])) numOfCollisions++;
        if (MAX_COLLISIONS != -1 && numOfCollisions >= MAX_COLLISIONS)
            finished = 1;
        if(MAX_TIME > -1)
        	if (checkTime(start, MAX_TIME)) return 0;//exit program if time exceeded
        
    }
    clock_gettime(CLOCK_MONOTONIC,  &end);
    double secs = calcTime(start, end);

    printf("Processing ratio: %f coordinates/sec \n", noOfLines/secs);
    printf("%d\n", numOfCollisions);
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
  int i, collision = 1;
  for (i=0;i<3;i++)
      if (xyz[i] < LOW || xyz[i] > HIGH)
          collision = 0;

  return collision;
}

double calcTime(struct timespec start, struct timespec end) {
    const int DAS_NANO_SECONDS_IN_SEC = 1000000000;
    long timeElapsed_s = end.tv_sec - start.tv_sec;
    long timeElapsed_n = end.tv_nsec - start.tv_nsec;
    if ( timeElapsed_n < 0 ) {
        timeElapsed_n = DAS_NANO_SECONDS_IN_SEC + timeElapsed_n;
        timeElapsed_s--;
    } 
    printf("Time: %ld.%09ld secs \n",timeElapsed_s,timeElapsed_n);

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
