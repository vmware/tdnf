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
    if(!hQuery || !ppszPackageNameSpecs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    while(*ppszPackageNameSpecs)
    {
        if(TDNFIsGlob(*ppszPackageNameSpecs))
        {
            hy_query_filter(hQuery, HY_PKG_NAME, HY_GLOB, *ppszPackageNameSpecs);
        }
        else
        {
            hy_query_filter(hQuery, HY_PKG_NAME, HY_EQ, *ppszPackageNameSpecs);
        }
        ++ppszPackageNameSpecs;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
