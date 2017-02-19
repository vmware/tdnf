/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
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
                            1,
                            sizeof(TDNF_UPDATEINFO_ARGS),
                            (void**)&pUpdateInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    //Assume first arg as mode 
    //(tdnf updateinfo <mode> <availability> <type> <pkgnamespecs>)
    if(pCmdArgs->nCmdCount > nStartIndex)
    {
        dwError = TDNFCliParseMode(
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
        dwError = TDNFCliParseScope(
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
                  nPackageCount + 1,
                  sizeof(char*),
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
        TDNFCliFreeUpdateInfoArgs(pUpdateInfoArgs);
    }
    goto cleanup;
}

uint32_t
TDNFCliParseMode(
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
TDNFCliFreeUpdateInfoArgs(
    PTDNF_UPDATEINFO_ARGS pUpdateInfoArgs
    )
{
    if(pUpdateInfoArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pUpdateInfoArgs->ppszPackageNameSpecs);
        TDNFFreeMemory(pUpdateInfoArgs);
    }
}
