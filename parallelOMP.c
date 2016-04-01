
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>
#define LOW 12
#define HIGH 30

int countlines(FILE *file);
int readFile(char *fname, char *** res, int rank, int numOfProcesses,
                    MPI_Comm custom_comm,int maxNumOfLines);
int checkCollision(double xyz[3]);
int checkInRange(char ** lines, int num,
                    const int MAX_TIME,
                    const int MAX_COLLISIONS,
                    struct timespec start,
                    int *numOfChecked);
int checkTime(struct timespec start,const int MAX_TIME);
void printResults(double totalTime, int inRange, int numOfCollisions);
double calcTime(struct timespec start,struct timespec end);
void freeCoords(char ***coords, int no_of_lines);
void limitProcesses(int* numOfProcesses, int limit, MPI_Comm* comm);

int main(int argc, char *argv[]) {
    // Check program arguments
    if (argc != 6) {
        fprintf(stderr, "Incorrect number of parameters\n");
        return 1;
    }
    // 1st argument: Number of collisions
    const int MAX_COLLISIONS = atoi(argv[1]);
    // 2nd argument: Maximum time the program should complete its calculations
    const int MAX_TIME = atoi(argv[2]);
    // 3rd argument: Name of file
    char* FILENAME = argv[3];

    if (FILENAME == NULL) {
        fprintf(stderr, "No filename given\n");
        return 1;
    }
    // Number of threads for OpenMP
    const int MAX_THREADS = atoi(argv[4]);

    if(MAX_THREADS > 0)
    	omp_set_num_threads(MAX_THREADS);
    // Number of processes for MPI
    const int MAX_PROCESSES = atoi(argv[5]);

    struct timespec start, end;
    char **lines = NULL;
    int inRange;
    int rank, numOfProcesses;

    MPI_Init(&argc, &argv);
    // Number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &numOfProcesses);
    // ID of each process executing this command
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI communicator of all processes
    MPI_Comm custom_comm = MPI_COMM_WORLD;

    // Limit the number of processes
    if (MAX_PROCESSES != -1 && MAX_PROCESSES < numOfProcesses)
        limitProcesses(&numOfProcesses, MAX_PROCESSES, &custom_comm);

    // Start timer
    clock_gettime(CLOCK_MONOTONIC,  &start);
    // Read file and write content in lines
    int numOfLines = readFile(FILENAME, &lines, rank, numOfProcesses, custom_comm,MAX_COLLISIONS);

    if (!numOfLines) {
        fprintf(stderr, "No data given\n");
        return 1;
    }

    // Number of coordinates checked
    int numOfChecked;
    // Find the number of coordinates that are in range
    inRange = checkInRange(lines, numOfLines, MAX_TIME, MAX_COLLISIONS, start,&numOfChecked);

    // Stop timer
    clock_gettime(CLOCK_MONOTONIC,  &end);
    // Calculate time elapsed
    double secs = calcTime(start, end);

    free(lines);

    double dataForEachProccess[3];
    double *rootBuffer = NULL;
    dataForEachProccess[0] = secs;
    dataForEachProccess[1] = inRange;
    dataForEachProccess[2] = numOfChecked;

    if (rank == 0) {
	    rootBuffer = (double*)malloc( numOfProcesses * sizeof(double) * 3 );
	    if (rootBuffer == NULL){
		    printf("Out of memory!\n");
	    }
    }
    // Gather data from each process
    MPI_Gather(dataForEachProccess,3,MPI_DOUBLE,rootBuffer,3,MPI_DOUBLE,0,custom_comm);

    // Split root buffer
    if (rank == 0) {
	     int i;
	     double whatWeWantToKeep[3];
	     whatWeWantToKeep[0] = whatWeWantToKeep[1] = whatWeWantToKeep[2] = 0;
	     for ( i = 0 ; i < numOfProcesses *3 ; i = i+3 ) {

    	    if (rootBuffer[i] > whatWeWantToKeep[0])
    	        whatWeWantToKeep[0] = rootBuffer[i];

    	    whatWeWantToKeep[1] += rootBuffer[i+1];
            whatWeWantToKeep[2] += rootBuffer[i+2];
	     }
	     printResults(whatWeWantToKeep[0],whatWeWantToKeep[1],whatWeWantToKeep[2]);
	}
    MPI_Finalize();
    return 0;
}


