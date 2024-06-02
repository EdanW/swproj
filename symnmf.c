#define PY_SSIZE_T_CLEAN
//#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "symnmf.h"
// ^ need help with locally enabling these
#define     ITER_DEFAULT    200
#define     epsilon         0.001
#define     ERROR_MSG       "An Error Has Occurred\n"

// todo rewrite prototypes

double   dist(double*, double*, int);
void     freeMemory(double**, int);
int      getDimension(FILE *file);
int      getN(FILE *file);
void     parsePoints(double**, int, int, FILE *); //todo 
double** initialize2DimArray(int, int);
void     printMatrix(double**, int, int);

int main(int argc, char *argv[])
{
    // variables
    char *goal;
    char *filePath;
    double **points; // N points of d dimension
    int n, d, i;
    double **mat;

    goal = argv[1];
    filePath = argv[2];
    FILE *file = fopen(filePath, "r");
    d = getDimension(file);
    n = getN(file);
    
    printf("%d  , %d\n", n, d);
    points = initialize2DimArray(n,d);

    parsePoints(points, n ,d, file); //TODO stopped here, doesnt print after this line!

    printMatrix(points, n, d);

    freeMemory(points, n);
    fclose(file);

    /*
    // variables
    char *goal;
    char *filePath;
    double **points; // N points of d dimension
    int n, d, i;
    double **mat;
    // assumption 2 no need to validate input

    goal = argv[1];
    filePath = argv[2];
    FILE *file = fopen(filePath, "r");
    d = getDimension(file);
    n = getN(file);
    points = initialize2DimArray(n,d);

    parsePoints(points, n, d, file);

    if (strcmp(goal, "sym") == 0) {
        mat = sym(points, n, d);
    } else if (strcmp(goal, "ddg") == 0) {
        mat = ddg(points, n, d);
    } else if (strcmp(goal, "norm") == 0) {
        mat = norm(points, n, d);
    }

    // todo print mat

    fclose(file);
    return 0;
    */
}

double **sym(double** points, int n, int d) {
    // mat = initialize2DimArray(n, n);
    // assert its not null, if it is call freeMemory
    printf("Goal is sym\n");
    return NULL;
}

int getDimension(FILE *file) {
    int commas = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == ',') {
            commas++;
        }
        if (ch == '\n') {
            break; // Stop counting after the first line
        }
    }
    rewind(file);
    return commas + 1;
    // TODO check it works
}

int getN(FILE *file) {
    int lines = 0;
    int ch;
    int last_char_was_newline = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }
    rewind (file);
    return lines;
    // TODO check if input ends in newline or just ends
}

double dist(double* p1, double* p2, int d){
    double diff, sum = 0;
    int i;

    for (i = 0; i < d; i++){
        diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

void freeMemory(double **points, int n){
    if (!points) {return;}
    int i;
    for (i = 0 ; i < n ; i ++){
        free(points[i]);
    }
    free(points);
}

double **initialize2DimArray(int n, int d) {
    int i;
    double **arr = malloc(sizeof(double*)*n);
    if (!arr){
        return NULL;
    }
    for (i = 0 ; i < n ; i++) {
        arr[i] = malloc(sizeof(double)*d);
        if (arr[i]) {
            freeMemory(arr,n);
            return NULL;
        }
    }
    return arr;
} // todo check works & valgrind

void parsePoints(double **points, int n, int d, FILE * file) {
    int i, j;
    double curr;
    for (i = 0 ; i < n ; i ++){
        for (j = 0 ; j < d ; j++){
            fscanf(file, "%lf", &curr);
            points[i][j] = curr;
        }
    }
}

void printMatrix(double **mat, int n, int d) {
    int i, j;
    for (i = 0 ; i < n ; i++){
        for (j = 0 ; j < d-1 ; j++){
            printf("%.4f,", mat[i][j]);
        }
        printf("%.4f\n", mat[i][d-1]);
    }
}
