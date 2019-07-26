/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static char tdnf__doc__[] = "";

PyObject *pyTDNFError = NULL;

static int
prepareInitModule(
    )
{
    return 1;
}

static PyMethodDef tdnfMethods[] =
{
    {NULL}  /* Sentinel */
};

static int
initModule(
    PyObject *pModule
    )
{
    int ret = 0;
    int error = 0;
    PyObject *pyDict = NULL;

    error = TDNFInit();
    BAIL_ON_TDNF_ERROR(error);

    pyDict = PyModule_GetDict(pModule);
    if (!pyDict)
    {
        error = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(error);
    }

    pyTDNFError = PyErr_NewException("_tdnf.error", NULL, NULL);
    if (pyTDNFError != NULL)
    {
	PyDict_SetItemString(pyDict, "error", pyTDNFError);
    }

    PyModule_AddStringConstant(pModule, "__version__", TDNFGetVersion());

    ret = 1;

error: 
    return ret;
}

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef tdnfModule =
{
    PyModuleDef_HEAD_INIT,
    "_tdnf",     /* name of module */
    tdnf__doc__, /* module documentation, may be NULL */
    0,           /* m_size */
    tdnfMethods
};

PyObject *
PyInit__tdnf(
     )
{
    PyObject *pModule = NULL;

    if (!prepareInitModule())
        return NULL;

    pModule = PyModule_Create(&tdnfModule);
    if(!pModule)
    {
        goto error;
    }

    if(!initModule(pModule))
    {
        goto error;
    }

cleanup:
    return pModule;

error:
    if(pModule)
    {
        Py_XDECREF(pModule);
    }
    pModule = NULL;
    goto cleanup;
}

int
main(
    int argc,
    char *argv[]
    )
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    PyImport_AppendInittab("_tdnf", PyInit__tdnf);

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    PyImport_ImportModule("_tdnf");

    PyMem_RawFree(program);
    return 0;
}

#else

PyMODINIT_FUNC
init_tdnf(
    )
{
    PyObject *pModule = NULL;

    if (!prepareInitModule())
        return;

    pModule = Py_InitModule3("_tdnf", tdnfMethods, tdnf__doc__);
    if(pModule)
    {
        initModule(pModule);
    }
}

int
main(
    int argc,
    char *argv[]
    )
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);


    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();

    /* Add a static module */
    init_tdnf();

    return 0;
}

#endif