/**
* Read the content of the file
*/
int readFile(char *fname, char *** res, int rank, int numOfProcesses,
                    MPI_Comm custom_comm, int maxNumOfLines) {

    MPI_File fh;
    MPI_Offset filesize, partsize, start, end;
    char *part, *data;
    char **lines;
    // Size of Overlap: 3 * (6decimals + 4integers + point) + "\0" + "\n" + 2 spaces
    int overlap = 40 * sizeof(char);
    int textStart, textEnd, numOfLines, i;

    // Open file
    MPI_File_open(custom_comm, fname, MPI_MODE_RDONLY, MPI_INFO_NULL,&fh);
    // Get size of file in bytes
    MPI_File_get_size(fh, &filesize);
    // The number of bytes for each process
    partsize = filesize/numOfProcesses;
    // The start of the part to be read from the process
    start = rank*partsize;
    // The end of the part to be read from the process
    end = start+partsize-1;

    // Add overlap to every process except for the last
    if (rank != numOfProcesses-1) {
        end += overlap;
    } else {
        end = filesize;
    }
    partsize =  end - start + 1;

    // Memory for the file reading
    part = (char *)malloc((partsize+1)*sizeof(char));
    if (part == NULL){
    	printf("Out of memory!\n");
    }

    // Read file
    MPI_File_read_at_all(fh, start, part, partsize, MPI_CHAR, MPI_STATUS_IGNORE);
    // Close file
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
    if (data == NULL){
    	printf("Out of memory!\n");
    }

    memcpy(data, &(part[textStart]), partsize);
    free(part);
    data[partsize] = '\0';

    // Count the lines
    numOfLines = 0;
    int numOfLimit = 0;
    maxNumOfLines = maxNumOfLines/numOfProcesses;

    // Split for each process parts of the file to multiple threads
    for (i=0; i<partsize; i++){
        if (data[i] == '\n'){
            (numOfLines)++;
            (numOfLimit)++;
        }
        if(i == maxNumOfLines-1){
            i = partsize;
        }
    }

    // Split the data into lines
    lines = (char **)malloc((numOfLines)*sizeof(char *));
   	if (lines == NULL){
    	printf("Out of memory!\n");
    }
    lines[0] = strtok(data,"\n");

	  for (i=1; i<numOfLines; i++)
        lines[i] = strtok(NULL, "\n");

    *res = lines;

    return numOfLines;
}

/**
 * Checks if the given coordinates are within the range
 */
int checkCollision(double xyz[3]) {
    int i, inRange = 1;
    for (i=0;i<3 && inRange;i++)
        if (xyz[i] < LOW || xyz[i] > HIGH)
            inRange = 0;

    return inRange;
}

/**
 * Returns the number of coordinates that are within the range
 */
int checkInRange(char ** lines, int num,
                    const int MAX_TIME,
                    const int MAX_COLLISIONS,
                    struct timespec start,
                    int *numOfChecked) {
    int inRange = 0;
    int temp = 0;
    int i;
    double coords[3];

    if (MAX_COLLISIONS != -1)
        num = MAX_COLLISIONS;

    // Split for each process the calculations to multiple threads
    #pragma omp parallel private(i,coords) shared(inRange,temp)
    {

    	#pragma omp for reduction(+:inRange) reduction(+:temp)
    	for (i=0; i<num ; i++) {
        	char * saveptr;

        	// Split line into three doubles
        	coords[0] = atof(strtok_r(lines[i]," ",&saveptr));
        	coords[1] = atof(strtok_r(NULL," ",&saveptr));
        	coords[2] = atof(strtok_r(NULL," ",&saveptr));

        	inRange += checkCollision(coords);

        	temp++;

        	if(MAX_TIME > -1)
            	if (checkTime(start, MAX_TIME))
            		i = num; // Exit program if time exceeded
    	}
    }
    *numOfChecked = temp;
    return inRange;
}

/**
* Print the results of the program
*/
void printResults(double totalTime, int inRange, int numOfCollisions) {
    printf("Number of coordinates: %d\n", numOfCollisions);
    printf("Number of coordinates in range: %d\n", inRange);
    printf("In range ratio: %f\n", numOfCollisions/(double)inRange);
    printf("Processing ratio: %f coordinates/sec \n", numOfCollisions/totalTime);
    printf("Total time: %f secs \n", totalTime);
}

/**
 * Calculates the number of seconds elapsed
 */
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

/**
* Check if maximum time exceeded
*/
int checkTime(struct timespec start, const int MAX_TIME){
		struct timespec current;
    // Get current time
	clock_gettime(CLOCK_MONOTONIC,&current);

    if (calcTime(start,current) >= MAX_TIME){
        printf("Maximum time exceeded! \n");
        printf("%f \n", calcTime(start,current));
        return 1;
    }

    return 0;
}


/**
* Limit the processes to the given number
*/
void limitProcesses(int* numOfProcesses, int limit, MPI_Comm* comm) {
    MPI_Comm custom_comm = *comm;
    // Get processes in MPI_COMM_WORLD
    MPI_Group firstGroup;
    MPI_Comm_group(custom_comm, &firstGroup);

    // Remove uneeded ranks from the group
    MPI_Group newGroup;
    int ranges[3] = { limit, (*numOfProcesses)-1, 1 };
    MPI_Group_range_excl(firstGroup, 1, &ranges, &newGroup);

    // Create a new communicator
    MPI_Comm_create(MPI_COMM_WORLD, newGroup, &custom_comm);
    *numOfProcesses = limit;
    *comm = custom_comm;

    // If the current rank needs to be excluded
    if (custom_comm == MPI_COMM_NULL) {
        MPI_Finalize();
        exit(0);
    }
}
