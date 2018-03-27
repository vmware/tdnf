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
TDNFAddNotInstalled(
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0 ;
    int nIndex = 0;

    if(!pSolvedInfo ||
       !pSolvedInfo->ppszPkgsNotInstalled ||
       IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while(pSolvedInfo->ppszPkgsNotInstalled[nIndex++]);

    dwError = TDNFAllocateString(
                  pszPkgName,
                  &pSolvedInfo->ppszPkgsNotInstalled[--nIndex]);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

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
TDNFGetMatching(
    PTDNF pTdnf,
    int nSystem,
    const char* pszPkgName,
    HyPackageList* phPkgList
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgList = NULL;
    HyQuery hQuery = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszPkgName) || !phPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_FAILED;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_REPONAME,
                  nSystem == 1 ? HY_EQ : HY_NEQ,
                  HY_SYSTEM_REPO_NAME);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    dwError = hy_query_filter(
                  hQuery,
                  HY_PKG_NAME,
                  HY_EQ,
                  pszPkgName);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        dwError = HY_E_FAILED;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    *phPkgList = hPkgList;

cleanup:
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
    return dwError;

error:
    if(phPkgList)
    {
        *phPkgList = NULL;
    }
    TDNF_SAFE_FREE_PKGLIST(hPkgList);
    goto cleanup;
}

