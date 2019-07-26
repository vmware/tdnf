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

void
TDNFPyRaiseException(
    PyObject *self,
    uint32_t dwErrorCode
    )
{
    uint32_t dwError = 0;
    char *pszError = NULL;
    char *pszMessage = NULL;

    dwError = TDNFGetErrorString(dwErrorCode, &pszError);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(&pszMessage,
                                       "Error = %d: %s",
                                       dwErrorCode,
                                       pszError);
    BAIL_ON_TDNF_ERROR(dwError);

    PyErr_SetString(PyExc_Exception, pszMessage);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszMessage);
    TDNF_SAFE_FREE_MEMORY(pszError);
    return;

error:
    goto cleanup;
}

uint32_t
TDNFPyAddEnums(PyObject *pModule)
{
    uint32_t dwError = 0;
    if (!pModule)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = PyModule_AddIntMacro(pModule, REPOLISTFILTER_ALL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = PyModule_AddIntMacro(pModule, REPOLISTFILTER_ENABLED);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = PyModule_AddIntMacro(pModule, REPOLISTFILTER_DISABLED);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}
