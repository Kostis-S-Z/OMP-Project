#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char* argv[])
{
    char filename[40]; // The first argument from command line that indicates the name of the output file
    int coordinate_index; // The second argument from command line that indicates the number of coordinates

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

    float cords[coordinate_index][3];

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

    for (i = 0; i < coordinate_index; i++)
    {
        cords[i][0]=(float)34*rand()/(RAND_MAX-1);
        cords[i][1]=(float)34*rand()/(RAND_MAX-1);
        cords[i][2]=(float)34*rand()/(RAND_MAX-1);


        fprintf(file,"%f,%f,%f\n",cords[i][0],cords[i][1],cords[i][2]);
    }

    fclose(file);
    return 0;
}
