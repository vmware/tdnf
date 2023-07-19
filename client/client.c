/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
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

    if (!pTdnf->pArgs->nDisableExcludes && pTdnf->pConf->ppszExcludes)
    {
        pr_info("Warning: The following packages are excluded "
                "from tdnf.conf:\n");
        while (pTdnf->pConf->ppszExcludes[nIndex])
        {
            pr_info("  %s\n", pTdnf->pConf->ppszExcludes[nIndex]);
            dwCount++;
            nIndex++;
        }
    }

    pSetOpt = pTdnf->pArgs->pSetOpt;
    while(pSetOpt)
    {
        if(!strcasecmp(pSetOpt->pszOptName, "exclude"))
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

        nIndex = 0;
        if (!pTdnf->pArgs->nDisableExcludes && pTdnf->pConf->ppszExcludes)
        {
            while (pTdnf->pConf->ppszExcludes[nIndex])
            {
                dwError = TDNFAllocateString(pTdnf->pConf->ppszExcludes[nIndex],
                                             &ppszExcludes[nIndex]);
                BAIL_ON_TDNF_ERROR(dwError);
                dwCount++;
                nIndex++;
            }
        }

        pSetOpt = pTdnf->pArgs->pSetOpt;
        while(pSetOpt)
        {
            if(!strcasecmp(pSetOpt->pszOptName, "exclude"))
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
