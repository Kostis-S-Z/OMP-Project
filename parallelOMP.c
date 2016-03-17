#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>
#define LOW 12
#define HIGH 30


int countlines(FILE *file);
int readFile(char *fname, char *** res, int rank, int numOfProcesses);
int checkCollision(double xyz[3]);
int checkInRange(char ** lines, int num,
                    const int MAX_TIME,
                    const int MAX_COLLISIONS,
                    struct timespec start);
int checkTime(struct timespec start,const int MAX_TIME);
void printResults(double totalTime, double checkingTime, int inRange, int numOfCollisions);
double calcTime(struct timespec start,struct timespec end);
void freeCoords(char ***coords, int no_of_lines);

int main(int argc, char *argv[]) {
    // Program arguments and checking
    if (argc != 6) {
        fprintf(stderr, "Incorrect number of parameters\n");
        return 1;
    }
    const int MAX_COLLISIONS = atoi(argv[1]);
    const int MAX_TIME = atoi(argv[2]);
    char* FILENAME = argv[3];
    if (FILENAME == NULL) {
        fprintf(stderr, "No filename given\n");
        return 1;
    }
    const int MAX_THREADS = atoi(argv[4]);
    if(MAX_THREADS > 0)
    	omp_set_num_threads(MAX_THREADS);
    const int MAX_PROCESSES = atoi(argv[5]);

    struct timespec start, cStart, end;
    char **lines = NULL;
    int inRange;
    int rank, noOfProcesses;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &noOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Read file and write content in coords
    clock_gettime(CLOCK_MONOTONIC,  &start);
    int noOfLines = readFile(FILENAME, &lines, rank, noOfProcesses);
    if (!noOfLines) {
        fprintf(stderr, "No data given\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC,  &cStart);

    // Find the number of coordinates that are in range
    inRange = checkInRange(lines, noOfLines, MAX_TIME, MAX_COLLISIONS, start);

    clock_gettime(CLOCK_MONOTONIC,  &end);

    double secs = calcTime(start, end);
    double cSecs = calcTime(cStart, end);
    printResults(secs, cSecs, inRange, noOfLines);
    free(lines);
    MPI_Finalize();
    return 0;
}


int readFile(char *fname, char *** res, int rank, int numOfProcesses) {
    MPI_File fh;
    MPI_Offset filesize, partsize, start, end;
    char *part, *data;
    char **lines;
    int overlap = 100;
    int textStart, textEnd, numOfLines, i;
    // Open file
    MPI_File_open(MPI_COMM_WORLD, fname, MPI_MODE_RDONLY, MPI_INFO_NULL,&fh);

    MPI_File_get_size(fh, &filesize); // Get size of file in bytes
    partsize = filesize/numOfProcesses; // The number of bytes for each process
    start = rank*partsize; // The start of the part to be read from the process
    end = start+partsize-1; // The end of the part to be read from the process
    if (rank != numOfProcesses-1) { // Add overlap to every process except for the last
        end += overlap;
    } else {
        end = filesize;
    }
    partsize =  end - start + 1; // NOT NEEDED????

    // Memory for the file reading
    part = (char *)malloc((partsize+1)*sizeof(char));

    // Read file
    MPI_File_read_at_all(fh, start, part, partsize, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);
    part[partsize] = '\0';

    // Find the real start and end of the text
    textStart = 0;
    textEnd = partsize;
    if (rank != 0) {
        while(part[textStart] != '\n')
            textStart++;
        textStart++;
    }
    if (rank != numOfProcesses-1) {
        textEnd -= overlap;
        while(part[textEnd] != '\n')
            textEnd++;
    }
    partsize = textEnd-textStart+1;
    // Copy the real text to a new array
    data = (char *)malloc((partsize+1)*sizeof(char));
    memcpy(data, &(part[textStart]), partsize);
    free(part);
    data[partsize] = '\0';

    // Count the lines
    numOfLines = 0;
    for (i=0; i<partsize; i++)
        if (data[i] == '\n')
            (numOfLines)++;

    // Split the data into lines
    lines = (char **)malloc((numOfLines)*sizeof(char *));
    lines[0] = strtok(data,"\n");
    for (i=1; i<numOfLines; i++)
        lines[i] = strtok(NULL, "\n");

    *res = lines;
    return numOfLines;
}

int checkCollision(double xyz[3]) {
    int i, inRange = 1;
    for (i=0;i<3 && inRange;i++)
        if (xyz[i] < LOW || xyz[i] > HIGH)
            inRange = 0;

  return inRange;
}

int checkInRange(char ** lines, int num,
                    const int MAX_TIME,
                    const int MAX_COLLISIONS,
                    struct timespec start) {
    int inRange = 0;
    int i, finished;
    double coords[3];

    if (MAX_COLLISIONS != -1)
        num = MAX_COLLISIONS;

    #pragma omp parallel private(i,finished,coords) shared(inRange)
    {
    finished = 0;
    #pragma omp for reduction(+:inRange)
    for (i=0; i<num ; i++) {
        char * saveptr;
        if ( finished ) i = num;
        // Split line into three doubles
        coords[0] = atof(strtok_r(lines[i]," ",&saveptr));
        coords[1] = atof(strtok_r(NULL," ",&saveptr));
        coords[2] = atof(strtok_r(NULL," ",&saveptr));

        inRange += checkCollision(coords);

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

void freeCoords(char ***coords, int no_of_lines) {
    int j=0;
    for (j=0;j<no_of_lines;j++) {
        free((*coords)[j]);
    }
    free(*coords);
}