uint32_t
TDNFGetSelector(
    PTDNF pTdnf,
    const char* pszPkg,
    HySelector* phSelector
    )
{
    uint32_t dwError = 0;
    HyNevra hNevra = NULL;
    HyPossibilities hPoss = NULL;
    HySelector hSelector = NULL;
    HySubject hSubject = NULL;

    const char* pszName = NULL;
    const char* pszVersion = NULL;
    const char* pszRelease = NULL;
    const char* pszArch = NULL;
    char* pszEVR = NULL;
    int nEpoch = 0;

    if(!pTdnf || IsNullOrEmptyString(pszPkg) || !phSelector)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    
    hSubject = hy_subject_create(pszPkg);

    hPoss = hy_subject_nevra_possibilities_real(
                hSubject,
                NULL,
                pTdnf->hSack,
                0);
    if(hy_possibilities_next_nevra(hPoss, &hNevra) == -1)
    {
        //make sure that we reset on failure to avoid
        //a potential double free
        hNevra = NULL;

        dwError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszName = hy_nevra_get_string(hNevra, HY_NEVRA_NAME);
    pszVersion = hy_nevra_get_string(hNevra, HY_NEVRA_VERSION);
    pszRelease = hy_nevra_get_string(hNevra, HY_NEVRA_RELEASE);
    pszArch = hy_nevra_get_string(hNevra, HY_NEVRA_ARCH);
    nEpoch = hy_nevra_get_epoch(hNevra);

    hSelector = hy_selector_create(pTdnf->hSack);

    if(pszName)
    {
        dwError = hy_selector_set(
            hSelector,
            HY_PKG_NAME,
            TDNFIsGlob(pszName)? HY_GLOB:HY_EQ,
            pszName);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }
    if(pszVersion)
    {
        dwError = TDNFAllocateString(pszVersion, &pszEVR);
        BAIL_ON_TDNF_ERROR(dwError);

        if(nEpoch > 0)
        {
            if(pszRelease)
            {
                dwError = TDNFAllocateStringPrintf(
                              &pszEVR,
                              "%d:%s-%s",
                              nEpoch,
                              pszVersion,
                              pszRelease);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else
            {
                dwError = TDNFAllocateStringPrintf(
                              &pszEVR,
                              "%d:%s",
                              nEpoch,
                              pszVersion);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else if(pszRelease)
        {
            dwError = TDNFAllocateStringPrintf(
                          &pszEVR,
                          "%s-%s",
                          pszVersion,
                          pszRelease);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if(pszEVR)
        {
            dwError = hy_selector_set(hSelector, HY_PKG_EVR, HY_EQ, pszEVR);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
        }
        else
        {
            dwError = hy_selector_set(
                          hSelector,
                          HY_PKG_VERSION,
                          HY_EQ,
                          pszVersion);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
        }
    }
    if(pszArch)
    {
        dwError = hy_selector_set(hSelector, HY_PKG_ARCH, HY_EQ, pszArch);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    *phSelector = hSelector;

cleanup:
    if(hNevra)
    {
        hy_nevra_free(hNevra);
    }
    if(hPoss)
    {
        hy_possibilities_free(hPoss);
    }
    if(hSubject)
    {
        hy_subject_free(hSubject);
    }
    TDNF_SAFE_FREE_MEMORY(pszEVR);
    return dwError;

error:
    if(phSelector)
    {
        *phSelector = NULL;
    }
    if(hSelector)
    {
        hy_selector_free(hSelector);
    }
    goto cleanup;
}

uint32_t
TDNFPrepareAllPackages(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList* phPkgListGoal
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgListGoal = NULL;
    HyPackageList hPkgListGlob = NULL;
    HyPackage hPkgGlob = NULL;
    uint32_t dwSecutiry = 0;

    PTDNF_CMD_ARGS pCmdArgs = NULL;
    int nCmdIndex = 0;
    int nPkgIndex = 0;
    char* pszPkgName = NULL;

    char** ppszPkgArray = NULL;
    uint32_t dwCount = 0;

    if(!pTdnf || !pTdnf->pArgs || !pSolvedPkgInfo || !phPkgListGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCmdArgs = pTdnf->pArgs;
    hPkgListGoal = hy_packagelist_create();

    if(pSolvedPkgInfo->nAlterType == ALTER_DOWNGRADEALL)
    {
        dwError = TDNFAddFilteredPkgs(
                      pTdnf,
                      SCOPE_DOWNGRADES,
                      pSolvedPkgInfo,
                      hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pSolvedPkgInfo->nAlterType == ALTER_AUTOERASE)
    {
        dwError = TDNFAddFilteredPkgs(
                      pTdnf,
                      SCOPE_INSTALLED,
                      pSolvedPkgInfo,
                      hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCheckSecurityOption(pTdnf, &dwSecutiry);
    BAIL_ON_TDNF_ERROR(dwError);

    if((pSolvedPkgInfo->nAlterType == ALTER_UPGRADEALL ||
        pSolvedPkgInfo->nAlterType == ALTER_UPGRADE) &&
        dwSecutiry)
    {
        dwError = TdnfGetSecurityUpdatePkgs(pTdnf, &ppszPkgArray, &dwCount);
        BAIL_ON_TDNF_ERROR(dwError);
        for(nPkgIndex = 0; nPkgIndex < dwCount; ++nPkgIndex)
        {
            dwError = TDNFPrepareAndAddPkg(
                          pTdnf,
                          1,
                          ppszPkgArray[nPkgIndex],
                          pSolvedPkgInfo,
                          hPkgListGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    else
    {
        for(nCmdIndex = 1; nCmdIndex < pCmdArgs->nCmdCount; ++nCmdIndex)
        {
            pszPkgName = pCmdArgs->ppszCmds[nCmdIndex];
            if(TDNFIsGlob(pszPkgName))
            {
                dwError = TDNFGetGlobPackages(pTdnf, pszPkgName, &hPkgListGlob);
                BAIL_ON_TDNF_ERROR(dwError);

                nPkgIndex = 0;
                FOR_PACKAGELIST(hPkgGlob, hPkgListGlob, nPkgIndex)
                {
                    dwError = TDNFPrepareAndAddPkg(
                                  pTdnf,
                                  1,
                                  hy_package_get_name(hPkgGlob),
                                  pSolvedPkgInfo,
                                  hPkgListGoal);
                    BAIL_ON_TDNF_ERROR(dwError);
                }
                if(nPkgIndex == 0)
                {
                    dwError = TDNFAddNotResolved(pSolvedPkgInfo, pszPkgName);
                    BAIL_ON_TDNF_ERROR(dwError);
                }
            }
            else
            {
                dwError = TDNFPrepareAndAddPkg(
                              pTdnf,
                              0,
                              pszPkgName,
                              pSolvedPkgInfo,
                              hPkgListGoal);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }
    *phPkgListGoal = hPkgListGoal;

cleanup:
    if(ppszPkgArray)
    {
        TDNFFreeStringArray(ppszPkgArray);
    }
    if(hPkgListGlob)
    {
        hy_packagelist_free(hPkgListGlob);
    }
    return dwError;

error:
    if(phPkgListGoal)
    {
        *phPkgListGoal = NULL;
    }
    if(hPkgListGoal)
    {
        hy_packagelist_free(hPkgListGoal);
    }
    goto cleanup;
}

uint32_t
TDNFAddFilteredPkgs(
    PTDNF pTdnf,
    int nScope,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList hPkgListGoal
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;
    int nPkgIndex = 0;

    if(!pTdnf || !pSolvedPkgInfo || !hPkgListGoal || nScope == SCOPE_NONE)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    
    dwError = TDNFFilterPackages(pTdnf, nScope, &hPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    nPkgIndex = 0;
    FOR_PACKAGELIST(hPkg, hPkgList, nPkgIndex)
    {
        dwError = TDNFPrepareAndAddPkg(
                      pTdnf,
                      0,
                      hy_package_get_name(hPkg),
                      pSolvedPkgInfo,
                      hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFPrepareAndAddPkg(
    PTDNF pTdnf,
    int nIsGlobExpanded,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList hPkgListGoal
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;
    int nPkgIndex = 0;

    if( !pTdnf ||
        IsNullOrEmptyString(pszPkgName) ||
        !pSolvedPkgInfo ||
        !hPkgListGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPrepareSinglePkg(
                  pTdnf,
                  nIsGlobExpanded,
                  pszPkgName,
                  pSolvedPkgInfo,
                  &hPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    if(hPkgList)
    {
        FOR_PACKAGELIST(hPkg, hPkgList, nPkgIndex)
        {
            hy_packagelist_push(hPkgListGoal, hPkg);
        }
    }

cleanup:
    //do not free hPkgList as it is shallow copied to hPkgListGoal
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFPrepareSinglePkg(
    PTDNF pTdnf,
    int nIsGlobExpanded,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList* phPkgListGoal
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgListGoal = NULL;
    HyPackageList hPkgListGoalTemp = NULL;
    HyPackage hPkgTemp = NULL;
    HyPackage hPkg = NULL;
    HySelector hSelector = NULL;
    int i = 0;
    int nAlterType = 0;

    if(!pTdnf
       || IsNullOrEmptyString(pszPkgName)
       || !pSolvedPkgInfo
       || !phPkgListGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nAlterType = pSolvedPkgInfo->nAlterType;

    //Check if this is a known package. If not add to unresolved
    dwError = TDNFGetSelector(
                  pTdnf,
                  pszPkgName,
                  &hSelector);
    BAIL_ON_TDNF_ERROR(dwError);

    //Check if package is installed before proceeding
    if(nAlterType == ALTER_ERASE)
    {
        dwError = TDNFFindInstalledPkgByName(
                      pTdnf->hSack,
                      pszPkgName,
                      &hPkgTemp);
        if(dwError == ERROR_TDNF_NO_MATCH)
        {
            dwError = ERROR_TDNF_ERASE_NEEDS_INSTALL;
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFMatchForReinstall(
                      pTdnf->hSack,
                      pszPkgName,
                      &hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(hSelector != NULL)
    {
        hPkgListGoalTemp = hy_selector_matches(hSelector);
        hPkgListGoal = hy_packagelist_create();

        if(nAlterType == ALTER_ERASE)
        {
            FOR_PACKAGELIST(hPkg, hPkgListGoalTemp, i)
            {
                if(hy_package_installed(hPkg))
                {
                    hy_packagelist_push(hPkgListGoal, hPkg);
                    break;
                }
            }
        }
        else if (nAlterType == ALTER_INSTALL)
        {
            dwError = TDNFAddPackagesForInstall(
                          hPkgListGoalTemp,
                          hPkgListGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (nAlterType == ALTER_UPGRADE)
        {
            dwError = TDNFAddPackagesForUpgrade(
                          pTdnf->hSack,
                          hPkgListGoalTemp,
                          hPkgListGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (nAlterType == ALTER_DOWNGRADE ||
                 nAlterType == ALTER_DOWNGRADEALL)
        {
            dwError = TDNFAddPackagesForDowngrade(
                          pTdnf->hSack,
                          hPkgListGoalTemp,
                          hPkgListGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else
    {
        //TODO: Shouldnt be here. Better error needed
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *phPkgListGoal = hPkgListGoal;

cleanup:
    if(hPkgTemp)
    {
    //     hy_package_free(hPkgTemp);
    }
    if(hPkgListGoalTemp)
    {
//        hy_packagelist_free(hPkgListGoalTemp);
    }
    if(hSelector)
    {
        hy_selector_free(hSelector);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_ALREADY_INSTALLED)
    {
        int nShowAlreadyInstalled = 1;
        //dont show already installed errors in the check path
        if(pTdnf && pTdnf->pArgs)
        {
            if(!strcmp(pTdnf->pArgs->ppszCmds[0], "check"))
            {
                nShowAlreadyInstalled = 0;
            }
        }
        dwError = 0;
        if(nShowAlreadyInstalled)
        {
            fprintf(stderr, "Package %s is already installed.\n", pszPkgName);
        }
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
    if(dwError == ERROR_TDNF_ERASE_NEEDS_INSTALL)
    {
        dwError = 0;
        if(!nIsGlobExpanded && TDNFAddNotInstalled(pSolvedPkgInfo, pszPkgName))
        {
            fprintf(stderr, "Error while adding not installed packages\n");
        }
    }
    if(phPkgListGoal)
    {
        *phPkgListGoal = NULL;
    }
    if(hPkgListGoal)
    {
        hy_packagelist_free(hPkgListGoal);
    }
    goto cleanup;
}

