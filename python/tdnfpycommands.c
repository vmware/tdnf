/*
 * Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

/* returns a list of repos */
PyObject *
TDNFPyRepoList(PyObject *self, PyObject *args, PyObject *kwds)
{
    uint32_t dwError = 0;
    char *kwlist[] = { "filter", "config", NULL };
    PyObject *ppyRepoList = Py_None;
    TDNF_CMD_ARGS cmdArgs = {0};
    char *szCmds[] = {""};
    PTDNF pTDNF = NULL;
    PTDNF_REPO_DATA pRepos = NULL;
    PTDNF_REPO_DATA pReposTemp = NULL;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;
    PyObject *CfgFile = NULL;

    cmdArgs.pszInstallRoot = "/";
    cmdArgs.ppszCmds = szCmds;
    cmdArgs.nCmdCount = 1;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "|iO", kwlist,
            &nFilter, &CfgFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (CfgFile)
    {
        const char *fname = PyUnicode_AsUTF8(CfgFile);
        dwError = TDNFAllocateString(fname, &cmdArgs.pszConfFile);
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
    TDNF_SAFE_FREE_MEMORY(cmdArgs.pszConfFile);
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

/* alter */
uint32_t
_TDNFPyGetAlterArgs(TDNF_ALTERTYPE type, PyObject *args,
                    PyObject *kwds, PTDNF_CMD_ARGS pCmdArgs)
{
    size_t nPkgCount = 0;
    uint32_t dwError = 0;
    char *kwlist[] = { "pkgs", "refresh", "quiet", "config", NULL };
    PyObject *pyPkgList = NULL;
    PyObject *pyRefresh = NULL;
    PyObject *pyQuiet = NULL;
    PyObject *CfgFile = NULL;
    PyObject *ppyString = NULL;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "|O!O!O!O", kwlist,
            &PyList_Type, &pyPkgList,
            &PyBool_Type, &pyRefresh,
            &PyBool_Type, &pyQuiet,
            &CfgFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(pyPkgList)
    {
        ppyString = PyBytes_FromFormat("%s", " ");
        if (!ppyString)
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        /* add an empty item in front of list for cmdargs book keeping */
        dwError = PyList_Insert(pyPkgList, 0, ppyString);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFPyListAsStringList(
                     pyPkgList,
                     &pCmdArgs->ppszCmds,
                     &nPkgCount);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateMemory(2, sizeof(char *), (void **)&pCmdArgs->ppszCmds);
        BAIL_ON_TDNF_ERROR(dwError);

        pCmdArgs->ppszCmds[0] = strdup("");
        nPkgCount = 1;
    }

    if (CfgFile)
    {
        const char *fname = PyUnicode_AsUTF8(CfgFile);
        dwError = TDNFAllocateString(fname, &pCmdArgs->pszConfFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCmdArgs->nRefresh = pyRefresh ? PyObject_IsTrue(pyRefresh) : 0;
    pCmdArgs->nQuiet = pyQuiet ? PyObject_IsTrue(pyQuiet) : 0;
    pCmdArgs->nCmdCount = nPkgCount;

error:
    return dwError;
}

/* alter */
PyObject *
_TDNFPyAlter(TDNF_ALTERTYPE alterType, PyObject *self, PyObject *args, PyObject *kwds)
{
    uint32_t dwError = 0;
    TDNF_CMD_ARGS cmdArgs = {0};
    PTDNF pTDNF = NULL;
    PTDNF_SOLVED_PKG_INFO pSolvedInfo = NULL;

    cmdArgs.pszInstallRoot = "/";

    dwError = _TDNFPyGetAlterArgs(alterType, args, kwds, &cmdArgs);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFOpenHandle(&cmdArgs, &pTDNF);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFResolve(pTDNF, alterType, &pSolvedInfo);
    if (dwError == ERROR_TDNF_ALREADY_INSTALLED)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if (pSolvedInfo && pSolvedInfo->nNeedAction)
    {
        dwError = TDNFAlterCommand(pTDNF, alterType, pSolvedInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(cmdArgs.pszConfFile);
    TDNFFreeStringArrayWithCount(cmdArgs.ppszCmds, cmdArgs.nCmdCount);
    TDNFFreeSolvedPackageInfo(pSolvedInfo);
    if (pTDNF)
    {
        TDNFCloseHandle(pTDNF);
    }
    return Py_BuildValue("i", dwError);
error:
    TDNFPyRaiseException(self, dwError);
    goto cleanup;
}

/* install command */
PyObject *
TDNFPyInstall(PyObject *self, PyObject *args, PyObject *kwds)
{
    return _TDNFPyAlter(ALTER_INSTALL, self, args, kwds);
}

/* update command */
PyObject *
TDNFPyUpdate(PyObject *self, PyObject *args, PyObject *kwds)
{
    return _TDNFPyAlter(ALTER_UPGRADE, self, args, kwds);
}

/* downgrade command */
PyObject *
TDNFPyDowngrade(PyObject *self, PyObject *args, PyObject *kwds)
{
    return _TDNFPyAlter(ALTER_DOWNGRADE, self, args, kwds);
}

/* erase command */
PyObject *
TDNFPyErase(PyObject *self, PyObject *args, PyObject *kwds)
{
    return _TDNFPyAlter(ALTER_ERASE, self, args, kwds);
}

/* distro_sync command */
PyObject *
TDNFPyDistroSync(PyObject *self, PyObject *args, PyObject *kwds)
{
    return _TDNFPyAlter(ALTER_DISTRO_SYNC, self, args, kwds);
}
