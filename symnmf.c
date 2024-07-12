#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "symnmf.h"

#define ERROR_MSG "An Error Has Occurred"

double dist(double *, double *, int);
void freeMemory(double **, int);
int getDimension(FILE *file);
int getN(FILE *file);
void parsePoints(double **, int, int, FILE *);
double **initialize2DimArray(int, int);
void printMatrix(double **, int, int);
double **csym(double **, int, int);
double **cddg(double **, int, int);
double **calcDdg(double **, int);
double sumOfArray(double *, int);
double **cnorm(double **, int, int);
void multiplyMatrices(double **, double **, double **, int, int, int);
void throwOfficialError();

int main(int argc, char *argv[]) {
    char *goal;
    char *filePath;
    double **points; /* N points of d dimension */
    int n, d;
    double **mat;
    FILE *file;

    goal = argv[1];
    filePath = argv[2];
    file = fopen(filePath, "r");
    d = argc; /* This line gets rid of 'unused parameter' Error */
    d = getDimension(file);
    n = getN(file);
    points = initialize2DimArray(n, d);
    if (!points) {
        freeMemory(points, n);
        fclose(file);
        throwOfficialError();
        return 0;
    }

    parsePoints(points, n, d, file);

    if (strcmp(goal, "sym") == 0)
    {
        mat = csym(points, n, d);
    }
    else if (strcmp(goal, "ddg") == 0)
    {
        mat = cddg(points, n, d);
    }
    else if (strcmp(goal, "norm") == 0)
    {
        mat = cnorm(points, n, d);
    }

    freeMemory(points, n);
    fclose(file);

    if (mat) {
        printMatrix(mat, n, n);
    } else {
        throwOfficialError();
    }
    return 0;
}

double **csym(double **points, int n, int d) {
    double **sym = initialize2DimArray(n, n);
    int i, j;
    if (!sym)
    {
        freeMemory(points, n);
        return NULL;
    }
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            if (i == j)
            {
                sym[i][i] = 0;
            }
            else
            {
                double squaredDistOfVecs = pow(dist(points[i], points[j], d), 2);
                sym[i][j] = exp(-0.5 * squaredDistOfVecs);
            }
        }
    }
    return sym;
}

double **cddg(double **points, int n, int d) {
    double **mySym, **ddgRes;
    mySym = csym(points, n, d);
    if (!mySym) {
        return NULL;
    }
    ddgRes = calcDdg(mySym, n);
    if (!ddgRes) {
        return NULL;
    }
    freeMemory(mySym, n);
    return ddgRes;
}

double **calcDdg(double **sym, int n) {
    double **ddg;
    int i;

    ddg = initialize2DimArray(n, n);
    if (!ddg) {
        return NULL;
    }
    for (i = 0; i < n; i++)
    {
        ddg[i][i] = sumOfArray(sym[i], n);
    }
    return ddg;
}

double sumOfArray(double *arr, int n) {
    int i;
    double sum = 0;
    for (i = 0; i < n ; i ++) {
        sum += arr[i];
    }
    return sum;
}

double **cnorm(double **points, int n, int d) {
    int i;
    double **normRes = initialize2DimArray(n, n);
    double **symMat = csym(points, n, d);
    double **ddgMat = calcDdg(symMat, n);
    double **normTemp;

    /* calculate ddg^-1/2 */
    for (i = 0 ; i < n ; i++) {
        ddgMat[i][i] = pow(ddgMat[i][i], -0.5);
    }
    /* matrix mult */
    normTemp = initialize2DimArray(n, n);
    if (!normTemp) {
        freeMemory(symMat, n);
        freeMemory(ddgMat, n);
        return NULL;
    }
    multiplyMatrices(symMat, ddgMat, normTemp, n, n, n);
    multiplyMatrices(ddgMat, normTemp, normRes, n, n, n);
    freeMemory(normTemp, n);
    freeMemory(symMat, n);
    freeMemory(ddgMat, n);

    return normRes;
}

void multiplyMatrices(double **A, double **B, double **tgt, int rowsA, int colsA, int colsB) {
    int i, j, k;
    for (i = 0; i < rowsA; i++) {
        for (j = 0; j < colsB; j++) {
            tgt[i][j] = 0;
            for (k = 0; k < colsA; k++) {
                tgt[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int getDimension(FILE *file) {
    int commas = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == ',') {
            commas++;
        }
        if (ch == '\n') {
            break; /* Stop counting after the first line */
        }
    }
    rewind(file);
    return commas + 1;
}

int getN(FILE *file) {
    int lines = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }
    rewind(file);
    return lines;
}

double dist(double *p1, double *p2, int d) {
    double diff, sum = 0;
    int i;

    for (i = 0; i < d; i++)
    {
        diff = p1[i] - p2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

void freeMemory(double **points, int n) {
    int i;
    if (!points) {
        return;
    }
    for (i = 0; i < n; i++) {
        free(points[i]);
    }
    free(points);
}

double **initialize2DimArray(int n, int d) {
    int i;
    double **arr = calloc(sizeof(double *), n);
    if (!arr) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        arr[i] = calloc(sizeof(double), d);
        if (!arr[i]) {
            freeMemory(arr, n);
            return NULL;
        }
    }
    return arr;
}

void parsePoints(double **points, int n, int d, FILE *file) {
    int i, j;
    double curr;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < d; j++)
        {
            fscanf(file, "%lf", &curr);
            if (j != d - 1)
            {
                fscanf(file, ",");
            }
            points[i][j] = curr;
        }
    }
}

void printMatrix(double **mat, int n, int d) {
    int i, j;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < d - 1; j++)
        {
            printf("%.4f,", mat[i][j]);
        }
        printf("%.4f\n", mat[i][d - 1]);
    }
}

void throwOfficialError() {
    printf(ERROR_MSG);
}