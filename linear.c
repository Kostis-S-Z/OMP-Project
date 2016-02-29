/*
 * Date: February 29 2016
 * Author: Poiitis Marinos
 */

#include <stdio.h>
#include <stdlib.h>


int countlines(FILE *file);

int main(int argc, char *argv[]) {

    FILE *fp;

    fp = fopen(argv[3],"r");//read only

    int no_of_lines = countlines(fp);
    rewind(fp);//sends pointer at the beggining
    double **coords;

    coords = (double **)malloc(no_of_lines*sizeof(double *));
    int k;
    for(k=0;k<no_of_lines;k++){
        coords[k] = (double *)malloc(3*sizeof(double));
    }


    if(fp == NULL) {
        fprintf(stderr,"Error while opening file %s", argv[3]);
        return 0;
    }


    int i;

    for(i=0;i<no_of_lines;i++){
        fscanf(fp,"%lf",&coords[i][0]);
        fscanf(fp,"%lf",&coords[i][1]);
        fscanf(fp,"%lf",&coords[i][2]);

    }

    int j;
    for(j=0;j<no_of_lines;j++){
        printf("%f \n" , coords[j][0]);
        printf("%f \n" , coords[j][1]);
        printf("%f \n" , coords[j][2]);
    }

    fclose(fp);

    return 0;
}

int countlines(FILE *file){

    char c;
    int counter=0;

    c=fgetc(file);
    while(c != EOF){
        if(c == '\n')
            counter++;
        c=fgetc(file);
    }

    return counter;
}

