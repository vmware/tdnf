/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : parsecleanargs.c
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
ParseCleanType(
    const char* pszCleanType,
    TDNF_CLEANTYPE* pnCleanType
    );

uint32_t
TDNFCliParseCleanArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_CLEANTYPE* pnCleanType
    )
{
    uint32_t dwError = 0;
    TDNF_CLEANTYPE nCleanType = CLEANTYPE_NONE;

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
        dwError = ParseCleanType(pCmdArgs->ppszCmds[1], &nCleanType);
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
ParseCleanType(
    const char* pszCleanType,
    TDNF_CLEANTYPE* pnCleanType
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    TDNF_CLEANTYPE nCleanType = CLEANTYPE_ALL;
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
        {"expire-cache",  CLEANTYPE_EXPIRE_CACHE},
        {"all",           CLEANTYPE_ALL},
    };
    int nCount = sizeof(stCleanTypes)/sizeof(stCleanTypes[0]);
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
