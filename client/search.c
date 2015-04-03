/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : search.c
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Touseef Liaqat (tliaqat@vmware.com)
      *
*/
#include "includes.h"

uint32_t
TDNFSearchCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* punCount
    )
{
    HySack hSack = NULL;
    HyQuery hQuery = NULL;
    HyPackage hPkg = NULL;
    HyPackageList hAccumPkgList = NULL;
    uint32_t unError = 0;
    uint32_t unCount = 0;
    int nIndex = 0;
    int nStartArgIndex = 1;
    const char* pszFirstParam = NULL;
    bool bSearchAll = false;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pTdnf || !pCmdArgs || !ppPkgInfo || !punCount)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(unError);
    }

    hAccumPkgList = hy_packagelist_create();
    if(!hAccumPkgList)
    {
        unError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    if(pCmdArgs->nCmdCount > 1)
    {
        pszFirstParam = pCmdArgs->ppszCmds[1];
        if(!strncasecmp(pszFirstParam, "all", 3))
        {
            bSearchAll = true;
            nStartArgIndex = 2;
        }
    }

    hSack = hy_sack_create(NULL, NULL, NULL, 0);
    if(!hSack)
    {
        unError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    unError = hy_sack_load_system_repo(hSack, NULL, 0);
    BAIL_ON_TDNF_HAWKEY_ERROR(unError);

    hQuery = hy_query_create(hSack);
    if(!hQuery)
    {
      unError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    unError = TDNFQueryTerms(
        hAccumPkgList,
        pCmdArgs,
        hQuery,
        nStartArgIndex,
        QueryTermsInNameSummary);

    BAIL_ON_TDNF_ERROR(unError);

    unCount = hy_packagelist_count(hAccumPkgList);

    // Search more if nothing found or 'all' was requested.
    if (bSearchAll == true || unCount == 0)
    {
        unError = TDNFQueryTerms(
            hAccumPkgList,
            pCmdArgs,
            hQuery,
            nStartArgIndex,
            QueryTermsInDescUrl);

        BAIL_ON_TDNF_ERROR(unError);
    }

    unCount = hy_packagelist_count(hAccumPkgList);

    if (unCount < 1)
    {
        unError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(unError);
    }

    unError = TDNFAllocateMemory(
                sizeof(TDNF_PKG_INFO) * unCount,
                (void**)&pPkgInfo);

    BAIL_ON_TDNF_ERROR(unError);

    FOR_PACKAGELIST(hPkg, hAccumPkgList, nIndex)
    {
        PTDNF_PKG_INFO pPkg = &pPkgInfo[nIndex];
        unError = TDNFSafeAllocateString(hy_package_get_name(hPkg), &pPkg->pszName);
        BAIL_ON_TDNF_ERROR(unError);

        unError = TDNFSafeAllocateString(hy_package_get_summary(hPkg), &pPkg->pszSummary);
        BAIL_ON_TDNF_ERROR(unError);
    }

    *ppPkgInfo = pPkgInfo;
    *punCount = unCount;

cleanup:
    if (hAccumPkgList != NULL)
    {
        hy_packagelist_free(hAccumPkgList);
    }

    if (hQuery != NULL)
    {
        hy_query_free(hQuery);
    }

    if (hSack != NULL)
    {
        hy_sack_free(hSack);
    }

    return unError;
error:
    if(ppPkgInfo)
    {
      *ppPkgInfo = NULL;
    }
    if(punCount)
    {
      *punCount = 0;
    }
    TDNFFreePackageInfoArray(pPkgInfo, unCount);
    goto cleanup;
}

uint32_t
QueryTermsInNameSummary(
    HyPackageList hAccumPkgList,
    HyQuery hQuery,
    const char* pszSearchTerm
    )
{
    uint32_t unError = 0;

    if(!hAccumPkgList || !hQuery || !pszSearchTerm)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        goto error;
    }

    unError = TDNFQueryTermsHelper(hAccumPkgList, hQuery, HY_PKG_NAME, pszSearchTerm);
    if (unError == 0)
    {
        unError = TDNFQueryTermsHelper(hAccumPkgList, hQuery, HY_PKG_SUMMARY, pszSearchTerm);
    }

cleanup:
    return unError;
error:
    goto cleanup;
}

uint32_t
QueryTermsInDescUrl(
    HyPackageList hAccumPkgList,
    HyQuery hQuery,
    const char* pszSearchTerm
    )
{
    uint32_t unError = 0;

    if(!hAccumPkgList || !hQuery || !pszSearchTerm)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        goto error;
    }

    unError = TDNFQueryTermsHelper(hAccumPkgList, hQuery, HY_PKG_DESCRIPTION, pszSearchTerm);
    if (unError == 0)
    {
        unError = TDNFQueryTermsHelper(hAccumPkgList, hQuery, HY_PKG_URL, pszSearchTerm);
    }

cleanup:
    return unError;
error:
    goto cleanup;
}

