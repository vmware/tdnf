/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
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
    PSolvQuery pQuery,
    TDNF_SCOPE nScope
    )
{
    uint32_t dwError = 0;

    if(!pQuery || nScope == SCOPE_NONE)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery->nScope = nScope;

    switch(nScope)
    {
        case SCOPE_INSTALLED:
            dwError = SolvAddSystemRepoFilter(pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
            break;

        case SCOPE_AVAILABLE:
            dwError = SolvAddAvailableRepoFilter(pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
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
TDNFPkgsToExclude(
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
TDNFAddExcludes(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    char** ppszExcludes = NULL;
    uint32_t dwCount = 0;
    Map* nexcl = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPkgsToExclude(pTdnf, &dwCount, &ppszExcludes);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwCount == 0 || !ppszExcludes)
    {
        goto cleanup;
    }

    nexcl = solv_calloc(1, sizeof(Map));
    map_init(nexcl, pTdnf->pSack->pPool->nsolvables);

    dwError = TDNFFilterDataIterator(pTdnf, ppszExcludes, nexcl);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pTdnf->pSack->pPool->considered)
    {
        pTdnf->pSack->pPool->considered = solv_calloc(1, sizeof(Map));
        map_init(pTdnf->pSack->pPool->considered, pTdnf->pSack->pPool->nsolvables);
    }
    else
    {
       map_grow(pTdnf->pSack->pPool->considered, pTdnf->pSack->pPool->nsolvables);
    }

    map_setall(pTdnf->pSack->pPool->considered);
    if (nexcl)
    {
       map_subtract(pTdnf->pSack->pPool->considered, nexcl);
    }
cleanup:
    TDNF_SAFE_FREE_STRINGARRAY(ppszExcludes);
    TDNFFreeMemory(nexcl);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFFilterDataIterator(
     PTDNF pTdnf,
     char** ppszExcludes,
     Map* m
     )
{
    Dataiterator di;
    Id keyname = SOLVABLE_NAME;
    int flags = 0;
    char **ppszPackagesTemp = NULL;
    uint32_t dwError;

    if(!pTdnf || !ppszExcludes || !m)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ppszPackagesTemp = ppszExcludes;
    while(ppszPackagesTemp && *ppszPackagesTemp)
    {
          flags = SEARCH_STRING;
          if (TDNFIsGlob(*ppszPackagesTemp))
          {
              flags = SEARCH_GLOB;
          }
          dwError = dataiterator_init(&di, pTdnf->pSack->pPool, 0, 0, keyname, *ppszPackagesTemp, flags);
          BAIL_ON_TDNF_ERROR(dwError);
          while (dataiterator_step(&di))
          {
              MAPSET(m, di.solvid);
          }
          dataiterator_free(&di);
          ++ppszPackagesTemp;
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}
