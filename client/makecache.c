/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : makecache.c
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
TDNFRefreshCache(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    HySack hSack = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Creating a new sack without removing the
    //old one did not work well. Remove first, then 
    //create 
    if(pTdnf->hSack)
    {
        hy_sack_free(pTdnf->hSack);
        pTdnf->hSack = NULL;
    }

    //init with cache
    dwError = TDNFInitSack(pTdnf, &hSack, HY_LOAD_FILELISTS);
    BAIL_ON_TDNF_ERROR(dwError);

    //Do the same for all enabled repos
    if(pTdnf->pRepos)
    {
        PTDNF_REPO_DATA pRepo = pTdnf->pRepos;
        while(pRepo)
        {
            if(pRepo->nEnabled)
            {
                hy_repo_free(pRepo->hRepo);
                pRepo->hRepo = NULL;

                dwError = TDNFInitRepo(pTdnf, pRepo, &pRepo->hRepo);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            pRepo = pRepo->pNext;
        }
    }

    pTdnf->hSack = hSack;

cleanup:
    return dwError;

error:
    if(hSack)
    {
        hy_sack_free(hSack);
    }
    if(pTdnf->hSack)
    {
        hy_sack_free(pTdnf->hSack);
        pTdnf->hSack = NULL;
    }
    goto cleanup;
}
