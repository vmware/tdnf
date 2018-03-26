/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
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


char*
TDNFGetUpdateInfoType(
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
    PTDNF_CLI_CONTEXT pContext,
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
        dwError = TDNFCliUpdateInfoSummary(pContext, pCmdArgs, pInfoArgs);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else
    {
        dwError = pContext->pFnUpdateInfo(
                      pContext,
                      pInfoArgs,
                      &pUpdateInfo);
        BAIL_ON_CLI_ERROR(dwError);
        if(pInfoArgs->nMode == OUTPUT_LIST)
        {
            dwError = TDNFCliUpdateInfoList(pUpdateInfo);
            BAIL_ON_CLI_ERROR(dwError);
        }
        else if(pInfoArgs->nMode == OUTPUT_INFO)
        {
            dwError = TDNFCliUpdateInfoInfo(pUpdateInfo);
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
cleanup:
    if(pInfoArgs)
    {
        TDNFCliFreeUpdateInfoArgs(pInfoArgs);
    }
    TDNFFreeUpdateInfo(pUpdateInfo);
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}


uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    )
{
    uint32_t dwError = 0;
    int i = 0;
    PTDNF_UPDATEINFO_SUMMARY pSummary = NULL;

    if(!pContext || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = pContext->pFnUpdateInfoSummary(
                  pContext,
                  AVAIL_AVAILABLE,
                  pInfoArgs,
                  &pSummary);
    BAIL_ON_CLI_ERROR(dwError);

    for(i = UPDATE_UNKNOWN; i <= UPDATE_ENHANCEMENT; ++i)
    {
        if(pSummary[i].nCount > 0)
        {
            printf(
                "%d %s notice(s)\n",
                pSummary[i].nCount,
                TDNFGetUpdateInfoType(pSummary[i].nType));
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
                TDNFGetUpdateInfoType(pInfo->nType),
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

uint32_t
TDNFCliUpdateInfoInfo(
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
            fprintf(stdout, "       Name : %s\n",
                pPkg->pszName);
            fprintf(stdout, "  File Name : %s\n",
                pPkg->pszFileName);
            fprintf(stdout, "  Update ID : %s\n",
                pInfo->pszID);
            fprintf(stdout, "       Type : %s\n",
                TDNFGetUpdateInfoType(pInfo->nType));
            if(pInfo->nType == UPDATE_SECURITY)
            {
                if(pInfo->pszSeverity)
                {
                    fprintf(stdout, "   Severity : %s\n", pInfo->pszSeverity);
                }
            }
            fprintf(stdout, "    Updated : %s\n",
                pInfo->pszDate);
            fprintf(stdout, "Description : %s\n",
                pInfo->pszDescription);

            pPkg = pPkg->pNext;
        }
        pInfo = pInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
NumSecurityUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    if(!pInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }
    while(pInfo)
    {
        if(pInfo->nType == UPDATE_SECURITY)
        {
            pPkg = pInfo->pPackages;
            while(pPkg)
            {
                dwCount++;
                pPkg = pPkg->pNext;
            }
        }
        pInfo = pInfo->pNext;
    }
    *pdwCount = dwCount;
cleanup:
    return dwError;

error:
    if(pdwCount)
    {
        *pdwCount = 0;
    }
    goto cleanup;
}

uint32_t
NumHigherSeverityUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    double dblSeverity,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    if(!pInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }
    while(pInfo)
    {
        if(pInfo->nType == UPDATE_SECURITY)
        {
            if(pInfo->pszSeverity && atof(pInfo->pszSeverity) >= dblSeverity)
            {
                pPkg = pInfo->pPackages;
                while(pPkg)
                {
                    dwCount++;
                    pPkg = pPkg->pNext;
                }
            }
        }
        pInfo = pInfo->pNext;
    }
    *pdwCount = dwCount;
cleanup:
    return dwError;

error:
    if(pdwCount)
    {
        *pdwCount = 0;
    }
    goto cleanup;
}

uint32_t
GetSecurityUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    char*** pppszPkgs,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    char**   ppszPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    int nIndex = 0;
    if(!pInfo || !pdwCount || !pppszPkgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }
    dwError = NumSecurityUpdatePkgs(pInfo, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  dwCount + 1,
                  sizeof(char*),
                  (void**)&ppszPkgs);
    BAIL_ON_CLI_ERROR(dwError);

    while(pInfo)
    {
        if(pInfo->nType == UPDATE_SECURITY)
        {
            pPkg = pInfo->pPackages;
            while(pPkg)
            {
                dwError = TDNFAllocateString(
                              pInfo->pszID,
                              &ppszPkgs[nIndex++]);
                BAIL_ON_CLI_ERROR(dwError);
                pPkg = pPkg->pNext;
            }
        }
        pInfo = pInfo->pNext;
    }
    *pppszPkgs = ppszPkgs;
    *pdwCount  = dwCount;
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
GetHigherSeverityUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    double dblSeverity,
    char*** pppszPkgs,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    char**   ppszPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    int nIndex = 0;
    if(!pInfo || !pdwCount || !pppszPkgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }
    dwError = NumHigherSeverityUpdatePkgs(pInfo, dblSeverity, &dwCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  dwCount + 1,
                  sizeof(char*),
                  (void**)&ppszPkgs);
    BAIL_ON_CLI_ERROR(dwError);

    while(pInfo)
    {
        if(pInfo->nType == UPDATE_SECURITY)
        {
            if(pInfo->pszSeverity && atof(pInfo->pszSeverity) >= dblSeverity)
            {
                pPkg = pInfo->pPackages;
                while(pPkg)
                {
                    dwCount++;
                    pPkg = pPkg->pNext;
                }
            }
        }
        pInfo = pInfo->pNext;
    }
    while(pInfo)
    {
        if(pInfo->nType == UPDATE_SECURITY)
        {
            if(pInfo->pszSeverity && atof(pInfo->pszSeverity) >= dblSeverity)
            {
                pPkg = pInfo->pPackages;
                while(pPkg)
                {
                    dwError = TDNFAllocateString(
                                  pInfo->pszID,
                                  &ppszPkgs[nIndex++]);
                    BAIL_ON_CLI_ERROR(dwError);
                    pPkg = pPkg->pNext;
                }
            }
        }
        pInfo = pInfo->pNext;
    }
    *pppszPkgs = ppszPkgs;
    *pdwCount  = dwCount;
cleanup:
    return dwError;

error:
    goto cleanup;
}
