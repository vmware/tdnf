/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : rpmtrans.c
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
TDNFRpmExecTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    )
{
    uint32_t dwError = 0;
    int nKeepCachedRpms = 0;
    TDNFRPMTS ts = {0};

    if(!pTdnf || !pTdnf->pConf || !pSolvedInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nKeepCachedRpms = pTdnf->pConf->nKeepCache;

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_CACHED_RPM_LIST),
                  (void**)&ts.pCachedRpmsArray);

    if(!ts.pCachedRpmsArray)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    rpmSetVerbosity(TDNFConfGetRpmVerbosity(pTdnf));

    //Allow downgrades
    ts.nProbFilterFlags = RPMPROB_FILTER_OLDPACKAGE;
    if(pSolvedInfo->nAlterType == ALTER_REINSTALL)
    {
        ts.nProbFilterFlags = ts.nProbFilterFlags | RPMPROB_FILTER_REPLACEPKG;
    }

    ts.pTS = rpmtsCreate();
    if(!ts.pTS)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    ts.pKeyring = rpmKeyringNew();
    if(!ts.pKeyring)
    {
        dwError = ERROR_TDNF_RPMTS_KEYRING_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ts.nTransFlags = rpmtsSetFlags (ts.pTS, RPMTRANS_FLAG_NONE);

    if(rpmtsSetRootDir (ts.pTS, pTdnf->pArgs->pszInstallRoot))
    {
        dwError = ERROR_TDNF_RPMTS_BAD_ROOT_DIR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(rpmtsSetNotifyCallback(ts.pTS, TDNFRpmCB, (void*)&ts))
    {
        dwError = ERROR_TDNF_RPMTS_SET_CB_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPopulateTransaction(&ts, pTdnf, pSolvedInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRunTransaction(&ts, pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(ts.pTS)
    {
        rpmtsCloseDB(ts.pTS);
        rpmtsFree(ts.pTS);
    }
    if(ts.pKeyring)
    {
        rpmKeyringFree(ts.pKeyring);
    }
    if(ts.pCachedRpmsArray)
    {
        if(!nKeepCachedRpms)
        {
            TDNFRemoveCachedRpms(ts.pCachedRpmsArray);
        }
        TDNFFreeCachedRpmsArray(ts.pCachedRpmsArray);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPopulateTransaction(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    )
{
    uint32_t dwError = 0;

    if(pSolvedInfo->pPkgsToInstall)
    {
        dwError = TDNFTransAddInstallPkgs(
                      pTS,
                      pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToReinstall)
    {
        dwError = TDNFTransAddReInstallPkgs(
                      pTS,
                      pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToUpgrade)
    {
        dwError = TDNFTransAddUpgradePkgs(
                      pTS,
                      pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToRemove)
    {
        dwError = TDNFTransAddErasePkgs(
                      pTS,
                      pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsObsoleted)
    {
        dwError = TDNFTransAddObsoletedPkgs(
                      pTS,
                      pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToDowngrade)
    {
        dwError = TDNFTransAddDowngradePkgs(
                      pTS,
                      pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

int
doCheck(PTDNFRPMTS pTS)
{
    int nResult = 0;
    rpmpsi psi = NULL;
    rpmProblem prob = NULL;
    nResult = rpmtsCheck(pTS->pTS);

    rpmps ps = rpmtsProblems(pTS->pTS);
    if(ps)
    {
        int nProbs = rpmpsNumProblems(ps);
        if(nProbs > 0)
        {
            fprintf(stderr, "Found %d problems\n", nProbs);

            psi = rpmpsInitIterator(ps);
            while(rpmpsNextIterator(psi) >= 0)
            {
                prob = rpmpsGetProblem(psi);
                fprintf(stderr, "%s\n", rpmProblemString(prob));
                rpmProblemFree(prob);
            }
            rpmpsFreeIterator(psi);
            nResult = ERROR_TDNF_RPM_CHECK;
        }
    }
    return nResult;
}

uint32_t
TDNFRunTransaction(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int nSilent = 0;

    if(!pTS || !pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSilent = pTdnf->pArgs->nNoOutput;

    dwError = rpmtsOrder(pTS->pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = doCheck(pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    rpmtsClean(pTS->pTS);

    //TODO do callbacks for output
    if(!nSilent)
    {
        fprintf(stdout, "Testing transaction\n");
    }

    rpmtsSetFlags(pTS->pTS, RPMTRANS_FLAG_TEST);
    dwError = rpmtsRun(pTS->pTS, NULL, pTS->nProbFilterFlags);
    BAIL_ON_TDNF_ERROR(dwError);

    //TODO do callbacks for output
    if(!nSilent)
    {
        fprintf(stdout, "Running transaction\n");
    }
    rpmtsSetFlags(pTS->pTS, RPMTRANS_FLAG_NONE);
    dwError = rpmtsRun(pTS->pTS, NULL, pTS->nProbFilterFlags);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    if(pTS)
    {
        doCheck(pTS);
    }
    goto cleanup;
}

uint32_t
TDNFTransAddInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;

    hPkgList = hy_goal_list_installs(pTdnf->hGoal);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for(i = 0; (hPkg = hy_packagelist_get(hPkgList, i)) != NULL; ++i)
    {
        dwError = TDNFTransAddInstallPkg(pTS, pTdnf, hPkg, 0);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFTransAddReInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;

    hPkgList = hy_goal_list_reinstalls(pTdnf->hGoal);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for(i = 0; (hPkg = hy_packagelist_get(hPkgList, i)) != NULL; ++i)
    {
        dwError = TDNFTransAddInstallPkg(pTS, pTdnf, hPkg, 0);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFTransAddInstallPkg(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    HyPackage hPkg,
    int nUpgrade
    )
{
    uint32_t dwError = 0;
    int nGPGCheck = 0;
    char* pszRpmCacheDir = NULL;
    char* pszFilePath = NULL;
    char* pszFilePathCopy = NULL;
    const char* pszRepoName = NULL;
    char* pszHyName = NULL;
    Header rpmHeader = NULL;
    FD_t fp = NULL;
    char* pszDownloadCacheDir = NULL;
    char* pszUrlGPGKey = NULL;
    PTDNF_CACHED_RPM_ENTRY pRpmCache = NULL;

    pszRepoName = hy_package_get_reponame(hPkg);
    pszHyName = hy_package_get_location(hPkg);

    dwError = TDNFAllocateStringPrintf(
                  &pszRpmCacheDir,
                  "%s/%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pszRepoName,
                  "rpms");
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(
                  &pszFilePath,
                  "%s/%s",
                  pszRpmCacheDir,
                  pszHyName);
    BAIL_ON_TDNF_ERROR(dwError);

    // dirname() may modify the contents of path, so it may be desirable to
    // pass a copy when calling this function.
    dwError = TDNFAllocateString(pszFilePath, &pszFilePathCopy);
    BAIL_ON_TDNF_ERROR(dwError);
    pszDownloadCacheDir = dirname(pszFilePathCopy);
    if(!pszDownloadCacheDir)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    if(access(pszDownloadCacheDir, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
        } 
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(access(pszFilePath, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        dwError = TDNFDownloadPackage(pTdnf, hPkg, pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    //A download could have been triggered.
    //So check access and bail if not available
    if(access(pszFilePath, F_OK))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    //Check override, then repo config and launch
    //gpg check if needed
    dwError = TDNFGetGPGCheck(pTdnf, pszRepoName, &nGPGCheck, &pszUrlGPGKey);
    BAIL_ON_TDNF_ERROR(dwError);
    if(nGPGCheck)
    {
        dwError = TDNFGPGCheck(pTS->pKeyring, pszUrlGPGKey, pszFilePath);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fp = Fopen (pszFilePath, "r.ufdio");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = rpmReadPackageFile(
                     pTS->pTS,
                     fp,
                     pszFilePath,
                     &rpmHeader);
    //If not checking gpg sigs, ignore signature errors
    if(!nGPGCheck && (dwError == RPMRC_NOTTRUSTED || dwError == RPMRC_NOKEY))
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_RPM_ERROR(dwError);

    dwError = rpmtsAddInstallElement(
                   pTS->pTS,
                   rpmHeader,
                   (fnpyKey)pszFilePath,
                   nUpgrade,
                   NULL);
    BAIL_ON_TDNF_RPM_ERROR(dwError);

    if(pTS->pCachedRpmsArray)
    {
        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_CACHED_RPM_ENTRY),
                      (void**)&pRpmCache);
        BAIL_ON_TDNF_ERROR(dwError);
        pRpmCache->pszFilePath = pszFilePath;
        pRpmCache->pNext = pTS->pCachedRpmsArray->pHead;
        pTS->pCachedRpmsArray->pHead = pRpmCache;
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePathCopy);
    TDNF_SAFE_FREE_MEMORY(pszUrlGPGKey);
    if(pszHyName)
    {
        hy_free(pszHyName);
    }
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    if(fp)
    {
        Fclose(fp);
    }
    if(rpmHeader)
    {
        headerFree(rpmHeader);
    }
    if(!pTS->pCachedRpmsArray && dwError == 0)
    {
        TDNF_SAFE_FREE_MEMORY(pszFilePath);
    }
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pRpmCache);
    goto cleanup;
}

uint32_t
TDNFTransAddUpgradePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;

    hPkgList = hy_goal_list_upgrades(pTdnf->hGoal);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for(i = 0; (hPkg = hy_packagelist_get(hPkgList, i)) != NULL; ++i)
    {
        dwError = TDNFTransAddInstallPkg(pTS, pTdnf, hPkg, 1);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFTransAddErasePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;

    hPkgList = hy_goal_list_erasures(pTdnf->hGoal);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for(i = 0; (hPkg = hy_packagelist_get(hPkgList, i)) != NULL; ++i)
    {
        dwError = TDNFTransAddErasePkg(pTS, hPkg);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFTransAddObsoletedPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;

    hPkgList = hy_goal_list_obsoleted(pTdnf->hGoal);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for(i = 0; (hPkg = hy_packagelist_get(hPkgList, i)) != NULL; ++i)
    {
        dwError = TDNFTransAddErasePkg(pTS, hPkg);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFTransAddDowngradePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int i = 0;
    HyPackage hPkg = NULL;
    HyPackageList hPkgList = NULL;
    HyPackage hInstalledPkg = NULL;

    hPkgList = hy_goal_list_downgrades(pTdnf->hGoal);
    if(!hPkgList)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    for(i = 0; (hPkg = hy_packagelist_get(hPkgList, i)) != NULL; ++i)
    {
        dwError = TDNFTransAddInstallPkg(pTS, pTdnf, hPkg, 0);
        BAIL_ON_TDNF_ERROR(dwError);

        //Downgrade is a removal of existing and installing old.
        const char* pszName = NULL;
        pszName = hy_package_get_name(hPkg);
        if(IsNullOrEmptyString(pszName))
        {
            dwError = hy_get_errno();
            if(dwError == 0)
            {
                dwError = HY_E_FAILED;
            }
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
        }
        dwError = TDNFFindInstalledPkgByName(pTdnf->hSack, pszName, &hInstalledPkg);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFTransAddErasePkg(pTS, hInstalledPkg);
        BAIL_ON_TDNF_ERROR(dwError);

        hy_package_free(hInstalledPkg);
        hInstalledPkg = NULL;
    }

cleanup:
    if(hInstalledPkg)
    {
        hy_package_free(hInstalledPkg);
    }
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    goto cleanup;
}


uint32_t
TDNFTransAddErasePkg(
    PTDNFRPMTS pTS,
    HyPackage hPkg
    )
{
    uint32_t dwError = 0;
    Header pRpmHeader = NULL;
    rpmdbMatchIterator pIterator = NULL;
    const char* pszName = NULL;
    unsigned int nOffset = 0;

    if(!pTS || !hPkg)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszName = hy_package_get_name(hPkg);

    pIterator = rpmtsInitIterator(pTS->pTS, (rpmTag)RPMDBI_LABEL, pszName, 0);
    while ((pRpmHeader = rpmdbNextIterator(pIterator)) != NULL)
    {
        nOffset = rpmdbGetIteratorOffset(pIterator);
        if(nOffset)
        {
            dwError = rpmtsAddEraseElement(pTS->pTS, pRpmHeader, nOffset);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    if(pIterator)
    {
        rpmdbFreeIterator(pIterator);
    }
    return dwError;

error:
    goto cleanup;
}



void*
TDNFRpmCB(
     const void* pArg,
     const rpmCallbackType what,
     const rpm_loff_t amount,
     const rpm_loff_t total,
     fnpyKey key,
     rpmCallbackData data
     )
{
    Header pPkgHeader = (Header) pArg;
    void* pResult = NULL;
    char* pszFileName = (char*)key;
    PTDNFRPMTS pTS = (PTDNFRPMTS)data;

    switch (what)
    {
        case RPMCALLBACK_INST_OPEN_FILE:
            if(IsNullOrEmptyString(pszFileName))
            {
                return NULL;
            }
            pTS->pFD = Fopen(pszFileName, "r.ufdio");
            return (void *)pTS->pFD;
            break;

        case RPMCALLBACK_INST_CLOSE_FILE:
            if(pTS->pFD)
            {
                Fclose(pTS->pFD);
                pTS->pFD = NULL;
            }
            break;
        case RPMCALLBACK_INST_START:
        case RPMCALLBACK_UNINST_START:
            if(what == RPMCALLBACK_INST_START)
            {
                fprintf(stdout, "%s", "Installing/Updating: ");
            }
            else
            {
                fprintf(stdout, "%s", "Removing: ");
            }
            {
                char* pszNevra = headerGetAsString(pPkgHeader, RPMTAG_NEVRA);
                fprintf(stdout, "%s\n", pszNevra);
                free(pszNevra);
                (void)fflush(stdout);
            }
            break;
        default:
            break;
    }

    return pResult;
}

uint32_t
TDNFRemoveCachedRpms(
    PTDNF_CACHED_RPM_LIST pCachedRpmsList
    )
{
    uint32_t dwError = 0;
    PTDNF_CACHED_RPM_ENTRY pCur = NULL;

    if(!pCachedRpmsList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pCur = pCachedRpmsList->pHead;
    while(pCur != NULL)
    {
        if(!IsNullOrEmptyString(pCur->pszFilePath))
        {
            if(unlink(pCur->pszFilePath))
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
        }
        pCur = pCur->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
    return 1;
}

void
TDNFFreeCachedRpmsArray(
    PTDNF_CACHED_RPM_LIST pArray
    )
{
    PTDNF_CACHED_RPM_ENTRY pCur = NULL;
    PTDNF_CACHED_RPM_ENTRY pNext = NULL;

    if(pArray)
    {
        pCur = pArray->pHead;
        while(pCur != NULL)
        {
            pNext = pCur->pNext;
            TDNF_SAFE_FREE_MEMORY(pCur->pszFilePath);
            TDNF_SAFE_FREE_MEMORY(pCur);
            pCur = pNext;
        }
        TDNF_SAFE_FREE_MEMORY(pArray);
    }
}
