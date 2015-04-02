/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : updateinfocmd.c
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
char*
_TDNFGetUpdateInfoType(
    int nType
    );

static
uint32_t
TDNFCliUpdateInfoList(
    PTDNF_UPDATEINFO pUpdateInfo
    );

uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    );

char*
_TDNFGetUpdateInfoType(
    int nType
    )
{
    char* pszType = "Unknown";

    switch(nType)
    {
        case UPDATE_SECURITY:
            pszType = "Security";
            break; 
        case UPDATE_BUGFIX:
            pszType = "Bugfix";
            break; 
        case UPDATE_ENHANCEMENT:
            pszType = "Enhancement";
            break; 
    }

    return pszType;
}

uint32_t
TDNFCliUpdateInfoCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
  
    PTDNF_UPDATEINFO pUpdateInfo = NULL;
    PTDNF_UPDATEINFO_ARGS pInfoArgs = NULL;

    dwError = TDNFCliParseUpdateInfoArgs(pCmdArgs, &pInfoArgs);
    BAIL_ON_CLI_ERROR(dwError);

    if(pInfoArgs->nMode == OUTPUT_SUMMARY)
    {
        dwError = TDNFCliUpdateInfoSummary(pTdnf, pCmdArgs, pInfoArgs);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else
    {
        dwError = TDNFUpdateInfo(
                     pTdnf,
                     pInfoArgs->nMode,
                     pInfoArgs->nScope,
                     pInfoArgs->ppszPackageNameSpecs,
                     &pUpdateInfo);
        BAIL_ON_CLI_ERROR(dwError);
        if(pInfoArgs->nMode == OUTPUT_LIST)
        {
            dwError = TDNFCliUpdateInfoList(pUpdateInfo);
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
cleanup:
    if(pInfoArgs)
    {
        TDNFFreeUpdateInfoArgs(pInfoArgs);
    }
    TDNFFreeUpdateInfo(pUpdateInfo);
    return dwError;

error:
    goto cleanup;
}


uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    )
{
    uint32_t dwError = 0;
    int i = 0;
    PTDNF_UPDATEINFO_SUMMARY pSummary = NULL;

    if(!pTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFUpdateInfoSummary(
                  pTdnf,
                  AVAIL_AVAILABLE,
                  pInfoArgs->ppszPackageNameSpecs,
                  &pSummary);
    BAIL_ON_CLI_ERROR(dwError);

    for(i = UPDATE_UNKNOWN; i <= UPDATE_ENHANCEMENT; ++i)
    {
        if(pSummary[i].nCount > 0)
        {
            printf(
                "%d %s notice(s)\n",
                pSummary[i].nCount,
                _TDNFGetUpdateInfoType(pSummary[i].nType));
        }
    }

cleanup:
    if(pSummary)
    {
        TDNFFreeUpdateInfoSummary(pSummary);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliUpdateInfoList(
    PTDNF_UPDATEINFO pInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;

    if(!pInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while(pInfo)
    {
        pPkg = pInfo->pPackages;
        while(pPkg)
        {
            fprintf(stdout, "%s %s %s\n",
                pInfo->pszID,
                _TDNFGetUpdateInfoType(pInfo->nType),
                pPkg->pszFileName);
            
            pPkg = pPkg->pNext;
        }
        pInfo = pInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
