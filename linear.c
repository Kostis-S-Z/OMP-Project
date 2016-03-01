#include <stdio.h>
#include <stdlib.h>


int countlines(FILE *file);
int readFile(char *fname, double ***coords);
void freeCoords(double ***coords, int no_of_lines);

int main(int argc, char *argv[]) {
    double **coords = NULL;
    int no_of_lines = readFile(argv[3], &coords);
    
    int j;
    for(j=0;j<no_of_lines;j++) {
        printf("%f \n" , coords[j][0]);
        printf("%f \n" , coords[j][1]);
        printf("%f \n" , coords[j][2]);
    }

    freeCoords(&coords, no_of_lines);
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

int readFile(char *fname, double ***coords) {
    FILE *fp;

    fp = fopen(fname,"r");//read only
    
    if(fp == NULL) {
        fprintf(stderr,"Error while opening file %s", fname);
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

void freeCoords(double ***coords, int no_of_lines) {
    int j=0;
    for (j=0;j<no_of_lines;j++) {
        free((*coords)[j]);
    }
    free(*coords);   
}
