/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : resolve.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#include "includes.h"

uint32_t
TDNFAddNotResolved(
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0 ;
    int nIndex = 0;

    if(!pSolvedInfo ||
       !pSolvedInfo->ppszPkgsNotResolved ||
       IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while(pSolvedInfo->ppszPkgsNotResolved[nIndex++]);

    dwError = TDNFAllocateString(
                  pszPkgName,
                  &pSolvedInfo->ppszPkgsNotResolved[--nIndex]);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPrepareAllPackages(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* queueGoal
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    int nCmdIndex = 0;
    int nPkgIndex = 0;
    char* pszPkgName = NULL;
    char* pszName = NULL;
    Queue queueLocal = {0};

    if(!pTdnf || !pTdnf->pSack ||
       !pTdnf->pArgs || !pSolvedPkgInfo || !queueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_init(&queueLocal);
    pCmdArgs = pTdnf->pArgs;

    if(pSolvedPkgInfo->nAlterType == ALTER_DOWNGRADEALL || 
       pSolvedPkgInfo->nAlterType == ALTER_AUTOERASE)
    {
        dwError =  TDNFFilterPackages(
                       pTdnf->pSack, 
                       pSolvedPkgInfo,
                       queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(nCmdIndex = 1; nCmdIndex < pCmdArgs->nCmdCount; ++nCmdIndex)
    {
        pszPkgName = pCmdArgs->ppszCmds[nCmdIndex];
        if(TDNFIsGlob(pszPkgName))
        {
            queue_empty(&queueLocal);
            dwError = TDNFGetGlobPackages(
                          pTdnf->pSack,
                          pszPkgName,
                          &queueLocal);
            BAIL_ON_TDNF_ERROR(dwError);

            nPkgIndex = 0;
            for(nPkgIndex = 0; nPkgIndex < queueLocal.count; nPkgIndex++)
            {
                dwError = SolvGetPkgNameFromId(
                              pTdnf->pSack,
                              queueLocal.elements[nPkgIndex],
                              &pszName);
                BAIL_ON_TDNF_ERROR(dwError);

                dwError = TDNFPrepareAndAddPkg(
                              pTdnf->pSack,
                              pszName,
                              pSolvedPkgInfo,
                              queueGoal);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            if(queueLocal.count == 0)
            {
                dwError = TDNFAddNotResolved(pSolvedPkgInfo, pszPkgName);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            dwError = TDNFPrepareAndAddPkg(
                          pTdnf->pSack,
                          pszPkgName,
                          pSolvedPkgInfo,
                          queueGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    queue_free(&queueLocal);
    return dwError;

error:
    goto cleanup;
}

uint32_t TDNFFilterPackages(
    PSolvSack pSack,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* queueGoal)
{
    uint32_t dwError = 0;
    Id dwInstalledId = 0;
    uint32_t dwPkgIndex = 0;
    uint32_t dwSize = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    char* pszName = NULL;

    if(!pSack || !queueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvCreatePackageList(&pInstalledPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvFindAllInstalled(pSack, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }

    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwSize);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwSize; dwPkgIndex++)
    {
        SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &dwInstalledId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pSack,
                      dwInstalledId,
                      &pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFPrepareAndAddPkg(
                      pSack,
                      pszName,
                      pSolvedPkgInfo,
                      queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
        TDNF_SAFE_FREE_MEMORY(pszName);
        pszName = NULL;
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPrepareAndAddPkg(
    PSolvSack pSack,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* queueGoal
    )
{
    uint32_t dwError = 0;
    if( !pSack ||
        IsNullOrEmptyString(pszPkgName) ||
        !pSolvedPkgInfo ||
        !queueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPrepareSinglePkg(
                  pSack,
                  pszPkgName,
                  pSolvedPkgInfo,
                  queueGoal);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFPrepareSinglePkg(
    PSolvSack pSack,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* queueGoal
    )
{
    uint32_t dwError = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    uint32_t dwCount = 0;
    int nAlterType = 0;

    if(!pSack ||
       IsNullOrEmptyString(pszPkgName) ||
       !pSolvedPkgInfo ||
       !queueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nAlterType = pSolvedPkgInfo->nAlterType;

    //Check if this is a known package. If not add to unresolved
    dwError = SolvCountPkgByName(pSack, pszPkgName, &dwCount);
    if(dwError || dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFMatchForReinstall(
                      pSack,
                      pszPkgName,
                      queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_ERASE ||
        nAlterType == ALTER_AUTOERASE)
    {
        dwError = SolvCreatePackageList(&pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvFindInstalledPkgByName(
                      pSack,
                      pszPkgName,
                      pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAddPackagesForErase(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_INSTALL)
    {
        dwError = TDNFAddPackagesForInstall(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_UPGRADE)
    {
        dwError = TDNFAddPackagesForUpgrade(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_DOWNGRADE ||
             nAlterType == ALTER_DOWNGRADEALL)
    {
        dwError = TDNFAddPackagesForDowngrade(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_ALREADY_INSTALLED)
    {
        dwError = 0;
        fprintf(stderr, "Package %s is already installed.\n", pszPkgName);
    }
    if(dwError == ERROR_TDNF_NO_UPGRADE_PATH)
    {
        dwError = 0;
        fprintf(stderr, "There is no upgrade path for %s.\n", pszPkgName);
    }
    if(dwError == ERROR_TDNF_NO_DOWNGRADE_PATH)
    {
        dwError = 0;
        fprintf(stderr, "There is no downgrade path for %s.\n", pszPkgName);
    }
    if(dwError == ERROR_TDNF_NO_SEARCH_RESULTS)
    {
        dwError = 0;
        if(TDNFAddNotResolved(pSolvedPkgInfo, pszPkgName))
        {
            fprintf(stderr, "Error while adding not resolved packages\n");
        }
    }
    goto cleanup;
}

