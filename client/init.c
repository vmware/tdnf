/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static
int _repo_compare(const void *ppRepo1, const void *ppRepo2)
{
    return (*(PTDNF_REPO_DATA*)(ppRepo1))->nPriority -
           (*(PTDNF_REPO_DATA*)(ppRepo2))->nPriority;
}

uint32_t
TDNFRefreshSack(
    PTDNF pTdnf,
    PSolvSack pSack,
    int nCleanMetadata
    )
{
    uint32_t dwError = 0;
    char* pszRepoCacheDir = NULL;
    int nMetadataExpired = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    PTDNF_REPO_DATA *ppRepoArray = NULL;
    uint32_t nCount = 0;
    uint32_t i = 0;

    if (!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nCleanMetadata == 1 && pTdnf->pArgs)
    {
        pTdnf->pArgs->nRefresh = 1;
    }

    for (pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
    {
        /*
         * skip the @cmdline repo - options do not apply, and it is
         * initialized.
         */
        if (!strcmp(pRepo->pszName, CMDLINE_REPO_NAME) || !pRepo->nEnabled)
        {
            continue;
        }
        nCount++;
    }

    /* nCount may be 0 if --disablerepo=* is used */
    if (nCount > 0)
    {
        dwError = TDNFAllocateMemory(nCount, sizeof(PTDNF_REPO_DATA),
                                     (void **)&ppRepoArray);
        BAIL_ON_TDNF_ERROR(dwError);

        for(pRepo = pTdnf->pRepos; pRepo; pRepo = pRepo->pNext)
        {
            if (!strcmp(pRepo->pszName, CMDLINE_REPO_NAME) || !pRepo->nEnabled)
            {
                continue;
            }
            ppRepoArray[i++] = pRepo;
        }

        qsort(ppRepoArray, nCount, sizeof(PTDNF_REPO_DATA), _repo_compare);
    }

    for (i = 0; i < nCount; i++)
    {
        pRepo = ppRepoArray[i];

        nMetadataExpired = 0;
        /* Check if expired since last sync per metadata_expire
           unless requested to ignore. lMetadataExpire < 0 means never expire. */
        if(pRepo->lMetadataExpire >= 0 && !pTdnf->pArgs->nCacheOnly)
        {
            dwError = TDNFGetCachePath(pTdnf, pRepo,
                                       NULL, NULL,
                                       &pszRepoCacheDir);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFShouldSyncMetadata(
                          pszRepoCacheDir,
                          pRepo->lMetadataExpire,
                          &nMetadataExpired);
            BAIL_ON_TDNF_ERROR(dwError);

            TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
            pszRepoCacheDir = NULL;
        }

        if (nMetadataExpired)
        {
            dwError = TDNFRepoRemoveCache(pTdnf, pRepo);
            if (dwError == ERROR_TDNF_FILE_NOT_FOUND)
            {
                dwError = 0; // ignore not existing folders
            }
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFRemoveSolvCache(pTdnf, pRepo);
            if (dwError == ERROR_TDNF_FILE_NOT_FOUND)
            {
                dwError = 0; // ignore not existing folders
            }
            BAIL_ON_TDNF_ERROR(dwError);
        }

        if (pSack)
        {
            dwError = TDNFInitRepo(pTdnf, pRepo, pSack);
        }
        if (dwError && pRepo->nSkipIfUnavailable)
        {
            if (dwError != (ERROR_TDNF_SYSTEM_BASE + EACCES)) {
                pRepo->nEnabled = 0;
                pr_info("Disabling Repo: '%s'\n", pRepo->pszName);
                dwError = 0;
            }
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (i = 0; i < nCount; i++) {
        pRepo = ppRepoArray[i];

        if (pRepo->nEnabled && pRepo->pszSnapshotFile) {
            dwError = TDNFApplySnapshot(pRepo, pSack, pRepo->pRepo);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRepoCacheDir);
    TDNF_SAFE_FREE_MEMORY(ppRepoArray);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRefresh(
    PTDNF pTdnf)
{
    if(!pTdnf || !pTdnf->pSack || !pTdnf->pArgs)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    return TDNFRefreshSack(pTdnf, pTdnf->pSack, pTdnf->pArgs->nRefresh);
}
