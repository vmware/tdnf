/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFCliParseCleanArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    uint32_t* pnCleanType
    )
{
    uint32_t dwError = 0;
    uint32_t nCleanType = CLEANTYPE_NONE;

    if(!pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pCmdArgs->nCmdCount == 1)
    {
        dwError = ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION;
        BAIL_ON_CLI_ERROR(dwError);
    }

    //Should have type argument (tdnf clean <type>)
    if(pCmdArgs->nCmdCount > 1)
    {
        dwError = TDNFCliParseCleanType(pCmdArgs->ppszCmds[1], &nCleanType);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *pnCleanType = nCleanType;

cleanup:
    return dwError;

error:
    if(pnCleanType)
    {
        *pnCleanType = CLEANTYPE_NONE;
    }
    goto cleanup;
}

uint32_t
TDNFCliParseCleanType(
    const char* pszCleanType,
    uint32_t* pnCleanType
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    uint32_t nCleanType = CLEANTYPE_ALL;
    struct stTemp
    {
        char* pszTypeName;
        int nType;
    };
    struct stTemp  stCleanTypes[] =
    {
        {"packages",      CLEANTYPE_PACKAGES},
        {"metadata",      CLEANTYPE_METADATA},
        {"dbcache",       CLEANTYPE_DBCACHE},
        {"plugins",       CLEANTYPE_PLUGINS},
        {"keys",          CLEANTYPE_KEYS},
        {"expire-cache",  CLEANTYPE_EXPIRE_CACHE},
        {"all",           CLEANTYPE_ALL},
    };
    int nCount = ARRAY_SIZE(stCleanTypes);
    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
        if(!strcasecmp(stCleanTypes[nIndex].pszTypeName, pszCleanType))
        {
            nCleanType = stCleanTypes[nIndex].nType;
            break;
        }
    }
    if(nIndex >= nCount)
    {
        dwError = ERROR_TDNF_CLI_NO_MATCH;
    }
    BAIL_ON_CLI_ERROR(dwError);

    *pnCleanType = nCleanType;
cleanup:
    return dwError;

error:
    if(pnCleanType)
    {
        *pnCleanType = CLEANTYPE_NONE;
    }
    goto cleanup;
}
