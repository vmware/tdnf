/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : parserepolistargs.c
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
TDNFCliParseRepoListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_REPOLISTFILTER* pnFilter
    )
{
    uint32_t dwError = 0;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;

    if(!pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    //Could have filter argument (tdnf repolist <filter>)
    //assume REPOLISTFILTER_ENABLED if not specified.
    if(pCmdArgs->nCmdCount > 1)
    {
        dwError = TDNFCliParseFilter(pCmdArgs->ppszCmds[1], &nFilter);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *pnFilter = nFilter;

cleanup:
    return dwError;

error:
    if(pnFilter)
    {
        *pnFilter = REPOLISTFILTER_ENABLED;
    }
    goto cleanup;
}

uint32_t
TDNFCliParseFilter(
    const char* pszFilter,
    TDNF_REPOLISTFILTER* pnFilter
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    TDNF_REPOLISTFILTER nFilter = REPOLISTFILTER_ENABLED;
    struct stTemp
    {
        char* pszTypeName;
        int nType;
    };
    struct stTemp  stFilterTypes[] =
    {
        {"all",      REPOLISTFILTER_ALL},
        {"enabled",  REPOLISTFILTER_ENABLED},
        {"disabled", REPOLISTFILTER_DISABLED}
    };
    int nCount = ARRAY_SIZE(stFilterTypes);
    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
        if(!strcasecmp(stFilterTypes[nIndex].pszTypeName, pszFilter))
        {
            nFilter = stFilterTypes[nIndex].nType;
            break;
        }
    }
    if(nIndex >= nCount)
    {
        dwError = ERROR_TDNF_CLI_NO_MATCH;
    }
    BAIL_ON_CLI_ERROR(dwError);

    *pnFilter = nFilter;
cleanup:
    return dwError;

error:
    if(pnFilter)
    {
        *pnFilter = REPOLISTFILTER_ENABLED;
    }
    goto cleanup;
}
