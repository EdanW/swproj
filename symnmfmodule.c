#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <math.h>
#include "symnmf.h"

#define MAX_ITER 300
#define EPS 0.0001

void      freeMemoryModule(double**, int, double**, int, double**, int, double**, int);
int       getNModule(PyObject*);
double**  convertPyToC(PyObject*, int, int);
PyObject* convertCToPy(double**, int, int);
PyObject* calcByGoal(int, PyObject*);
void      updateH(double**, double**, double**, int, int, double**);
double    calcUpdateHEntry( double**, double**, int, int, int, int, double**);
double    innerProduct(double**, double**, int, int, int);
double    efficientCalcHHTHIJ(double**, int, int, int, int, double**);
int       isConverged(double**, double**, int, int);
void      matrixCopy(double**, double**, int, int);

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
    /* python args are: h, w, n, k */
    int n, k, i;
    double** h;
    double** w;
    double** h_prior; /* a temp variable for the for loop, holding the last h calculated */
    double** resRow; /* a temp variable for efficiently calculate matrices multiplications */
    PyObject *pyh, *pyw, *pyres; /* the matrices received */
    
    if(!PyArg_ParseTuple(args, "OOii", &pyh, &pyw, &n, &k)){
        return NULL;
    }
    if (PyObject_Length(pyh) < 0 || PyObject_Length(pyw) < 0){
        return NULL;
    }

    h = convertPyToC(pyh, n, k);
    w = convertPyToC(pyw, n, n);
    h_prior = initialize2DimArray(n,k);
    resRow = initialize2DimArray(1, n);

    if (h == NULL || w == NULL || h_prior == NULL || resRow == NULL){
        freeMemoryModule(h, n, w, n, h_prior, n, resRow, 1);
        return NULL;
    }

    for (i = 0 ; i < MAX_ITER ; i++) {
    
        matrixCopy(h, h_prior, n, k);
        updateH(h, h_prior, w, n, k, resRow);
        if (isConverged(h, h_prior, n, k)) { 
            break;
        }
    }

    /* h is optimized, convert to py */

    pyres = convertCToPy(h, n, k); /* if it's null, either way we will free memory and return null */
    freeMemoryModule(h, n, w, n, h_prior, n, resRow, 1);
    
    return pyres;
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
    {"symnmf",
    (PyCFunction) symnmf,
    METH_VARARGS,
    PyDoc_STR("symnmf func")},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef symnmfmodule = {
    PyModuleDef_HEAD_INIT,
    "symnmf",
    NULL,
    -1,
    symnmfMethods
};

PyMODINIT_FUNC PyInit_symnmf(void){
    PyObject *m;
    m = PyModule_Create(&symnmfmodule);
    if (!m){
        return NULL;
    }
    return m;
}

void freeMemoryModule(double** mat1, int dim1, double** mat2, int dim2, double** mat3, int dim3, double** mat4, int dim4){
    int i;

    if (mat1){
        for (i = 0; i < dim1; i++){
            free(mat1[i]);
        }
        free(mat1);
    }

    if (mat2){
        for (i = 0; i < dim2; i++){
            free(mat2[i]);
        }
        free(mat2);
    }

    if (mat3){
        for (i = 0; i < dim3; i++){
            free(mat3[i]);
        }
        free(mat3);
    }

    if (mat4){
        for (i = 0; i < dim4; i++){
            free(mat4[i]);
        }
        free(mat4);
    }
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

    points = initialize2DimArray(n, d);

    if (!points){
        return NULL;
    }
    
    for (i = 0; i < n; i++){ /* conversion of python type points to c type */
        point = PyList_GetItem(pypoints, i);
        for (j = 0; j < d; j++){
            coordinate = PyFloat_AsDouble(PyList_GetItem(point, j));
            points[i][j] = coordinate;
        }
    }
    return points;
}

PyObject *convertCToPy(double** cMat, int n, int d) {
    PyObject* pyMat;
    PyObject* pyMat_row;
    PyObject* value;
    int i, j;

    if (cMat == NULL){
        return NULL;
    }

    pyMat = PyList_New(n);
    if (pyMat == NULL) {
        return NULL;
    }

    for (i = 0; i < n; i++) {
        pyMat_row = PyList_New(d);
        if (pyMat_row == NULL) {
            return NULL;
        }

        for (j = 0; j < d; j++) {
            value = PyFloat_FromDouble(cMat[i][j]);
            if (value == NULL) {
                return NULL;
            }
            PyList_SET_ITEM(pyMat_row, j, value);
        }

        PyList_SET_ITEM(pyMat, i, pyMat_row);
    }
    return pyMat;
}

PyObject *calcByGoal(int func, PyObject *args) {
    /* cases are:
    0 - sym
    1 - ddg
    2 - norm */

    int n, d;
    double** points;
    double** targetMat = NULL;
    PyObject *pypoints, *pyres;
    if(!PyArg_ParseTuple(args, "O", &pypoints)){
        return NULL;
    }
    if (PyObject_Length(pypoints) < 0){
        return NULL;
    }

    n = getNModule(pypoints);
    d = getNModule(PyList_GetItem(pypoints, 0));

    points = convertPyToC(pypoints, n, d);

    if (!points){
        return NULL;
    }

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

    pyres = convertCToPy(targetMat, n, n);

    freeMemoryModule(points, n, targetMat, n, NULL, 0, NULL, 0);
    
    return pyres;
}

void updateH(double** h, double** h_prior, double** w, int n, int k, double** resRow) {
    /* receives identical matrices but function changes h inplace */
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
    double whij = innerProduct(w, h_prior, i, j, n);
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
        for (t = 0 ; t < k ; t++) { /* run over common dimension */
            sum += h[i][t] * h[m][t]; /* inner product of transpose */
        }
        resRow[0][m] = sum;
        sum = 0;
    }
    /* this is equivalent to (h * h^t)*h [i][j] */
    return innerProduct(resRow, h, 0, j, n);
}

int isConverged(double** h, double** h_prior, int n, int k) {
    /* 1 on convergance, 0 else */
    int i,j;
    double frobBeforeSqrt = 0;
    for (i = 0 ; i < n ; i++) {
        for (j = 0 ; j < k ; j++) {
            frobBeforeSqrt += pow(fabs(h[i][j] - h_prior[i][j]), 2);
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


