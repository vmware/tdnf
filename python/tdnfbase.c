/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include "tdnfbase.h"

static char base__doc__[] = "";

static void
base_dealloc(PPY_TDNF_BASE self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
base_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PPY_TDNF_BASE self = NULL;

    self = (PPY_TDNF_BASE)type->tp_alloc(type, 0);
    if (self != NULL)
    {
    }

    return (PyObject *)self;
}

static int
base_init(PPY_TDNF_BASE self, PyObject *args, PyObject *kwds)
{
    uint32_t dwError = 0;

//error:
    return dwError;
}

static PyObject *
base_get_version(
    PPY_TDNF_BASE self,
    void *closure)
{
    return NULL;
}

static PyGetSetDef base_getset[] = {
    {"version",
     (getter)base_get_version, (setter)NULL,
     "tdnf version",
     NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef base_methods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyMemberDef base_members[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject baseType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "tdnf.Base",               /*tp_name*/
    sizeof(PY_TDNF_BASE),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)base_dealloc,  /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    base__doc__,               /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    base_methods,              /* tp_methods */
    base_members,              /* tp_members */
    base_getset,               /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)base_init,       /* tp_init */
    0,                         /* tp_alloc */
    base_new,                  /* tp_new */
    0,                         /* tp_free */
    0,                         /* tp_is_gc */
};
