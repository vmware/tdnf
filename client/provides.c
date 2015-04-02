/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : provides.c
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
*/
#include "includes.h"

uint32_t
TDNFProvides(
    PTDNF pTdnf,
    const char* pszSpec,
    PTDNF_PKG_INFO* ppPkgInfo
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    HyQuery hQuery = NULL;
    HyPackageList hPkgList = NULL;
    HyReldep hReldep = NULL;

    int nFlag = HY_EQ;

    if(!pTdnf || IsNullOrEmptyString(pszSpec))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hQuery = hy_query_create(pTdnf->hSack);
    if(!hQuery)
    {
        dwError = HY_E_IO;
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    hReldep = hy_reldep_create(pTdnf->hSack, pszSpec, HY_EQ, NULL);
    if(hReldep)
    {
        dwError = hy_query_filter_provides(hQuery, HY_EQ, pszSpec, NULL);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }
    else
    {
        if(TDNFIsGlob(pszSpec))
        {
            nFlag = HY_GLOB;
        }
        dwError = hy_query_filter(hQuery, HY_PKG_FILE, nFlag, pszSpec);
        BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    hPkgList = hy_query_run(hQuery);
    if(!hPkgList)
    {
      dwError = HY_E_IO;
      BAIL_ON_TDNF_HAWKEY_ERROR(dwError);
    }

    dwError = TDNFPopulatePkgInfos(hPkgList, &pPkgInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPkgInfo = pPkgInfo;
cleanup:
    if(hPkgList)
    {
        hy_packagelist_free(hPkgList);
    }
    if(hQuery)
    {
        hy_query_free(hQuery);
    }
    return dwError;

error:
    if(ppPkgInfo)
    {
      *ppPkgInfo = NULL;
    }
    TDNFFreePackageInfo(pPkgInfo);
    goto cleanup;
}
