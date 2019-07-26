/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
py_string_as_string(
    PyObject *pyObj,
    PyObject **ppString
    )
{
    uint32_t dwError = 0;
    PyObject *pString = NULL;
    if(!pyObj || !ppString)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(PyBytes_Check(pyObj))
    {
        Py_XINCREF(pyObj);
        pString = pyObj;
    }
    else if(PyUnicode_Check(pyObj))
    {
        pString = PyUnicode_AsUTF8String(pyObj);
    }

    if(!pString)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppString = pString;
cleanup:
    return dwError;

error:
    goto cleanup;
}

char *
string_from_py_string(
    PyObject *pyString
    )
{
    char *pszResult = PyBytes_AsString(pyString);
    if(!pszResult || !*pszResult)
    {
        pszResult = NULL;
    }
    return pszResult;
}
