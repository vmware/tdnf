/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : parselistargs.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

uint32_t
TDNFCliParseInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    )
{
    return TDNFCliParseListArgs(
        pCmdArgs,
        ppListArgs);
}

uint32_t
TDNFCliParsePackageArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    char*** pppszPackageArgs,
    int* pnPackageCount
    )
{
    uint32_t dwError = 0;
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;
    int nIndex = 0;

    if(!pCmdArgs || !pppszPackageArgs || !pnPackageCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nPackageCount = pCmdArgs->nCmdCount - 1;
    if(nPackageCount < 0)
    {
        dwError = ERROR_TDNF_CLI_NOT_ENOUGH_ARGS;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  nPackageCount + 1,
                  sizeof(char*),
                  (void**)&ppszPackageArgs);
    BAIL_ON_CLI_ERROR(dwError);

    for(nIndex = 0; nIndex < nPackageCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                      pCmdArgs->ppszCmds[nIndex+1],
                      &ppszPackageArgs[nIndex]);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *pppszPackageArgs = ppszPackageArgs;
cleanup:
    return dwError;

error:
    if(pppszPackageArgs)
    {
        *pppszPackageArgs = NULL;
    }
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    goto cleanup;
}

uint32_t
TDNFCliParseListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_LIST_ARGS pListArgs = NULL;
    int nStartIndex = 1;
    int nPackageCount = 0;
    int nIndex = 0;

    if(!pCmdArgs || pCmdArgs->nCmdCount < 1)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }
    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_LIST_ARGS),
                  (void**)&pListArgs);
    BAIL_ON_CLI_ERROR(dwError);

    //Should have scope argument (tdnf list <scope> <pkgnamespecs>)
    pListArgs->nScope = SCOPE_ALL;
    if(pCmdArgs->nCmdCount > 1)
    {
        nStartIndex = 2;
        dwError = TDNFCliParseScope(pCmdArgs->ppszCmds[1], &pListArgs->nScope);
        if(dwError == ERROR_TDNF_CLI_NO_MATCH)
        {
            dwError = 0;
            nStartIndex = 1;
        }
    }
    //Copy the rest of the args as package name specs
    nPackageCount = pCmdArgs->nCmdCount - nStartIndex;
    dwError = TDNFAllocateMemory(
                  nPackageCount + 1,
                  sizeof(char*),
                  (void**)&pListArgs->ppszPackageNameSpecs);
    BAIL_ON_CLI_ERROR(dwError);

    for(nIndex = 0; nIndex < nPackageCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                      pCmdArgs->ppszCmds[nStartIndex + nIndex],
                      &pListArgs->ppszPackageNameSpecs[nIndex]);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *ppListArgs = pListArgs;

cleanup:
    return dwError;

error:
    if(ppListArgs)
    {
        *ppListArgs = NULL;
    }
    if(pListArgs)
    {
        TDNFCliFreeListArgs(pListArgs);
    }
    goto cleanup;
}

uint32_t
TDNFCliParseScope(
    const char* pszScope,
    TDNF_SCOPE* pnScope
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    TDNF_SCOPE nScope = SCOPE_ALL;
    struct stTemp
    {
        char* pszTypeName;
        int nType;
    };
    struct stTemp  stScopes[] =
    {
        {"all",       SCOPE_ALL},
        {"installed", SCOPE_INSTALLED},
        {"available", SCOPE_AVAILABLE},
        {"extras",    SCOPE_EXTRAS},
        {"obsoletes", SCOPE_OBSOLETES},
        {"recent",    SCOPE_RECENT},
        {"upgrades",  SCOPE_UPGRADES},
        {"updates",   SCOPE_UPGRADES},
        {"downgrades",SCOPE_DOWNGRADES}
    };
    int nCount = ARRAY_SIZE(stScopes);
    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
        if(!strcasecmp(stScopes[nIndex].pszTypeName, pszScope))
        {
            nScope = stScopes[nIndex].nType;
            break;
        }
    }
    if(nIndex >= nCount)
    {
        dwError = ERROR_TDNF_CLI_NO_MATCH;
    }
    BAIL_ON_CLI_ERROR(dwError);

    *pnScope = nScope;
cleanup:
    return dwError;

error:
    goto cleanup;
}

void
TDNFCliFreeListArgs(
    PTDNF_LIST_ARGS pListArgs
    )
{
    if(pListArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pListArgs->ppszPackageNameSpecs);
        TDNFFreeMemory(pListArgs);
    }
}
