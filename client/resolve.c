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
    Queue* qGoal
    )
{
    uint32_t dwError = 0;
    Queue qGlob;
    queue_init(&qGlob);

    PTDNF_CMD_ARGS pCmdArgs = NULL;
    int nCmdIndex = 0;
    int nPkgIndex = 0;
    char* pszPkgName = NULL;


    if(!pTdnf || !pTdnf->pArgs || !pSolvedPkgInfo || !qGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCmdArgs = pTdnf->pArgs;

    if(pSolvedPkgInfo->nAlterType == ALTER_DOWNGRADEALL)
    {
        dwError =  TDNFFilterPackages(
                        pTdnf, 
                        pSolvedPkgInfo,
                        qGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pSolvedPkgInfo->nAlterType == ALTER_AUTOERASE)
    {
        dwError =  TDNFFilterPackages(
                        pTdnf, 
                        pSolvedPkgInfo,
                        qGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(nCmdIndex = 1; nCmdIndex < pCmdArgs->nCmdCount; ++nCmdIndex)
    {
        pszPkgName = pCmdArgs->ppszCmds[nCmdIndex];
        if(TDNFIsGlob(pszPkgName))
        {
            queue_empty(&qGlob);
            dwError = TDNFGetGlobPackages(pTdnf, pszPkgName, &qGlob);
            BAIL_ON_TDNF_ERROR(dwError);

            nPkgIndex = 0;
            for(nPkgIndex = 0; nPkgIndex < qGlob.count; nPkgIndex++)
            {
                dwError = TDNFPrepareAndAddPkg(
                              pTdnf,
                              SolvGetPkgNameFromId(pTdnf->pSack, qGlob.elements[nPkgIndex]),
                              pSolvedPkgInfo,
                              qGoal);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            if(qGlob.count == 0)
            {
                dwError = TDNFAddNotResolved(pSolvedPkgInfo, pszPkgName);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            dwError = TDNFPrepareAndAddPkg(
                          pTdnf,
                          pszPkgName,
                          pSolvedPkgInfo,
                          qGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    queue_free(&qGlob);
    return dwError;

error:
    goto cleanup;
}

uint32_t TDNFFilterPackages(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* qGoal)
{
    uint32_t dwError = 0;
    Id installedId = 0;
    int pkgIndex = 0;
    PSolvPackageList pInstalledPkgList = NULL;

    if(!pTdnf || !qGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pInstalledPkgList = SolvCreatePackageList();
    if(!pInstalledPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = SolvFindAllInstalled(pTdnf->pSack, pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }

    for(pkgIndex = 0; pkgIndex < SolvGetPackageListSize(pInstalledPkgList); pkgIndex++)
    {
        SolvGetPackageId(pInstalledPkgList, pkgIndex, &installedId);
        dwError = TDNFPrepareAndAddPkg(
                      pTdnf,
                      SolvGetPkgNameFromId(pTdnf->pSack, installedId),
                      pSolvedPkgInfo,
                      qGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
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
    PTDNF pTdnf,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* qGoal
    )
{
    uint32_t dwError = 0;
    if( !pTdnf ||
        IsNullOrEmptyString(pszPkgName) ||
        !pSolvedPkgInfo ||
        !qGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPrepareSinglePkg(
                  pTdnf,
                  pszPkgName,
                  pSolvedPkgInfo,
                  qGoal);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFPrepareSinglePkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* qGoal
    )
{
    uint32_t dwError = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    int count = 0;
    int nAlterType = 0;

    if(!pTdnf
       || IsNullOrEmptyString(pszPkgName)
       || !pSolvedPkgInfo
       || !qGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nAlterType = pSolvedPkgInfo->nAlterType;

    //Check if this is a known package. If not add to unresolved
    dwError = SolvCountPkgByName(pTdnf->pSack, pszPkgName, &count);
    if(dwError || count == 0)
    {
        dwError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFMatchForReinstall(
                      pTdnf->pSack,
                      pszPkgName,
                      qGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_ERASE ||
        nAlterType == ALTER_AUTOERASE)
    {
        pInstalledPkgList = SolvCreatePackageList();
        if(!pInstalledPkgList)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        dwError = SolvFindInstalledPkgByName(
                      pTdnf->pSack,
                      pszPkgName,
                      pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAddPackagesForErase(
                      pTdnf,
                      qGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_INSTALL)
    {
        dwError = TDNFAddPackagesForInstall(
                      pTdnf,
                      qGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_UPGRADE)
    {
        dwError = TDNFAddPackagesForUpgrade(
                      pTdnf,
                      qGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_DOWNGRADE ||
             nAlterType == ALTER_DOWNGRADEALL)
    {
        dwError = TDNFAddPackagesForDowngrade(
                      pTdnf,
                      qGoal,
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

