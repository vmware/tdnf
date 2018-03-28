/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : client.c
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
TDNFApplyScopeFilter(
    HyQuery hQuery,
    TDNF_SCOPE nScope
    )
{
    uint32_t dwError = 0;

    if(!hQuery || nScope == SCOPE_NONE)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    switch(nScope)
    {
        case SCOPE_INSTALLED:
            dwError = hy_query_filter(
                          hQuery,
                          HY_PKG_REPONAME,
                          HY_EQ,
                          HY_SYSTEM_REPO_NAME);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;

        case SCOPE_AVAILABLE:
            dwError = hy_query_filter(
                          hQuery,
                          HY_PKG_REPONAME,
                          HY_NEQ,
                          HY_SYSTEM_REPO_NAME);
            BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
            break;
           
        case SCOPE_UPGRADES:
            hy_query_filter_upgrades(hQuery, 1);
            break;

        case SCOPE_RECENT:
            hy_query_filter_latest_per_arch(hQuery, 1);
            break;
        case SCOPE_DOWNGRADES:
            hy_query_filter_downgrades(hQuery, 1);
            break;

        default:
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFApplyPackageFilter(
    HyQuery hQuery,
    char** ppszPackageNameSpecs
    )
{
    uint32_t dwError = 0;
    int nCmpType = HY_EQ;
    char** ppszPackagesTemp = NULL;

    if(!hQuery || !ppszPackageNameSpecs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!*ppszPackageNameSpecs)
    {
        goto cleanup;
    }

    ppszPackagesTemp = ppszPackageNameSpecs;
    while(ppszPackagesTemp && *ppszPackagesTemp)
    {
        if(TDNFIsGlob(*ppszPackagesTemp))
        {
            nCmpType = HY_GLOB;
            break;
        }
        ++ppszPackagesTemp;
    }

    dwError = hy_query_filter_in(
                  hQuery,
                  HY_PKG_NAME,
                  nCmpType,
                  (const char**)ppszPackageNameSpecs);
    BAIL_ON_TDNF_HAWKEY_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TdnfPkgsToExclude(
    PTDNF pTdnf,
    uint32_t *pdwCount,
    char***  pppszExcludes
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    uint32_t dwCount = 0;
    char**   ppszExcludes = NULL;
    int nIndex = 0;
    if(!pTdnf || !pTdnf->pArgs || !pdwCount || !pppszExcludes)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSetOpt = pTdnf->pArgs->pSetOpt;
    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
           !strcasecmp(pSetOpt->pszOptName, "exclude"))
        {
            dwCount++;
        }
        pSetOpt = pSetOpt->pNext;
    }

    if(dwCount > 0)
    {
        dwError = TDNFAllocateMemory(
                      dwCount + 1,
                      sizeof(char*),
                      (void**)&ppszExcludes);
        BAIL_ON_TDNF_ERROR(dwError);
        pSetOpt = pTdnf->pArgs->pSetOpt;
        while(pSetOpt)
        {
            if(pSetOpt->nType == CMDOPT_KEYVALUE &&
               !strcasecmp(pSetOpt->pszOptName, "exclude"))
            {
                dwError = TDNFAllocateString(
                      pSetOpt->pszOptValue,
                      &ppszExcludes[nIndex++]);
                BAIL_ON_TDNF_ERROR(dwError);

            }
            pSetOpt = pSetOpt->pNext;
        }
    }
    *pppszExcludes = ppszExcludes;
    *pdwCount = dwCount;
cleanup:

    return dwError;

error:
    if(pppszExcludes)
    {
        *pppszExcludes = NULL;
    }
    if(pdwCount)
    {
        *pdwCount = 0;
    }
    TDNF_SAFE_FREE_STRINGARRAY(ppszExcludes);
    goto cleanup;
}

uint32_t
TdnfAddExcludes(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    char** ppszExcludes = NULL;
    uint32_t dwCount = 0;

    HyQuery hQuery = NULL;
    HyPackageSet hPkgSet = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TdnfPkgsToExclude(pTdnf, &dwCount, &ppszExcludes);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwCount == 0 || !ppszExcludes)
    {
        goto cleanup;
    }
    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFApplyPackageFilter(hQuery, ppszExcludes);
    BAIL_ON_TDNF_ERROR(dwError);

    hPkgSet = hy_query_run_set(hQuery);
    if(hPkgSet)
    {
        hy_sack_add_excludes(pTdnf->hSack, hPkgSet);
    }

cleanup:
    TDNF_SAFE_FREE_STRINGARRAY(ppszExcludes);
    if(hPkgSet)
    {
        hy_packageset_free(hPkgSet);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
    }

    return dwError;
error:
    goto cleanup;
}
