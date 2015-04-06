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
