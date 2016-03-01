#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LOW 12
#define HIGH 30

int countlines(FILE *file);
int readFile(const char *fname, double ***coords);
int checkCollision(double xyz[3]);
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

    // Check points
    for (i=0; i<noOfLines && !finished; i++) {
        if (checkCollision(coords[i])) numOfCollisions++;
        if (MAX_COLLISIONS != -1 && numOfCollisions >= MAX_COLLISIONS)
            finished = 1;
        // POSIX STUFF FOR TIME HERE
    }

    printf("%d\n", numOfCollisions);
    freeCoords(&coords, noOfLines);
    return 0;
}

int countlines(FILE *file) {

    char c;
    int counter=0;

    c=fgetc(file);
    while(c != EOF) {
        if(c == '\n')
            counter++;
        c=fgetc(file);
    }

    rewind(file);//sends pointer at the beggining
    return counter;
}

int readFile(const char *fname, double ***coords) {
    FILE *fp;

    fp = fopen(fname,"r");//read only

    if(fp == NULL) {
        fprintf(stderr,"Error while opening file %s\n", fname);
        return 0;
    }

    int no_of_lines = countlines(fp);
    double ** temp;
    temp = (double **)malloc(no_of_lines*sizeof(double *));
    int k;
    for(k=0;k<no_of_lines;k++) {
        temp[k] = (double *)malloc(3*sizeof(double));
    }

    int i;
    for(i=0;i<no_of_lines;i++) {
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

void freeCoords(double ***coords, int no_of_lines) {
    int j=0;
    for (j=0;j<no_of_lines;j++) {
        free((*coords)[j]);
    }
    free(*coords);
}
