#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include "symnmf.h"

#define MAX_ITER 300
#define EPS 0.0001

void freeMemoryModule(double**, int*, double**, int, int);
int getNModule(PyObject*);
double **convertPyToC(PyObject*, int, int);
PyObject *convertCToPy(double**, int, int);
PyObject *calcByGoal(int, PyObject*);
void updateH(double**, double**, double**, int, int, double**);
double calcUpdateHEntry( double**, double**, int, int, int, int, double**);
double innerProduct(double**, double**, int, int, int);
double efficientCalcHHTHIJ(double**, int, int, int, int, double**);
int isConverged(double**, double**, int, int);
void matrixCopy(double**, double**, int, int);

// TODO validate the uses of sizeof(h) - will it return the length of array or just the size of pointer?

static PyObject* sym(PyObject *self, PyObject *args) {
    return calcByGoal(0, args);
}

static PyObject* ddg(PyObject *self, PyObject *args) {
    return calcByGoal(1, args);
} 

static PyObject* norm(PyObject *self, PyObject *args) {
    return calcByGoal(2, args);
}

static PyMethodDef symnmfMethods[] = {
    {"sym",
    (PyCFunction) sym,
    METH_VARARGS,
    PyDoc_STR("sym func")},
    {"ddg",
    (PyCFunction) ddg,
    METH_VARARGS,
    PyDoc_STR("ddg func")},
    {"norm",
    (PyCFunction) norm,
    METH_VARARGS,
    PyDoc_STR("norm func")},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef symnmfmodule = {
    PyModuleDef_HEAD_INIT,
    "asdf",
    NULL,
    -1,
    symnmfMethods
};

PyMODINIT_FUNC PyInit_asdf(void){
    PyObject *m;
    m = PyModule_Create(&symnmfmodule);
    if (!m){
        return NULL;
    }
    return m;
}

void freeMemoryModule(double** points, int* centroid_indices, double** centroids, int n, int k){
    /*
    int i;

    if (points != NULL){
        for (i = 0; i < n; i++){
            free(points[i]);
        }
        free(points);
    }

    free(centroid_indices);

    if (centroids != NULL){
        for (i = 0; i < k; i++){
            free(centroids[i]);
        }
        free(centroids);
    }
    */
}

int getNModule (PyObject *pypoints) {
    Py_ssize_t length = PyList_Size(pypoints);
    return PyLong_AsLong(PyLong_FromSsize_t(length));
}

double** convertPyToC (PyObject* pypoints, int n, int d){
    double **points;
    int i,j;
    double coordinate;
    PyObject *point;
    points = malloc(n * sizeof(double*));

    for (i = 0; i < n; i++){ 
        points[i] = malloc(d * sizeof(double));
        if (points[i] == NULL){
            freeMemoryModule(points, NULL, NULL, n, 0);
            return NULL;
        }
    }

    for (i = 0; i < n; i++){ //convert py to c points
        point = PyList_GetItem(pypoints, i);
        for (j = 0; j < d; j++){
            coordinate = PyFloat_AsDouble(PyList_GetItem(point, j));
            points[i][j] = coordinate;
        }
    }
    return points;
}

PyObject *convertCToPy(double** symMat, int n, int d) {
    PyObject* pysym, *pysym_row, *value;
    int i, j;

    if (symMat == NULL){
        return NULL;
    }

    pysym = PyList_New(n);
    if (pysym == NULL) {
        return NULL;
    }

    for (i = 0; i < n; i++) { // create pysym matrix
        pysym_row = PyList_New(n);
        if (pysym_row == NULL) {
            return NULL;
        }

        for (j = 0; j < n; j++) {
            value = PyFloat_FromDouble(symMat[i][j]);
            if (value == NULL) {
                return NULL;
            }
            PyList_SET_ITEM(pysym_row, j, value);
        }

        PyList_SET_ITEM(pysym, i, pysym_row);
    }
    return pysym;
}

PyObject *calcByGoal(int func, PyObject *args) {
    /* cases are:
    0 - sym
    1 - ddg
    2 - norm */

    int n, d;
    double** points;
    double** targetMat;
    PyObject *pypoints, *pysym;
    if(!PyArg_ParseTuple(args, "O", &pypoints)){
        return NULL;
    }
    if (PyObject_Length(pypoints) < 0){
        return NULL;
    }

    n = getNModule(pypoints);
    d = getNModule(PyList_GetItem(pypoints, 0));

    points = convertPyToC(pypoints, n, d);
    switch (func) {
        case 0:
            targetMat = csym(points, n, d);
            break;
        case 1:
            targetMat = cddg(points, n, d);
            break;
        case 2:
            targetMat = cnorm(points, n, d);
            break;
        default:
            printf("INTERNAL ERROR- INVALID FUNC #"); //TODO remove or change before submit
            break;
    }

    pysym = convertCToPy(targetMat, n, d);

    freeMemoryModule(targetMat, NULL, NULL, n, 0);
    freeMemoryModule(points, NULL, NULL, n, 0);
    return pysym;
}

static PyObject* symnmf_func(PyObject *self, PyObject *args) {
    // in python args there is h and w
    int n, k, i;
    double** h;
    double** w;
    double** h_prior; // a temp variable for the for loop, holding the last h calculated
    PyObject *pyh, *pyw, *pyret; // the matrices received
    
    if(!PyArg_ParseTuple(args, "OOii", &pyh, &pyw, &n, &k)){
        return NULL;
    }
    if (PyObject_Length(pyh) < 0 || PyObject_Length(pyw) < 0){
        return NULL;
    }

    h = convertPyToC(pyh, n, k);
    w = convertPyToC(pyw, n, n);

    h_prior = initialize2DimArray(n,k);
    if (h_prior == NULL) {
        freeMemoryModule(NULL, NULL, NULL, 0, 0);
        return NULL;
    }

    double **resRow = initialize2DimArray(1, n);
    if (resRow == NULL) {
        freeMemoryModule(NULL, NULL, NULL, 0, 0);
        return NULL;
    }

    for (i = 0 ; i < MAX_ITER ; i++) {
        matrixCopy(h, h_prior, n, k);
        updateH(h, h_prior, w, n, k, resRow);
        if (isConverged(h, h_prior, n, k)) { 
            break;
        }
    }

    // h is optimized, convert to py
    // SOMETHING HERE IS NOT WORKING
    pyret = convertCToPy(h, n, k);
    return pyret;
}

void updateH(double** h, double** h_prior, double** w, int n, int k, double** resRow) {
    // receives identical matrices but h changes inplace
    int i, j;

    for (i = 0 ; i < n ; i++) {
        for (j = 0 ; j < k ; j++) {
            h[i][j] = calcUpdateHEntry(h_prior, w, i, j, n, k, resRow);

        }
    }
}

double calcUpdateHEntry( double** h_prior, double** w, int i, int j, int n, int k, double** resRow) {
    double beta = 0.5;
    double hij = h_prior[i][j];
    double whij = innerProduct(w, h_prior, i, j, sizeof(w));
    double hhthij = efficientCalcHHTHIJ(h_prior, i, j, n, k, resRow);

    return (hij * (1 - beta + (beta * (whij / hhthij))));
}

double innerProduct(double** a, double** b, int i, int j, int commonDim) {
    double sum = 0.0;
    int k;
    for (k = 0 ; k < commonDim ; k++) {
        sum += a[i][k] * b[k][j];
    }
    return sum;
}

double efficientCalcHHTHIJ(double** h, int i, int j, int n, int k, double** resRow) {
    double sum = 0;
    int m, t;

    for (m = 0 ; m < n ; m++) {
        for (t = 0 ; t < k ; t++) { // run over common dimension
            sum += h[m][t] * h[t][m]; // inner product of transpose
        }
        resRow[1][m] = sum;
        sum = 0;
    }
    // this is equivalent to (h * h^t)*h [i][j]
    return innerProduct(resRow, h, 1, j, n);
}

int isConverged(double** h, double** h_prior, int n, int k) {
    // 1 on convergance, 0 else
    int i,j;
    double frobBeforeSqrt = 0;
    for (i = 0 ; i < n ; i++) {
        for (j = 0 ; j < k ; j++) {
            frobBeforeSqrt += pow(fabs(h[i][j] - h_prior[i][j]), 2); // according to wiki
        }
    }
    if (sqrt(frobBeforeSqrt) < EPS) { 
        return 1;
    }
    return 0;
}

void matrixCopy(double** src, double** tgt, int n, int k) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < k; ++j) {
            tgt[i][j] = src[i][j];
        }
    }
}

