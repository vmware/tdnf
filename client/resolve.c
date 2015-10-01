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
TDNFGetMatchingInstalledAndAvailable(
    PTDNF pTdnf,
    int nAlterType,
    const char* pszName,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    HyPackageList* phPkgListGoal
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgListGoal = NULL;
    HyPackageList hPkgsInstalled = NULL;
    HyPackageList hPkgsAvailable = NULL;

    if(!pTdnf || !pSolvedInfo || IsNullOrEmptyString(pszName) || !phPkgListGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetMatching(
                  pTdnf,
                  1,//Installed
                  pszName,
                  &hPkgsInstalled);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetMatching(
                  pTdnf,
                  0,//Available
                  pszName,
                  &hPkgsAvailable);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetGoalPackageList(
                  nAlterType,
                  hPkgsInstalled,
                  hPkgsAvailable,
                  &hPkgListGoal);
    BAIL_ON_TDNF_ERROR(dwError);

    if(hy_packagelist_count(hPkgsInstalled) > 0 && nAlterType == ALTER_INSTALL)
    {
        dwError = TDNFPopulatePkgInfos(
                      hPkgsInstalled,
                      &pSolvedInfo->pPkgsExisting);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *phPkgListGoal = hPkgListGoal;

cleanup:
    if(hPkgsInstalled)
    {
        hy_packagelist_free(hPkgsInstalled);
    }
    if(hPkgsAvailable)
    {
        hy_packagelist_free(hPkgsAvailable);
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
TDNFGetGoalPackageList(
    TDNF_ALTERTYPE nAlterType,
    HyPackageList hPkgsInstalled,
    HyPackageList hPkgsAvailable,
    HyPackageList* phPkgList
    )
{
    uint32_t dwError = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;
    int i = 0;

    hPkgList = hy_packagelist_create();
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(nAlterType == ALTER_INSTALL)
    {
        FOR_PACKAGELIST(hPkg, hPkgsAvailable, i)
        {
            if(!hy_packagelist_has(hPkgsInstalled, hPkg))
            {
                hy_packagelist_push(hPkgList, hPkg);
            }
        }
    }
    else if(nAlterType == ALTER_ERASE || nAlterType == ALTER_AUTOERASE)
    {
        FOR_PACKAGELIST(hPkg, hPkgsInstalled, i)
        {
            hy_packagelist_push(hPkgList, hPkg);
        }
    }
    else if(nAlterType == ALTER_UPGRADE || nAlterType == ALTER_DOWNGRADE)
    {
    }

    *phPkgList = hPkgList;

cleanup:
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
TDNFResolveAll(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;

    if(!pTdnf || !pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGoal(
                  pTdnf,
                  NULL,
                  NULL,
                  pSolvedPkgInfo->nAlterType,
                  pSolvedPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFResolvePackages(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;
    HyPackageList hPkgList = NULL;
    HyPackageList hPkgListGoal = NULL;
    HySelector hSelector = NULL;
    HyPackage hPkgTemp = NULL;
    const char* pszPkgName = NULL;
    int nAlterType = -1;

    if(!pTdnf || !pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nAlterType = pSolvedPkgInfo->nAlterType;

    //TODO: support multiple packages
    pszPkgName = pTdnf->pArgs->ppszCmds[1];

    //Check if package is installed before proceeding
    if(nAlterType == ALTER_ERASE)
    {
        dwError = TDNFFindInstalledPkgByName(
                      pTdnf->hSack,
                      pszPkgName,
                      &hPkgTemp);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_AUTOERASE)
    {
        dwError = TDNFGetInstalled(pTdnf->hSack, &hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFMatchForReinstall(
                      pTdnf->hSack,
                      pszPkgName,
                      &hPkgListGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFGetSelector(
                      pTdnf,
                      pszPkgName,
                      &hSelector);
        BAIL_ON_TDNF_ERROR(dwError);
        if(hSelector != NULL)
        {
            hPkgListGoal = hy_selector_matches(hSelector);
        }
        else
        {
            dwError = TDNFGetMatchingInstalledAndAvailable(
                          pTdnf,
                          nAlterType,
                          pszPkgName,
                          pSolvedPkgInfo,
                          &hPkgListGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    if(hy_packagelist_count(hPkgListGoal) > 0)
    {
        dwError = TDNFGoal(
                      pTdnf,
                      hPkgListGoal,
                      hSelector,
                      nAlterType,
                      pSolvedPkgInfo);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateMemory(
                      sizeof(TDNF_PKG_INFO),
                      (void**)&pSolvedPkgInfo->pPkgsNotAvailable);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(
                      pszPkgName,
                      &pSolvedPkgInfo->pPkgsNotAvailable->pszName);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgTemp)
    {
         hy_package_free(hPkgTemp);
    }
    if(hSelector)
    {
        hy_selector_free(hSelector);
    }
    if(hPkgListGoal)
    {
        hy_packagelist_free(hPkgListGoal);
    }
    TDNF_SAFE_FREE_PKGLIST(hPkgList);
    return dwError;

error:
    goto cleanup;
}
