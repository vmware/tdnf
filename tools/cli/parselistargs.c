/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
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
                            sizeof(TDNF_LIST_ARGS),
                            (void**)&pListArgs);
    BAIL_ON_CLI_ERROR(dwError);

    //Should have scope argument (tdnf list <scope> <pkgnamespecs>)
    if(pCmdArgs->nCmdCount > 1)
    {
        nStartIndex = 2;
        dwError = ParseScope(pCmdArgs->ppszCmds[1], &pListArgs->nScope);
        if(dwError == ERROR_TDNF_CLI_NO_MATCH)
        {
            dwError = 0;
            pListArgs->nScope = SCOPE_ALL;
            nStartIndex = 1;
        }
    }
    //Copy the rest of the args as package name specs
    nPackageCount = pCmdArgs->nCmdCount - nStartIndex;
    dwError = TDNFAllocateMemory(
                  sizeof(char*) * (nPackageCount + 1),
                  (void**)&pListArgs->ppszPackageNameSpecs);
    BAIL_ON_CLI_ERROR(dwError);

    for(nIndex = 0; nIndex < nPackageCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                      pCmdArgs->ppszCmds[nStartIndex],
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
        TDNFFreeListArgs(pListArgs);
    }
    goto cleanup;
}

uint32_t
ParseScope(
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
        {"updates",   SCOPE_UPGRADES}
    };
    int nCount = sizeof(stScopes)/sizeof(stScopes[0]);
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
    if(pnScope)
    {
        *pnScope = SCOPE_ALL;//Default
    }
    goto cleanup;
}

void
TDNFFreeListArgs(
    PTDNF_LIST_ARGS pListArgs
    )
{
    if(pListArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pListArgs->ppszPackageNameSpecs);
        TDNFFreeMemory(pListArgs);
    }
}
