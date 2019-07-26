/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <tdnf/tdnf.h>

/* returns a list of repos */
PyObject *
TDNFPyRepoList(PyObject *self, PyObject *args, PyObject *kwds)
{
    uint32_t dwError = 0;
    char *kwlist[] = { "filter", NULL };
    PyObject *ppyRepoList = Py_None;
    TDNF_CMD_ARGS cmdArgs = {0};
    char *szCmds[] = {""};
    PTDNF pTDNF = NULL;
    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pReposTemp = NULL;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;

    cmdArgs.pszInstallRoot = "/";
    cmdArgs.ppszCmds = szCmds;
    cmdArgs.nCmdCount = 1;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "|i", kwlist,
            &nFilter))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppyRepoList = PyList_New(0);
    if (!ppyRepoList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFOpenHandle(&cmdArgs, &pTDNF);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nFilter < REPOLISTFILTER_ALL || nFilter > REPOLISTFILTER_DISABLED)
    {
        nFilter = REPOLISTFILTER_ENABLED;
    }
    dwError = TDNFRepoList(pTDNF, nFilter, &pRepos);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pReposTemp = pRepos; pReposTemp; pReposTemp = pReposTemp->pNext)
    {
        PyObject *pPyRepoData = NULL;

        dwError = TDNFPyMakeRepoData(pReposTemp, &pPyRepoData);
        BAIL_ON_TDNF_ERROR(dwError);

        if(PyList_Append(ppyRepoList, pPyRepoData) == -1)
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    if (pRepos)
    {
        TDNFFreeRepos(pRepos);
    }
    if (pTDNF)
    {
        TDNFCloseHandle(pTDNF);
    }
    return ppyRepoList;

error:
    TDNFPyRaiseException(self, dwError);
    goto cleanup;
}
