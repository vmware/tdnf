/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : repo.c
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

//Download repo metadata and initialize
uint32_t
TDNFInitRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepoData,
    HyRepo* phRepo
    )
{
    uint32_t dwError = 0;
    gboolean bRet = 0;
    LrHandle* hLibRepo = NULL;
    LrResult* pResult = NULL;
    LrYumRepo* pRepo = NULL;
    int nLocalOnly = 0;
    char* pszRepoCacheDir = NULL;
    char* pszRepoDataDir = NULL;
    char* pszUserPass = NULL;

    char* ppszRepoUrls[] = {NULL, NULL};
    char* ppszLocalUrls[] = {NULL, NULL};
    char* ppszDownloadList[] = {"primary", "filelists", "updateinfo", NULL};
    PTDNF_CONF pConf = NULL;

    HyRepo hRepo = NULL;

    if(!pTdnf || !pTdnf->pConf || !pRepoData || !phRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pConf = pTdnf->pConf;

    pszRepoCacheDir = g_build_path(
                           G_DIR_SEPARATOR_S,
                           pConf->pszCacheDir,
                           pRepoData->pszId,
                           NULL);
    if(!pszRepoCacheDir)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszRepoDataDir = g_build_path(
                           G_DIR_SEPARATOR_S,
                           pszRepoCacheDir,
                           TDNF_REPODATA_DIR_NAME,
                           NULL);
    if(!pszRepoDataDir)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppszRepoUrls[0] = pRepoData->pszBaseUrl;
    ppszLocalUrls[0] = pszRepoCacheDir;

    hLibRepo = lr_handle_init();
    if(!hLibRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pResult = lr_result_init();
    if(!pResult)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Look for repodata dir - this is auto created
    //during last refresh so skip download if present
    if(!access(pszRepoDataDir, F_OK))
    {
        nLocalOnly = 1;
        lr_handle_setopt(hLibRepo, NULL, LRO_URLS, ppszLocalUrls);
        lr_handle_setopt(hLibRepo, NULL, LRO_IGNOREMISSING, 1);
    }
    else
    {
        //Look for the repo root cache dir. If not there,
        //try to create and download into it.
        if(access(pszRepoCacheDir, F_OK))
        {
            if(errno != ENOENT)
            {
                dwError = errno;
            }
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);

            if(mkdir(pszRepoCacheDir, 755))
            {
                dwError = errno;
                BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
            }
        }

        lr_handle_setopt(hLibRepo, NULL, LRO_URLS, ppszRepoUrls);
        lr_handle_setopt(hLibRepo, NULL, LRO_SSLVERIFYPEER, 1);
        lr_handle_setopt(hLibRepo, NULL, LRO_SSLVERIFYHOST, 2);
        lr_handle_setopt(hLibRepo, NULL, LRO_DESTDIR, pszRepoCacheDir);
        lr_handle_setopt(hLibRepo, NULL, LRO_YUMDLIST, ppszDownloadList);
        if(!IsNullOrEmptyString(pRepoData->pszUser) && 
           !IsNullOrEmptyString(pRepoData->pszPass))
        {
            dwError = TDNFAllocateStringPrintf(
                          &pszUserPass,
                          "%s:%s",
                          pRepoData->pszUser,
                          pRepoData->pszPass);
            BAIL_ON_TDNF_ERROR(dwError);
            lr_handle_setopt(
                hLibRepo,
                NULL,
                LRO_USERPWD,
                pszUserPass);
        }

        dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, hLibRepo);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    lr_handle_setopt(hLibRepo, NULL, LRO_REPOTYPE, LR_YUMREPO);
    lr_handle_setopt(hLibRepo, NULL, LRO_LOCAL, nLocalOnly);

    
    bRet = lr_handle_perform(hLibRepo, pResult, NULL);
    if(!bRet)
    {
        dwError = ERROR_TDNF_REPO_PERFORM;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    bRet = lr_result_getinfo(pResult, NULL, LRR_YUM_REPO, &pRepo);
    if(!bRet)
    {
        dwError = ERROR_TDNF_REPO_GETINFO;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Create and set repo properties   
    hRepo = hy_repo_create(pRepoData->pszId);
    if(!hRepo)
    {
        dwError = ERROR_TDNF_HAWKEY_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFInitRepoFromMetaData(hRepo, pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    *phRepo = hRepo;
cleanup:
    if(pszRepoDataDir)
    {
        g_free(pszRepoDataDir);
    }
    if(pszRepoCacheDir)
    {
        g_free(pszRepoCacheDir);
    }

    if(pResult)
    {
        lr_result_free(pResult);
    }
    if(hLibRepo)
    {
        lr_handle_free(hLibRepo);
    }
    return dwError;

error:
    //If there is an error during init, log the error
    //remove any cache data that could be potentially corrupt.
    if(pRepoData)
    {
        fprintf(
            stderr,
            "Error: Failed to synchronize cache for repo '%s' from '%s'\n",
            pRepoData->pszName,
            pRepoData->pszBaseUrl);

        if(pTdnf)
        {
            TDNFRepoRemoveCache(pTdnf, pRepoData->pszId);
        }
    }
    if(phRepo)
    {
        *phRepo = NULL;
    }
    if(hRepo)
    {
       hy_repo_free(hRepo);
    }
    goto cleanup;
}

uint32_t
TDNFInitRepoFromMetaData(
    HyRepo hRepo,
    LrYumRepo* pRepo
    )
{
    uint32_t dwError = 0;
    const char* pszValue = NULL;
    
    if(!pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hy_repo_set_string(hRepo, HY_REPO_MD_FN, pRepo->repomd);
    pszValue = lr_yum_repo_path(pRepo, "primary");
    if(pszValue)
    {
        hy_repo_set_string(hRepo, HY_REPO_PRIMARY_FN, pszValue);
    }
    pszValue = lr_yum_repo_path(pRepo, "filelists");
    if(pszValue != NULL)
    {
        hy_repo_set_string(hRepo, HY_REPO_FILELISTS_FN, pszValue);
    }
    pszValue = lr_yum_repo_path(pRepo, "updateinfo");
    if(pszValue != NULL)
    {
        hy_repo_set_string (hRepo, HY_REPO_UPDATEINFO_FN, pszValue);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPrintRepoMetadata(
    LrYumRepoMd* pRepoMD
    )
{
    uint32_t dwError = 0;

    GSList* pElem = NULL;

    if(!pRepoMD)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pElem = pRepoMD->records;

    printf("Repo metadata info:\n");

    for (; pElem; pElem = g_slist_next(pElem))
    {
        LrYumRepoMdRecord* pRec = (LrYumRepoMdRecord*) pElem->data;
        printf(" Type: %s\n", pRec->type);
        printf(" Location href: %s\n", pRec->location_href);
        if (pRec->location_base)
            printf(" Location base: %s\n", pRec->location_base);
        if (pRec->checksum)
            printf(" Checksum: %s\n", pRec->checksum);
        if (pRec->checksum_type)
            printf(" Checksum type: %s\n", pRec->checksum_type);
        if (pRec->checksum_open)
            printf(" Checksum open: %s\n", pRec->checksum_open);
        if (pRec->checksum_open_type)
            printf(" Checksum open type: %s\n", pRec->checksum_open_type);
        if (pRec->timestamp > 0)
            printf(" Timestamp: %"G_GINT64_FORMAT"\n", pRec->timestamp);
        if (pRec->size > 0)
            printf(" Size: %"G_GINT64_FORMAT"\n", pRec->size);
        if (pRec->size_open > 0)
            printf(" Size open: %"G_GINT64_FORMAT"\n", pRec->size_open);
        if (pRec->db_version > 0)
            printf(" Db version: %d\n", pRec->db_version);
        printf("------------------------------------------------\n");
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGetGPGCheck(
    PTDNF pTdnf,
    const char* pszRepo,
    int* pnGPGCheck,
    char** ppszUrlGPGKey
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    int nGPGCheck = 0;
    char* pszUrlGPGKey = NULL;

    if(!pTdnf || !ppszUrlGPGKey || IsNullOrEmptyString(pszRepo))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(!pTdnf->pArgs->nNoGPGCheck)
    {
        dwError = TDNFGetRepoByName(pTdnf, pszRepo, &pRepo);
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);
        if(pRepo)
        {
            nGPGCheck = pRepo->nGPGCheck;
            dwError = TDNFAllocateString(pRepo->pszUrlGPGKey, &pszUrlGPGKey);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    *pnGPGCheck = nGPGCheck;
    *ppszUrlGPGKey = pszUrlGPGKey;

cleanup:
    return dwError;

error:
    if(ppszUrlGPGKey)
    {
        *ppszUrlGPGKey = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszUrlGPGKey);
    goto cleanup;
}

//TODO: This needs to be GetRepoById.
uint32_t
TDNFGetRepoByName(
    PTDNF pTdnf,
    const char* pszName,
    PTDNF_REPO_DATA* ppRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = NULL;
    PTDNF_REPO_DATA pRepos = NULL;

    if(!pTdnf || IsNullOrEmptyString(pszName) || !ppRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pTdnf->pRepos)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pRepos = pTdnf->pRepos;

    while(pRepos)
    {
        if(!strcmp(pszName, pRepos->pszId))
        {
            pRepo = pRepos;
            break;
        }
        pRepos = pRepos->pNext;
    }

    if(!pRepo)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppRepo = pRepo;
cleanup:
    return dwError;

error:
    goto cleanup;
}
