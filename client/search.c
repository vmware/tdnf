/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : search.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Touseef Liaqat (tliaqat@vmware.com)
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
    uint32_t dwError = 0;
    int nArgIndex = 0;
    char* pszSearchTerm = NULL;

    if(!hAccumPkgList || !pCmdArgs || !hQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (nArgIndex = nStartArgIndex; nArgIndex < pCmdArgs->nCmdCount; nArgIndex++)
    {
        // Convert 'searchTerm' to '*searchTerm*'
        dwError = TDNFAllocateStringPrintf(
                     &pszSearchTerm,
                     "*%s*",
                     pCmdArgs->ppszCmds[nArgIndex]);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = pfQueryTerms(hAccumPkgList, hQuery, pszSearchTerm);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszSearchTerm);
        pszSearchTerm = NULL;
    }

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszSearchTerm);
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
