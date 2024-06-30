#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include "symnmf.h"

void freeMemoryModule(double**, int*, double**, int, int);
int getNModule(PyObject*);
double **convertPyToC(PyObject*, int, int);
PyObject *convertCToPy(double**, int, int);

static PyObject* sym(PyObject *self, PyObject *args){
    int n, d;
    double** points;
    double** symMat;
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
    symMat = csym(points, n, d);
    pysym = convertCToPy(symMat, n, d);

    freeMemoryModule(symMat, NULL, NULL, n, 0);
    freeMemoryModule(points, NULL, NULL, n, 0);
    return pysym;
}

static PyMethodDef symnmfMethods[] = {
    {"sym",
    (PyCFunction) sym,
    METH_VARARGS,
    PyDoc_STR("sym func")},
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