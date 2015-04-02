/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : parseupdateinfo.c
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

static
uint32_t
ParseMode(
    const char* pszOutMode,
    TDNF_UPDATEINFO_OUTPUT* pnOutMode
    );

uint32_t
TDNFCliParseUpdateInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS* ppUpdateInfoArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_UPDATEINFO_ARGS pUpdateInfoArgs = NULL;
    int nStartIndex = 1;
    int nPackageCount = 0;
    int nIndex = 0;

    if(!pCmdArgs || pCmdArgs->nCmdCount < 1)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                            sizeof(TDNF_UPDATEINFO_ARGS),
                            (void**)&pUpdateInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    //Assume first arg as mode 
    //(tdnf updateinfo <mode> <availability> <type> <pkgnamespecs>)
    if(pCmdArgs->nCmdCount > nStartIndex)
    {
        dwError = ParseMode(
                      pCmdArgs->ppszCmds[nStartIndex],
                      &pUpdateInfoArgs->nMode);
        if(dwError == ERROR_TDNF_CLI_NO_MATCH)
        {
            dwError = 0;
            pUpdateInfoArgs->nMode = OUTPUT_SUMMARY;
            //There was no valid mode. Let the next arg start here.
            --nStartIndex;
        }
        ++nStartIndex;
    }

    //Next arg should be availability
    if(pCmdArgs->nCmdCount > nStartIndex)
    {
        dwError = ParseScope(
                      pCmdArgs->ppszCmds[nStartIndex],
                      &pUpdateInfoArgs->nScope);
        if(dwError == ERROR_TDNF_CLI_NO_MATCH)
        {
            dwError = 0;
            pUpdateInfoArgs->nScope = SCOPE_AVAILABLE;
            //There was no valid availability. Let the next arg start here.
            --nStartIndex;
        }
        ++nStartIndex;
    }

    //Copy the rest of the args as package name specs
    nPackageCount = pCmdArgs->nCmdCount - nStartIndex;
    dwError = TDNFAllocateMemory(
                  sizeof(char*) * (nPackageCount + 1),
                  (void**)&pUpdateInfoArgs->ppszPackageNameSpecs);
    BAIL_ON_CLI_ERROR(dwError);

    for(nIndex = 0; nIndex < nPackageCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                      pCmdArgs->ppszCmds[nStartIndex++],
                      &pUpdateInfoArgs->ppszPackageNameSpecs[nIndex]);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *ppUpdateInfoArgs = pUpdateInfoArgs;

cleanup:
    return dwError;

error:
    if(ppUpdateInfoArgs)
    {
        *ppUpdateInfoArgs = NULL;
    }
    if(pUpdateInfoArgs)
    {
        TDNFFreeUpdateInfoArgs(pUpdateInfoArgs);
    }
    goto cleanup;
}

uint32_t
ParseMode(
    const char* pszMode,
    TDNF_UPDATEINFO_OUTPUT* pnMode
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    TDNF_UPDATEINFO_OUTPUT nMode = OUTPUT_SUMMARY;
    struct stTemp
    {
        char* pszModeName;
        int nMode;
    };
    struct stTemp  stModes[] = 
    {
        {"summary",   OUTPUT_SUMMARY},
        {"list",      OUTPUT_LIST},
        {"info",      OUTPUT_INFO}
    };
    int nCount = sizeof(stModes)/sizeof(stModes[0]);

    if(IsNullOrEmptyString(pszMode) || !pnMode)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
        if(!strcasecmp(stModes[nIndex].pszModeName, pszMode))
        {
            nMode = stModes[nIndex].nMode;
            break;
        }
    }
    if(nIndex >= nCount)
    {
        dwError = ERROR_TDNF_CLI_NO_MATCH;
    }
    BAIL_ON_CLI_ERROR(dwError);

    *pnMode = nMode;
cleanup:
    return dwError;

error:
    if(pnMode)
    {
        *pnMode = OUTPUT_SUMMARY;//Default
    }
    goto cleanup;
}

void
TDNFFreeUpdateInfoArgs(
    PTDNF_UPDATEINFO_ARGS pUpdateInfoArgs
    )
{
    if(pUpdateInfoArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pUpdateInfoArgs->ppszPackageNameSpecs);
        TDNFFreeMemory(pUpdateInfoArgs);
    }
}
