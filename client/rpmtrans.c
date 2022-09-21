/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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
TDNFRpmCleanupTS(PTDNF pTdnf,
                 PTDNFRPMTS pTS)
{
    uint32_t dwError = 0;
    int nKeepCachedRpms = 0;
    int nDownloadOnly = 0;

    if(!pTS)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nKeepCachedRpms = pTdnf->pConf->nKeepCache;
    nDownloadOnly = pTdnf->pArgs->nDownloadOnly;

    if(pTS->pTS)
    {
        rpmtsCloseDB(pTS->pTS);
        rpmtsFree(pTS->pTS);
    }
    if(pTS->pCachedRpmsArray)
    {
        if(!nKeepCachedRpms && !nDownloadOnly)
        {
            TDNFRemoveCachedRpms(pTS->pCachedRpmsArray);
        }
        TDNFFreeCachedRpmsArray(pTS->pCachedRpmsArray);
    }
    TDNF_SAFE_FREE_MEMORY(pTS);

error:
    return dwError;
}

uint32_t
TDNFRpmCreateTS(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    PTDNFRPMTS *ppTS
    )
{
    uint32_t dwError = 0;
    PTDNFRPMTS pTS = NULL;
    PTDNF_CMD_OPT pSetOpt = NULL;

    if(!pTdnf || !pTdnf->pArgs || !pTdnf->pConf || !pSolvedInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNFRPMTS), (void **)&pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    pTS->nQuiet = pTdnf->pArgs->nQuiet;

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_CACHED_RPM_LIST),
                  (void**)&pTS->pCachedRpmsArray);
    BAIL_ON_TDNF_ERROR(dwError);

    rpmSetVerbosity(TDNFConfGetRpmVerbosity(pTdnf));

    //Allow downgrades
    pTS->nProbFilterFlags = RPMPROB_FILTER_OLDPACKAGE;
    if(pSolvedInfo->pPkgsToReinstall)
    {
        pTS->nProbFilterFlags = pTS->nProbFilterFlags | RPMPROB_FILTER_REPLACEPKG;
    }

    pTS->pTS = rpmtsCreate();
    if(!pTS->pTS)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* parse transaction flags - so far only tsflags=noscripts is
       supported. */
    pTS->nTransFlags = RPMTRANS_FLAG_NONE;
    for (pSetOpt = pTdnf->pArgs->pSetOpt; pSetOpt; pSetOpt = pSetOpt->pNext)
    {
        if (strcasecmp(pSetOpt->pszOptName, "tsflags"))
        {
            continue;
        }

        if (!strcasecmp(pSetOpt->pszOptValue, "noscripts"))
        {
            pTS->nTransFlags |= RPMTRANS_FLAG_NOSCRIPTS;
        }
    }

    if(rpmtsSetRootDir (pTS->pTS, pTdnf->pArgs->pszInstallRoot))
    {
        dwError = ERROR_TDNF_RPMTS_BAD_ROOT_DIR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(rpmtsSetNotifyCallback(pTS->pTS, TDNFRpmCB, (void*)pTS))
    {
        dwError = ERROR_TDNF_RPMTS_SET_CB_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPopulateTransaction(pTS, pTdnf, pSolvedInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppTS = pTS;

cleanup:
    return dwError;

error:
    if (pTS != NULL)
    {
        TDNFRpmCleanupTS(pTdnf, pTS);
    }
    goto cleanup;
}

uint32_t
TDNFRunTransactionWithHistory(
    PTDNF pTdnf,
    PTDNFRPMTS pTS,
    struct history_ctx *pHistoryCtx,
    char *pszCmdLine
    )
{
    uint32_t dwError = 0;
    int rc;

    if(!pTdnf || !pTS || !pHistoryCtx)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    rc = history_sync(pHistoryCtx, pTS->pTS);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRunTransaction(pTS, pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    rc = history_update_state(pHistoryCtx, pTS->pTS, pszCmdLine);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFRpmExecTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    )
{
    uint32_t dwError = 0;
    int nDownloadOnly = 0;
    PTDNFRPMTS pTS = NULL;
    struct history_ctx *pHistoryCtx = NULL;
    char *pszCmdLine = NULL;

    if(!pTdnf || !pTdnf->pArgs || !pTdnf->pConf || !pSolvedInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRpmCreateTS(pTdnf, pSolvedInfo, &pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    nDownloadOnly = pTdnf->pArgs->nDownloadOnly;
    if (!nDownloadOnly) {
        if (!pTdnf->pArgs->nTestOnly)
        {
            dwError = TDNFGetHistoryCtx(pTdnf, &pHistoryCtx, 0);
            BAIL_ON_TDNF_ERROR(dwError);

            if (pTdnf->pArgs->nArgc >= 1)
            {
                dwError = TDNFJoinArrayToString(&(pTdnf->pArgs->ppszArgv[1]),
                                                " ",
                                                pTdnf->pArgs->nArgc,
                                                &pszCmdLine);
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFRunTransactionWithHistory(pTdnf, pTS, pHistoryCtx, pszCmdLine);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFMarkAutoInstalled(pTdnf, pHistoryCtx, pSolvedInfo, 0);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else
        {
            dwError = TDNFRunTransaction(pTS, pTdnf);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCmdLine);
    if (pTS != NULL)
    {
        TDNFRpmCleanupTS(pTdnf, pTS);
    }
    if (pHistoryCtx)
    {
        destroy_history_ctx(pHistoryCtx);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRpmExecHistoryTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    PTDNF_HISTORY_ARGS pHistoryArgs
    )
{
    uint32_t dwError = 0;
    int nDownloadOnly = 0;
    PTDNFRPMTS pTS = NULL;
    struct history_ctx *pHistoryCtx = NULL;
    char *pszCmdLine = NULL;

    if(!pTdnf || !pSolvedInfo || !pHistoryArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRpmCreateTS(pTdnf, pSolvedInfo, &pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    nDownloadOnly = pTdnf->pArgs->nDownloadOnly;
    if (!nDownloadOnly && !pTdnf->pArgs->nTestOnly) {
        int rc = 0;
        int trans_id;

        dwError = TDNFGetHistoryCtx(pTdnf, &pHistoryCtx, 0);
        BAIL_ON_TDNF_ERROR(dwError);

        trans_id = pHistoryCtx->trans_id;

        if (pTdnf->pArgs->nArgc >= 1)
        {
            dwError = TDNFJoinArrayToString(&(pTdnf->pArgs->ppszArgv[1]),
                                            " ",
                                            pTdnf->pArgs->nArgc,
                                            &pszCmdLine);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwError = TDNFRunTransactionWithHistory(pTdnf, pTS, pHistoryCtx, pszCmdLine);
        BAIL_ON_TDNF_ERROR(dwError);

        /* if no rpm was added/removed no transaction was added yet,
            so we need to create a new transaction for the flags */
        if (trans_id == pHistoryCtx->trans_id)
        {
            rc = history_add_transaction(pHistoryCtx, pszCmdLine);
            if (rc != 0)
            {
                dwError = ERROR_TDNF_HISTORY_ERROR;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        else
        {
            /* Corner case where a redo/undo pulls additional dependencies. This
               can happen when those were installed originally, but have since
               been removed (this cannot happen on rollback because we'd restore the
               exact state).
               Avoid setting the flag to 0 (by using nAutoOnly=1) because it may
               be re-set again, and this case only applies to auto installed pkgs.
            */
            dwError = TDNFMarkAutoInstalled(pTdnf, pHistoryCtx, pSolvedInfo, 1);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        if (pHistoryArgs->nCommand == HISTORY_CMD_ROLLBACK)
        {
            rc = history_restore_auto_flags(pHistoryCtx, pHistoryArgs->nTo);
        }
        else if (pHistoryArgs->nCommand == HISTORY_CMD_UNDO)
        {
            rc = history_replay_auto_flags(pHistoryCtx, pHistoryArgs->nTo, pHistoryArgs->nFrom - 1);
        }
        else if (pHistoryArgs->nCommand == HISTORY_CMD_REDO)
        {
            rc = history_replay_auto_flags(pHistoryCtx, pHistoryArgs->nFrom - 1, pHistoryArgs->nTo);
        }
        if (rc != 0)
        {
            dwError = ERROR_TDNF_HISTORY_ERROR;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else if(!nDownloadOnly && pTdnf->pArgs->nTestOnly)
    {
        dwError = TDNFRunTransaction(pTS, pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCmdLine);
    if (pTS != NULL)
    {
        TDNFRpmCleanupTS(pTdnf, pTS);
    }
    if (pHistoryCtx)
    {
        destroy_history_ctx(pHistoryCtx);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFTransAddErasePkgs(
    PTDNFRPMTS pTS,
    PTDNF_PKG_INFO pInfo)
{
    uint32_t dwError = 0;
    if(!pInfo)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    while(pInfo)
    {
        dwError = TDNFTransAddErasePkg(pTS, pInfo->pszName);
        pInfo = pInfo->pNext;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
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
                      pTdnf,
                      pSolvedInfo->pPkgsToInstall);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToReinstall)
    {
        dwError = TDNFTransAddInstallPkgs(
                      pTS,
                      pTdnf,
                      pSolvedInfo->pPkgsToReinstall);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToUpgrade)
    {
        dwError = TDNFTransAddUpgradePkgs(
                      pTS,
                      pTdnf,
                      pSolvedInfo->pPkgsToUpgrade);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToRemove)
    {
        dwError = TDNFTransAddErasePkgs(
                      pTS,
                      pSolvedInfo->pPkgsToRemove);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsObsoleted)
    {
        dwError = TDNFTransAddObsoletedPkgs(
                      pTS,
                      pSolvedInfo->pPkgsObsoleted);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pSolvedInfo->pPkgsToDowngrade)
    {
        dwError = TDNFTransAddInstallPkgs(
                      pTS,
                      pTdnf,
                      pSolvedInfo->pPkgsToDowngrade);
        BAIL_ON_TDNF_ERROR(dwError);
        if(pSolvedInfo->pPkgsRemovedByDowngrade)
        {
            dwError = TDNFTransAddErasePkgs(
                      pTS,
                      pSolvedInfo->pPkgsRemovedByDowngrade);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFDetectPreTransFailure(
    rpmts pTS,
    char *pszError
    )
{
    uint32_t dwError = 0;
    rpmtsi pi = NULL;
    rpmte pte = NULL;
    char *pszToken;
    int i = 0;
    char *pszErrorStr = NULL;
    char *pszPkgName = NULL;
    char *pszSymbol = NULL;
    char *pszVersion = NULL;
    char *pszCachePkgName = NULL;
    char *pszCachePkgEVR = NULL;

    if (!pTS || IsNullOrEmptyString(pszError))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    // Error Str has the format: <Pkg-Name> <Symbol> <version-release>
    dwError = TDNFAllocateString(pszError, &pszErrorStr);
    BAIL_ON_TDNF_ERROR(dwError);

    pszToken = strtok(pszErrorStr, " ");
    while (pszToken != NULL)
    {
        switch(i)
        {
            case 0:
                pszPkgName = pszToken;
                break;
            case 1:
                pszSymbol = pszToken;
                break;
            case 2:
                pszVersion = pszToken;
                break;
            default:
                pr_err("RPM problem string format unsupported\n");
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
        }
        i++;
        pszToken = strtok(NULL, " ");
    }

    if (IsNullOrEmptyString(pszPkgName) || IsNullOrEmptyString(pszSymbol) || IsNullOrEmptyString(pszVersion))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pi = rpmtsiInit(pTS);
    while ((pte = rpmtsiNext(pi, 0)) != NULL)
    {
        dwError = TDNFAllocateString(rpmteN(pte), &pszCachePkgName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(rpmteEVR(pte), &pszCachePkgEVR);
        BAIL_ON_TDNF_ERROR(dwError);

        if (strcmp(pszCachePkgName, pszPkgName) == 0)
        {
            if ((strchr(pszSymbol, '>') && (rpmvercmp(pszCachePkgEVR, pszVersion) > 0)) ||
                (strchr(pszSymbol, '<') && (rpmvercmp(pszCachePkgEVR, pszVersion) < 0)) ||
                (strchr(pszSymbol, '=') && (rpmvercmp(pszCachePkgEVR, pszVersion) == 0)))
            {
                pr_err("Detected rpm pre-transaction dependency errors. "
                        "Install %s %s %s first to resolve this failure.\n",
                        pszPkgName, pszSymbol, pszVersion);
                break;
            }
        }
    }

cleanup:
    rpmtsiFree(pi);
    TDNF_SAFE_FREE_MEMORY(pszErrorStr);
    TDNF_SAFE_FREE_MEMORY(pszCachePkgName);
    TDNF_SAFE_FREE_MEMORY(pszCachePkgEVR);
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
    char *pErrorStr = NULL;

    rpmps ps = rpmtsProblems(pTS->pTS);
    if(ps)
    {
        int nProbs = rpmpsNumProblems(ps);
        if(nProbs > 0)
        {
            pr_crit("Found %d problems\n", nProbs);

            psi = rpmpsInitIterator(ps);
            while(rpmpsNextIterator(psi) >= 0)
            {
                prob = rpmpsGetProblem(psi);
                char *msg = rpmProblemString(prob);
                if (strstr(msg, "no digest") != NULL)
                {
                    pr_crit("%s. Use --skipdigest to ignore\n", msg);
                }
                else
                {
                    pr_crit("%s\n", msg);
                    if (rpmProblemGetType(prob) == RPMPROB_REQUIRES)
                    {
                        uint32_t dwError = 0;
                        dwError = TDNFAllocateString(rpmProblemGetStr(prob), &pErrorStr);
                        BAIL_ON_TDNF_ERROR(dwError);

                        dwError = TDNFDetectPreTransFailure(pTS->pTS, pErrorStr);
                        BAIL_ON_TDNF_ERROR(dwError);
                    }
                }
                rpmProblemFree(prob);
            }
            rpmpsFreeIterator(psi);
            nResult = ERROR_TDNF_RPM_CHECK;
        }
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pErrorStr);
    return nResult;
error:
    nResult = ERROR_TDNF_RPM_CHECK;
    goto cleanup;
}

uint32_t
TDNFRunTransaction(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    int rpmVfyLevelMask = 0;
    uint32_t dwSkipSignature = 0;
    uint32_t dwSkipDigest = 0;
    int rc;

    if(!pTS || !pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = rpmtsOrder(pTS->pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = doCheck(pTS);
    BAIL_ON_TDNF_ERROR(dwError);

    rpmtsClean(pTS->pTS);

    dwError = TDNFGetSkipSignatureOption(pTdnf, &dwSkipSignature);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetSkipDigestOption(pTdnf, &dwSkipDigest);
    BAIL_ON_TDNF_ERROR(dwError);

    //TODO do callbacks for output
    pr_info("Testing transaction\n");

    if (pTdnf->pArgs->nNoGPGCheck)
    {
        rpmtsSetVSFlags(pTS->pTS, rpmtsVSFlags(pTS->pTS) | RPMVSF_MASK_NODIGESTS | RPMVSF_MASK_NOSIGNATURES);
        rpmtsSetVfyLevel(pTS->pTS, ~RPMSIG_VERIFIABLE_TYPE);
    }

    else if (dwSkipSignature || dwSkipDigest)
    {
         if (dwSkipSignature)
         {
             rpmtsSetVSFlags(pTS->pTS, rpmtsVSFlags(pTS->pTS) | RPMVSF_MASK_NOSIGNATURES);
             rpmVfyLevelMask |= RPMSIG_SIGNATURE_TYPE;
         }
         if (dwSkipDigest)
         {
             rpmtsSetVSFlags(pTS->pTS, rpmtsVSFlags(pTS->pTS) | RPMVSF_MASK_NODIGESTS);
             rpmVfyLevelMask |= RPMSIG_DIGEST_TYPE;
         }
         rpmtsSetVfyLevel(pTS->pTS, ~rpmVfyLevelMask);
    }
    rpmtsSetFlags(pTS->pTS, RPMTRANS_FLAG_TEST);
    rc = rpmtsRun(pTS->pTS, NULL, pTS->nProbFilterFlags);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_TRANSACTION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (!pTdnf->pArgs->nTestOnly)
    {
        pr_info("Running transaction\n");

        rpmtsSetFlags(pTS->pTS, pTS->nTransFlags);
        rc = rpmtsRun(pTS->pTS, NULL, pTS->nProbFilterFlags);
        if (rc != 0)
        {
            dwError = ERROR_TDNF_TRANSACTION_FAILED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    if(pTS && dwError != ERROR_TDNF_RPM_CHECK)
    {
        doCheck(pTS);
    }
    goto cleanup;
}

uint32_t
TDNFTransAddInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfos
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pInfo;

    if(!pInfos)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pInfo = pInfos; pInfo; pInfo = pInfo->pNext)
    {
        PTDNF_REPO_DATA pRepo = NULL;

        dwError = TDNFFindRepoById(pTdnf, pInfo->pszRepoName, &pRepo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFTransAddInstallPkg(
                      pTS,
                      pTdnf,
                      pInfo->pszLocation,
                      pInfo->pszName,
                      pRepo, 0);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
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
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    int nUpgrade
    )
{
    uint32_t dwError = 0;
    int nGPGCheck = 0;
    char* pszFilePath = NULL;
    Header rpmHeader = NULL;
    PTDNF_CACHED_RPM_ENTRY pRpmCache = NULL;

    if (pszPackageLocation[0] == '/')
    {
        dwError = TDNFAllocateString(
                      pszPackageLocation,
                      &pszFilePath
                  );
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        if (!pTdnf->pArgs->nDownloadOnly || pTdnf->pArgs->pszDownloadDir == NULL)
        {
            dwError = TDNFDownloadPackageToCache(
                          pTdnf,
                          pszPackageLocation,
                          pszPkgName,
                          pRepo,
                          &pszFilePath
            );
        }
        else
        {
            dwError = TDNFDownloadPackageToDirectory(
                          pTdnf,
                          pszPackageLocation,
                          pszPkgName,
                          pRepo,
                          pTdnf->pArgs->pszDownloadDir,
                          &pszFilePath
            );

        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //A download could have been triggered.
    //So check access and bail if not available
    if(access(pszFilePath, F_OK))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = TDNFGPGCheckPackage(pTS, pTdnf, pRepo->pszId, pszFilePath, &rpmHeader);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetGPGCheck(pTdnf, pRepo->pszId, &nGPGCheck);
    BAIL_ON_TDNF_ERROR(dwError);
    if (!nGPGCheck)
    {
        rpmtsSetVSFlags(pTS->pTS, rpmtsVSFlags(pTS->pTS) | RPMVSF_MASK_NODIGESTS | RPMVSF_MASK_NOSIGNATURES);
        rpmtsSetVfyLevel(pTS->pTS, ~RPMSIG_VERIFIABLE_TYPE);
    }

    dwError = rpmtsAddInstallElement(
                  pTS->pTS,
                  rpmHeader,
                  (fnpyKey)pszFilePath,
                  nUpgrade,
                  NULL);
    BAIL_ON_TDNF_RPM_ERROR(dwError);

    /* add to cached array only when file is actually in cache dir */
    if(pTS->pCachedRpmsArray &&
        !strncmp(pszFilePath, pTdnf->pConf->pszCacheDir,
            strlen(pTdnf->pConf->pszCacheDir)))
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
    pr_err("Error processing package: %s\n", pszPackageLocation);
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pRpmCache);
    goto cleanup;
}

uint32_t
TDNFTransAddUpgradePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfos)
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pInfo;

    if(!pInfos)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pInfo = pInfos; pInfo; pInfo = pInfo->pNext)
    {
        PTDNF_REPO_DATA pRepo = NULL;

        dwError = TDNFFindRepoById(pTdnf, pInfo->pszRepoName, &pRepo);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFTransAddInstallPkg(
                      pTS,
                      pTdnf,
                      pInfo->pszLocation,
                      pInfo->pszName,
                      pRepo,
                      1);
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
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
    PTDNF_PKG_INFO pInfo
    )
{
    uint32_t dwError = 0;

    if(!pInfo)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while(pInfo)
    {
        dwError = TDNFTransAddErasePkg(pTS, pInfo->pszName);
        pInfo = pInfo->pNext;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
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
    const char* pkgName
    )
{
    uint32_t dwError = 0;
    Header pRpmHeader = NULL;
    rpmdbMatchIterator pIterator = NULL;

    if(!pTS || IsNullOrEmptyString(pkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pIterator = rpmtsInitIterator(pTS->pTS, (rpmTag)RPMDBI_LABEL, pkgName, 0);
    while ((pRpmHeader = rpmdbNextIterator(pIterator)) != NULL)
    {
        uint32_t nOffset = rpmdbGetIteratorOffset(pIterator);
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
            if(pTS->nQuiet)
                break;
            if(what == RPMCALLBACK_INST_START)
            {
                pr_info("%s", "Installing/Updating: ");
            }
            else
            {
                pr_info("%s", "Removing: ");
            }
            {
                char* pszNevra = headerGetAsString(pPkgHeader, RPMTAG_NEVRA);
                pr_info("%s\n", pszNevra);
                free(pszNevra);
                (void)fflush(stdout);
            }
            break;
        case RPMCALLBACK_SCRIPT_ERROR:
            {
                /* https://bugzilla.redhat.com/show_bug.cgi?id=216221#c15 */
                const char *pszScript;
                const char* pszNevra = headerGetAsString(pPkgHeader, RPMTAG_NEVRA);

                switch (amount)
                {
                    case RPMTAG_PREIN:
                        pszScript = "%prein";
                        break;
                    case RPMTAG_POSTIN:
                        pszScript = "%postin";
                        break;
                    case RPMTAG_PREUN:
                        pszScript = "%preun";
                        break;
                    case RPMTAG_POSTUN:
                        pszScript = "%postun";
                        break;
                    default:
                        pszScript = "(unknown)";
                        break;
                }
                /* %pre and %preun will cause errors (install/uninstall will fail),
                   other scripts just warn (install/uninstall will succeed) */
                pr_crit("package %s: script %s in %s\n",
                    pszNevra, total == RPMRC_OK ? "warning" : "error", pszScript);
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
