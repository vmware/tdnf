/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static
uint32_t
TDNFHasRepo(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pcszRepoId,
    int *pnHasRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_METALINK_DATA pData = NULL;
    int nHasRepo = 0;

    if (!pHandle || IsNullOrEmptyString(pcszRepoId) || !pnHasRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pData = pHandle->pData; pData; pData = pData->pNext)
    {
        if (strcmp(pData->pszRepoId, pcszRepoId) == 0)
        {
            nHasRepo = 1;
            break;
        }
    }

    *pnHasRepo = nHasRepo;

error:
    return dwError;
}

uint32_t
TDNFMetalinkReadConfig(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    char *pszMetalink = NULL;
    PCONF_SECTION pSection = NULL;
    PTDNF_METALINK_DATA pData = NULL;

    if (!pHandle || !pHandle->pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* event has a repo section which has all the config data */
    dwError = TDNFEventContextGetItemPtr(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_SECTION,
                  (const void **)&pSection);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueString(
                  pSection,
                  TDNF_REPO_CONFIG_METALINK_KEY,
                  NULL,
                  &pszMetalink);
    BAIL_ON_TDNF_ERROR(dwError);

    /*
     * if metalink is set, keep this repo id
     * section name is the repo id
    */
    if (pszMetalink)
    {
        dwError = TDNFAllocateMemory(sizeof(*pData), 1, (void **)&pData);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pSection->pszName, &pData->pszRepoId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pszMetalink, &pData->pszMetalink);
        BAIL_ON_TDNF_ERROR(dwError);

        pData->pNext = pHandle->pData;
        pHandle->pData = pData;
    }
cleanup:
    return dwError;

error:
    TDNFFreeMetalinkData(pData);
    goto cleanup;
}
