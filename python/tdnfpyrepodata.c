/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include "tdnfpyrepodata.h"

static char repodata__doc__[] = "";

static void
TDNFPyRepoDataFree(PY_TDNF_REPODATA *self)
{
    Py_XDECREF(self->id);
    Py_XDECREF(self->name);
    Py_XDECREF(self->baseurl);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
TDNFPyRepoDataNew(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    uint32_t dwError = 0;
    PPY_TDNF_REPODATA self = NULL;

    self = (PPY_TDNF_REPODATA)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        if(!(self->id = PyBytes_FromString("")))
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if(!(self->name = PyBytes_FromString("")))
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if(!(self->baseurl = PyBytes_FromString("")))
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    return (PyObject *)self;

error:
    Py_DECREF(self);
    self = NULL;
    goto cleanup;
}

static int
TDNFPyRepoDataInit(
    PY_TDNF_REPODATA *self,
    PyObject *args,
    PyObject *kwds
    )
{
    uint32_t dwError = 0;
    PyObject *id = NULL;
    PyObject *name = NULL;
    PyObject *baseurl = NULL;
    PyObject *tmp = NULL;

    static char *kwlist[] = {"id", "name", "baseurl", "enabled", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|SSSI", kwlist,
                                      &id, &name, &baseurl, &self->enabled))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (id)
    {
        tmp = self->id;
        Py_INCREF(id);
        self->id = id;
        Py_XDECREF(tmp);
    }
    if (name)
    {
        tmp = self->name;
        Py_INCREF(name);
        self->name = name;
        Py_XDECREF(tmp);
    }
    if (baseurl)
    {
        tmp = self->baseurl;
        Py_INCREF(baseurl);
        self->baseurl = baseurl;
        Py_XDECREF(tmp);
    }

cleanup:
    return dwError > 0 ? -1 : 0;

error:
    fprintf(stderr, "Error = %d\n", dwError);
    goto cleanup;
}

PyObject*
TDNFPyRepoDataRepr(
    PyObject *self
    )
{
    uint32_t dwError = 0;
    PyObject *pyRepr = Py_None;
    PPY_TDNF_REPODATA pRepoData = NULL;
    char *pszRepr = NULL;

    pRepoData = (PPY_TDNF_REPODATA)self;
    dwError = TDNFAllocateStringPrintf(
                  &pszRepr,
                  "{id: %s, name: %s, baseurl: %s, enabled: %d}",
                  pRepoData->id ? PyBytes_AsString(pRepoData->id) : "",
                  pRepoData->name ? PyBytes_AsString(pRepoData->name) : "",
                  pRepoData->baseurl ? PyBytes_AsString(pRepoData->baseurl) : "",
                  pRepoData->enabled);
    BAIL_ON_TDNF_ERROR(dwError);

    pyRepr = Py_BuildValue("s", pszRepr);
    Py_INCREF(pyRepr);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRepr);
    return pyRepr;

error:
    printf("Error = %d\n", dwError);
    pyRepr = Py_None;
    goto cleanup;

}

PyObject*
TDNFPyRepoDataStr(
    PyObject *self
    )
{
    return TDNFPyRepoDataRepr(self);
}

uint32_t
TDNFPyMakeRepoData(
   PTDNF_REPO_DATA pRepoData,
   PyObject **ppPyRepoData
   )
{
    uint32_t dwError = 0;
    PPY_TDNF_REPODATA pPyRepoData = NULL;
    PyTypeObject *retType = &repodataType;

    if(!pRepoData || !ppPyRepoData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPyRepoData = (PPY_TDNF_REPODATA)retType->tp_alloc(retType, 0);
    if(!pPyRepoData)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pPyRepoData->id = PyBytes_FromString(pRepoData->pszId);
    pPyRepoData->name = PyBytes_FromString(pRepoData->pszName);
    pPyRepoData->baseurl = PyBytes_FromString(pRepoData->pszBaseUrl);
    pPyRepoData->enabled = pRepoData->nEnabled;

    *ppPyRepoData = (PyObject *)pPyRepoData;
cleanup:
    return dwError;

error:
    Py_XDECREF(pPyRepoData);
    goto cleanup;
}

static PyGetSetDef TDNFPyRepoDataGetSet[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef TDNFPyRepoDataMethods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyMemberDef TDNFPyRepoDataMembers[] = {
    {"id", T_OBJECT_EX, offsetof(PY_TDNF_REPODATA, id), 0,
     "repo id"},
    {"name", T_OBJECT_EX, offsetof(PY_TDNF_REPODATA, name), 0,
     "repo name"},
    {"baseurl", T_OBJECT_EX, offsetof(PY_TDNF_REPODATA, baseurl), 0,
     "repo baseurl"},
    {"enabled", T_INT, offsetof(PY_TDNF_REPODATA, enabled), 0,
     "repo enabled status"},
    {NULL}  /* Sentinel */
};

PyTypeObject repodataType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "repodata",                                  /*tp_name*/
    sizeof(PY_TDNF_REPODATA),                    /*tp_basicsize*/
    0,                                           /*tp_itemsize*/
    (destructor)TDNFPyRepoDataFree,              /*tp_dealloc*/
    0,                                           /*tp_print*/
    0,                                           /*tp_getattr*/
    0,                                           /*tp_setattr*/
    0,                                           /*tp_compare*/
    TDNFPyRepoDataRepr,                          /*tp_repr*/
    0,                                           /*tp_as_number*/
    0,                                           /*tp_as_sequence*/
    0,                                           /*tp_as_mapping*/
    0,                                           /*tp_hash */
    0,                                           /*tp_call*/
    TDNFPyRepoDataStr,                           /*tp_str*/
    0,                                           /*tp_getattro*/
    0,                                           /*tp_setattro*/
    0,                                           /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /*tp_flags*/
    repodata__doc__,                             /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    0,                                           /* tp_iter */
    0,                                           /* tp_iternext */
    TDNFPyRepoDataMethods,                       /* tp_methods */
    TDNFPyRepoDataMembers,                       /* tp_members */
    TDNFPyRepoDataGetSet,                        /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    (initproc)TDNFPyRepoDataInit,                /* tp_init */
    0,                                           /* tp_alloc */
    TDNFPyRepoDataNew,                           /* tp_new */
    0,                                           /* tp_free */
    0                                            /* tp_is_gc */
};
