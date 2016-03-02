#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void freeCords(double ***coords, int coordinate_index);

int main(int argc, char* argv[])
{
    char filename[40]; // The first argument from command line that indicates the name of the output file
    long int coordinate_index; // The second argument from command line that indicates the number of coordinates

    if (argc == 3)
    {
        strcpy(filename,argv[1]);
        strcat(filename,".out");

        coordinate_index = atoi(argv[2]);
    }
    else
    {
        printf("Expected 2 arguments \n");
        exit(1);
    }


    FILE *file = fopen(filename,"w+");

    int i;

    double **cords;
    cords = (double **)malloc(coordinate_index*sizeof(double *));
    int k;
    for (k=0;k<coordinate_index;k++){
        cords[k] = (double *)malloc(3*sizeof(double));
    }

    int utime;
    long int ltime;

    ltime = time(NULL);
    utime = (unsigned int) ltime / 2;

    srand(utime);

    if (file == NULL)
    {
        printf("Error creating file \n");
        exit(1);
    }

    for (i=0;i<coordinate_index;i++)
    {
        cords[i][0]=(double)34*rand()/(RAND_MAX-1);
        cords[i][1]=(double)34*rand()/(RAND_MAX-1);
        cords[i][2]=(double)34*rand()/(RAND_MAX-1);


        fprintf(file,"%f %f %f\n",cords[i][0],cords[i][1],cords[i][2]);
    }

    freeCords(&cords,coordinate_index);
    fclose(file);
    return 0;
}


void freeCords(double ***coords, int coordinate_index) {
    int j=0;
    for (j=0;j<coordinate_index;j++) {
        free((*coords)[j]);
    }
    free(*coords);
}
