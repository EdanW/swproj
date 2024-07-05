#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include "symnmf.h"

#define MAX_ITER = 300
#define EPS = 0.0001 //1e-4

void freeMemoryModule(double**, int*, double**, int, int);
int getNModule(PyObject*);
double **convertPyToC(PyObject*, int, int);
PyObject *convertCToPy(double**, int, int);
PyObject *calcByGoal(int, PyObject*);


static PyObject* sym(PyObject *self, PyObject *args) {
    return calcByGoal(0, args);
}

static PyObject* ddg(PyObject *self, PyObject *args) {
    return calcByGoal(1, args);
} 

static PyObject* norm(PyObject *self, PyObject *args) {
    return calcByGoal(2, args);
}

static PyObject* symnmf(PyObject *self, PyObject *args) {
    // THIS IS JUST A SKELETON FOR NOW AND NEEDS TO BE TESTED
    // in args there is h and w
    int n, k, i, j;
    double beta = 0.5;
    double** h;
    double** w;
    double** h_prior; // a temp variable for the for loop, holding the last h calculated
    PyObject *pyh, *pyw; // the matrices received
    
    if(!PyArg_ParseTuple(args, "OO", &pyh, &pyw)){
        return NULL;
    }
    if (PyObject_Length(pyh) < 0 || PyObject_Length(pyw) < 0){
        return NULL;
    }

    // get n of pyh and pyw
    // get k of pyh

    h = convertPyToC(pyh, n, k);
    w = convertPyToC(pyw, n, n);
    
    for (i = 0 ; i < MAX_ITER ; i++) {
        h_prior = h; // make sure im not off by one
        // h = updateH (h_prior) NEED TO IMPLEMENT THIS
        // if isConverged(h, h_prior)
            // break and return H?
    }

    return pyh;
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
            printf("INTERNAL ERROR- INVALID FUNC #");
            break;
    }

    pysym = convertCToPy(targetMat, n, d);

    freeMemoryModule(targetMat, NULL, NULL, n, 0);
    freeMemoryModule(points, NULL, NULL, n, 0);
    return pysym;
}