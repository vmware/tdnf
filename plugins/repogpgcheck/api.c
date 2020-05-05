/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include "config.h"
#include <gpgme.h>

TDNF_PLUGIN_INTERFACE _interface = {0};

const char *
TDNFPluginGetVersion(
    )
{
    return PLUGIN_VERSION;
}

const char *
TDNFPluginGetName(
    )
{
    return PLUGIN_NAME;
}

uint32_t
TDNFPluginLoadInterface(
    PTDNF_PLUGIN_INTERFACE pInterface
    )
{
    uint32_t dwError = 0;

    if (!pInterface)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pInterface->pFnInitialize = TDNFRepoGPGCheckInitialize;
    pInterface->pFnEventsNeeded = TDNFRepoGPGCheckEventsNeeded;
    pInterface->pFnGetErrorString = TDNFRepoGPGCheckGetErrorString;
    pInterface->pFnEvent = TDNFRepoGPGCheckEvent;
    pInterface->pFnCloseHandle = TDNFRepoGPGCheckClose;

error:
    return dwError;
}

uint32_t
TDNFRepoGPGCheckInitialize(
    const char *pszConfig,
    PTDNF_PLUGIN_HANDLE *ppHandle
    )
{
    UNUSED(pszConfig);
    uint32_t dwError = 0;
    PTDNF_PLUGIN_HANDLE pHandle = NULL;

    /* plugin does not expect config */
    if (!ppHandle)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGPGCheckVerifyVersion();
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(sizeof(*pHandle), 1, (void **)&pHandle);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppHandle = pHandle;

cleanup:
    return dwError;

error:
    FreePluginHandle(pHandle);
    goto cleanup;
}

void
TDNFFreeRepoGPGCheckData(
    PTDNF_REPO_GPG_CHECK_DATA pData
    )
{
    PTDNF_REPO_GPG_CHECK_DATA pTemp = NULL;
    while (pData)
    {
        pTemp = pData->pNext;
        TDNF_SAFE_FREE_MEMORY(pData->pszRepoId);
        TDNFFreeMemory(pData);
        pData = pTemp;
    }
}

void
FreePluginHandle(
    PTDNF_PLUGIN_HANDLE pHandle
    )
{
    if (pHandle)
    {
        TDNFFreeRepoGPGCheckData(pHandle->pData);
        TDNFFreeMemory(pHandle);
    }
}

/*
 * repogpgcheck is only interested in repo events
 * this means all repo events will recieve a
 * callback to the registered event callback function
*/
uint32_t
TDNFRepoGPGCheckEventsNeeded(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    )
{
    uint32_t dwError = 0;
    if (!pHandle || !pnEvents)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pnEvents = TDNF_PLUGIN_EVENT_TYPE_REPO;

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * pContext->nEvent has the following
 * 1. event type such as TDNF_PLUGIN_EVENT_TYPE_REPO
 * 2. event state such as TDNF_PLUGIN_EVENT_STATE_READCONFIG
 * 3. event phase such as TDNF_PLUGIN_EVENT_PHASE_START
 * pContext->pTdnf is the handle to libtdnf
*/
uint32_t
TDNFRepoGPGCheckEvent(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    TDNF_PLUGIN_EVENT_TYPE nEventType = TDNF_PLUGIN_EVENT_TYPE_NONE;
    TDNF_PLUGIN_EVENT_STATE nEventState = TDNF_PLUGIN_EVENT_STATE_NONE;
    TDNF_PLUGIN_EVENT_PHASE nEventPhase = TDNF_PLUGIN_EVENT_PHASE_NONE;

    if (!pHandle || !pContext)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nEventType = PLUGIN_EVENT_TYPE(pContext->nEvent);
    nEventState = PLUGIN_EVENT_STATE(pContext->nEvent);
    nEventPhase = PLUGIN_EVENT_PHASE(pContext->nEvent);

    if (nEventType == TDNF_PLUGIN_EVENT_TYPE_INIT)
    {
        dwError = TDNFEventContextGetItemPtr(
                      pContext,
                      TDNF_EVENT_ITEM_TDNF_HANDLE,
                      (const void **)&pHandle->pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    /* repo events to read config and cache entries with repo_gpgcheck=1 */
    else if (nEventType == TDNF_PLUGIN_EVENT_TYPE_REPO)
    {
        if (nEventState == TDNF_PLUGIN_EVENT_STATE_READCONFIG &&
            nEventPhase == TDNF_PLUGIN_EVENT_PHASE_END)
        {
            dwError = TDNFRepoGPGCheckReadConfig(pHandle, pContext);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else if (nEventType == TDNF_PLUGIN_EVENT_TYPE_REPO_MD)
    {
        if (nEventState == TDNF_PLUGIN_EVENT_STATE_DOWNLOAD &&
            nEventPhase == TDNF_PLUGIN_EVENT_PHASE_END)
        {
            dwError = TDNFRepoMDCheckSignature(pHandle, pContext);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else
    {
        fprintf(stderr, "Unexpected event %d in %s plugin\n",
                pContext->nEvent, PLUGIN_NAME);
        goto cleanup;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoGPGCheckGetErrorString(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t nErrorCode,
    char **ppszError
    )
{
    uint32_t dwError = 0;
    char *pszError = NULL;
    char *pszErrorPre = NULL;
    const char *pszGPGError = NULL;
    size_t i = 0;
    TDNF_ERROR_DESC arErrorDesc[] = REPOGPGCHECK_ERROR_TABLE;

    if (!pHandle || !ppszError)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (nErrorCode > ERROR_TDNF_GPG_BASE_START && nErrorCode < ERROR_TDNF_GPGME_START)
    {
        for(i = 0; i < ARRAY_SIZE(arErrorDesc); ++i)
        {
            if (nErrorCode == (uint32_t)arErrorDesc[i].nCode)
            {
                pszErrorPre = arErrorDesc[i].pszDesc;
                break;
            }
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pHandle->nGPGError)
    {
        pszGPGError = gpgme_strerror(pHandle->nGPGError);
    }

    if (pszGPGError)
    {
        dwError = TDNFAllocateStringPrintf(
                      &pszError, "%s %s: %s\n",
                      REPOGPGCHECK_PLUGIN_ERROR, pszErrorPre, pszGPGError);
    }
    else
    {
        dwError = TDNFAllocateStringPrintf(
                      &pszError, "%s: %s\n",
                      REPOGPGCHECK_PLUGIN_ERROR, pszErrorPre);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszError = pszError;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszError);
    goto cleanup;
}

uint32_t
TDNFRepoGPGCheckClose(
    PTDNF_PLUGIN_HANDLE pHandle
    )
{
    uint32_t dwError = 0;

    if (!pHandle)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    FreePluginHandle(pHandle);

error:
    return dwError;
}
