#include <Python.h>
#include <numpy/arrayobject.h>

#include <biosig.h>

#define BIOSIG_MODULE
#include "biosigmodule.h"

#if PY_MAJOR_VERSION >= 3
  #define MOD_ERROR_VAL NULL
  #define MOD_SUCCESS_VAL(val) val
  #define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
          ob = PyModule_Create(&moduledef);
#else
  #define MOD_ERROR_VAL
  #define MOD_SUCCESS_VAL(val)
  #define MOD_INIT(name) void init##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          ob = Py_InitModule3(name, methods, doc);
#endif

static PyObject *BiosigError;

static int PyBiosig_Header(const char *filename, char **jsonstr) {
	// open file 
	HDRTYPE *hdr = NULL;
	hdr = sopen(filename, "r", hdr);

	if (serror2(hdr)) {
	        PyErr_SetString(BiosigError, "could not open file");
		destructHDR(hdr);
	        return -1;
	}

	// convert to json-string
	char *str = NULL;
	asprintf_hdr2json(&str, hdr);

	destructHDR(hdr);

	*jsonstr = strdup(str);
	return 0;
}

static PyObject *biosig_json_header(PyObject *self, PyObject *args) {
	// get input arguments 

	const char *filename = NULL;
	char *str = NULL;

	if (!PyArg_ParseTuple(args, "s", &filename)) return NULL;

	if (PyBiosig_Header(filename, &str)) return NULL;

	return Py_BuildValue("s", str);
}

static int PyBiosig_Data(const char *filename, PyObject **D) {
	// open file
	HDRTYPE *hdr = sopen(filename, "r", NULL);

	if (serror2(hdr)) {
	        PyErr_SetString(BiosigError, "could not open file");
		destructHDR(hdr);
	        return -1;
	}

	const int nd=2;
	npy_intp dims[nd];
	dims[0] = (int)biosig_get_number_of_samples(hdr);
	dims[1] = (int)biosig_get_number_of_channels(hdr);
	int type_num;

	switch (sizeof(biosig_data_type)) {
	case 4:
		type_num=NPY_FLOAT32;
		break;
	case 8:
		type_num=NPY_FLOAT64;
		break;
#if NPY_BITSOF_LONGDOUBLE >= 128
	case 16:
		type_num=NPY_FLOAT128;
		break;
#endif
	}

        *D = PyArray_SimpleNew(nd, dims, type_num);
	hdr->FLAG.ROW_BASED_CHANNELS = 1;

	/*
	*D = PyArray_New(&PyArray_Type, nd, dims, type_num, NULL, NULL, 0, NPY_ARRAY_CARRAY, NULL);
	hdr->FLAG.ROW_BASED_CHANNELS = 0;
	*/
	size_t count = sread((double*)(((PyArrayObject *)(*D))->data), 0, biosig_get_number_of_records(hdr), hdr);
//	size_t count = sread((double*)(PyArray_Data(D)), 0, biosig_get_number_of_records(hdr), hdr);

	hdr->data.block = NULL;
	destructHDR(hdr);

	return 0;
}

static PyObject *biosig_data(PyObject *self, PyObject *args) {
	// get input arguments
	const char *filename = NULL;

	if (!PyArg_ParseTuple(args, "s", &filename)) return NULL;

	PyObject* Data;

	if (PyBiosig_Data(filename, &Data)) return NULL;

	return Data;
}



static PyMethodDef BiosigMethods[] = {
    {"header",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
    {"data",    biosig_data,        METH_VARARGS, "load biosig data."},
/*
    {"base64",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
    {"fhir_json_binary_template",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
    {"fhir_xml_binary_template",  biosig_json_header, METH_VARARGS, "load biosig header and export as JSON ."},
*/
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

const char module___doc__[] = "biosig tools for loading signal data";

#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "biosig",     /* m_name */
        module___doc__,      /* m_doc */
        -1,                  /* m_size */
        BiosigMethods,       /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

MOD_INIT(biosig) {
    import_array();

    PyObject *m;
    MOD_DEF(m, "biosig", module___doc__, BiosigMethods);
    if (m == NULL) return MOD_ERROR_VAL;

    BiosigError = PyErr_NewException("biosig.error", NULL, NULL);
    Py_INCREF(BiosigError);
    PyModule_AddObject(m, "error", BiosigError);

     /* additional initialization can happen here */

   return MOD_SUCCESS_VAL(m);
}

