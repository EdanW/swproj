#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include "symnmf.h"

void freeMemoryModule(double**, int*, double**, int, int);
int getNModule(PyObject*);

static PyObject* sym(PyObject *self, PyObject *args){
    int n, d, i, j;
    double** points;
    double** symMat;
    PyObject *pypoints;
    PyObject *point;
    PyObject *pysym;
    double coordinate;
    if(!PyArg_ParseTuple(args, "O", &pypoints)){
        return NULL;
    }

    if (PyObject_Length(pypoints) < 0){
        return NULL;
    }

    n = getNModule(pypoints);
    d = getNModule(PyList_GetItem(pypoints, 0));

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

    symMat = csym(points, n, d);
    
    if (symMat == NULL){
        freeMemoryModule(points, NULL, NULL, n, 0);
        return NULL;
    }

    pysym = PyList_New(n);
    if (pysym == NULL) {
        freeMemoryModule(points, NULL, NULL, n, 0);
        return NULL;
    }

    for (i = 0; i < n; i++) { // create pysym matrix
        PyObject *pysym_row = PyList_New(n);
        if (pysym_row == NULL) {
            freeMemoryModule(points, NULL, NULL, n, 0);
            return NULL;
        }

        for (j = 0; j < n; j++) {
            PyObject *value = PyFloat_FromDouble(symMat[i][j]);
            if (value == NULL) {
                freeMemoryModule(points, NULL, NULL, n, 0);
                return NULL;
            }
            PyList_SET_ITEM(pysym_row, j, value);
        }

        PyList_SET_ITEM(pysym, i, pysym_row);
    }

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