uint32_t
TDNFQueryTerms(
    HyPackageList hAccumPkgList,
    PTDNF_CMD_ARGS pCmdArgs,
    HyQuery hQuery,
    int nStartArgIndex,
    TDNFQueryTermsFunction pfQueryTerms
    )
{
    uint32_t unError = 0;
    int nArgIndex = 0;
    const char* pszSearchTerm = NULL;

    if(!hAccumPkgList || !pCmdArgs || !hQuery)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        goto error;
    }

    for (nArgIndex = nStartArgIndex; nArgIndex < pCmdArgs->nCmdCount && unError == 0; nArgIndex++)
    {
        // Convert 'searchTerm' to '*searchTerm*'
        unError = TDNFCopyWithWildCards(
            pCmdArgs->ppszCmds[nArgIndex],
            (const char**)&pszSearchTerm);

        if (unError != 0)
        {
            goto error;
        }

        unError = pfQueryTerms(hAccumPkgList, hQuery, pszSearchTerm);

        if (pszSearchTerm != NULL)
        {
            TDNFFreeMemory((void *)pszSearchTerm);
        }
    }

cleanup:
    return unError;
error:
    goto cleanup;
}

uint32_t
TDNFQueryTermsHelper(
    HyPackageList hAccumPkgList,
    HyQuery hQuery,
    int nKeyId,
    const char* pszMatch
    )
{
    HyPackageList hPkgList = NULL;
    HyPackage hPkg = NULL;
    uint32_t unError = 0;
    int nIndex = 0;

    if(!hAccumPkgList || !hQuery || !pszMatch)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        goto error;
    }

    unError = hy_query_filter(
                  hQuery,
                  nKeyId,
                  HY_GLOB | HY_ICASE,
                  pszMatch);
    BAIL_ON_TDNF_HAWKEY_ERROR(unError);

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
        unError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(unError);
    }

    FOR_PACKAGELIST(hPkg, hPkgList, nIndex)
    {
        if (!hy_packagelist_has(hAccumPkgList, hPkg))
        {
            hy_packagelist_push(hAccumPkgList, hy_package_link(hPkg));
        }
    }

cleanup:
    if (hPkgList != NULL)
    {
       hy_packagelist_free(hPkgList);
    }

    if (hQuery != NULL)
    {
        hy_query_clear(hQuery);
    }

    return unError;
error:
    goto cleanup;
}

uint32_t
TDNFCopyWithWildCards(
    const char* pszSrc,
    const char** ppszDst
    )
{
    uint32_t unError = 0;
    char* pszNewDst = NULL;

    if(!pszSrc || !ppszDst)
    {
        unError = ERROR_TDNF_INVALID_PARAMETER;
        goto error;
    }

    unError = TDNFAllocateMemory(
                           3 + strlen(pszSrc),
                           (void**)&pszNewDst);
    if (unError == 0)
    {
        strncpy((char *)pszNewDst, "*", 1);
        strncpy((char *)pszNewDst + 1, pszSrc, strlen(pszSrc));
        strncpy((char *)pszNewDst + strlen(pszSrc) + 1, "*", 1);

        *ppszDst = pszNewDst;
    }

cleanup:
    return unError;
error:
    goto cleanup;
}
