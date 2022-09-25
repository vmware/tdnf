/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
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
TDNFPluginGetVersion(void)
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

    pInterface->pFnInitialize = TDNFMetalinkInitialize;
    pInterface->pFnEventsNeeded = TDNFMetalinkEventsNeeded;
    pInterface->pFnGetErrorString = TDNFMetalinkGetErrorString;
    pInterface->pFnEvent = TDNFMetalinkEvent;
    pInterface->pFnCloseHandle = TDNFMetalinkClose;

error:
    return dwError;
}

uint32_t
TDNFMetalinkInitialize(
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
TDNFFreeMetalinkData(
    PTDNF_METALINK_DATA pData
    )
{
    PTDNF_METALINK_DATA pTemp = NULL;
    while (pData)
    {
        pTemp = pData->pNext;
        TDNF_SAFE_FREE_MEMORY(pData->pszRepoId);
        TDNF_SAFE_FREE_MEMORY(pData->pszMetalink);
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
        TDNFFreeMetalinkData(pHandle->pData);
        TDNFFreeMemory(pHandle);
    }
}

/*
 * Metalink is only interested in repo events
 * this means all repo events will receive a
 * callback to the registered event callback function
*/
uint32_t
TDNFMetalinkEventsNeeded(
    const PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    )
{
    uint32_t dwError = 0;
    if (!pHandle || !pnEvents)
    {
        dwError = 1;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *pnEvents = TDNF_PLUGIN_EVENT_TYPE_REPO | TDNF_PLUGIN_EVENT_TYPE_REPO_MD;

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
TDNFMetalinkEvent(
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
            dwError = TDNFMetalinkReadConfig(pHandle, pContext);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else if (nEventType == TDNF_PLUGIN_EVENT_TYPE_REPO_MD)
    {
        if (nEventState == TDNF_PLUGIN_EVENT_STATE_DOWNLOAD &&
            nEventPhase == TDNF_PLUGIN_EVENT_PHASE_START)
        {
            dwError = TDNFMetalinkRepoMDDownloadStart(pHandle, pContext);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (nEventState == TDNF_PLUGIN_EVENT_STATE_DOWNLOAD &&
            nEventPhase == TDNF_PLUGIN_EVENT_PHASE_END)
        {
            dwError = TDNFMetalinkRepoMDDownloadEnd(pHandle, pContext);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else
    {
        pr_err("Unexpected event %d in %s plugin\n",
                pContext->nEvent, PLUGIN_NAME);
        goto cleanup;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFMetalinkGetErrorString(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t nErrorCode,
    char **ppszError
    )
{
    uint32_t dwError = 0;
    char *pszError = NULL;
    char *pszErrorPre = NULL;
    const char *pszGPGError = NULL;
    TDNF_ERROR_DESC arErrorDesc[] = METALINK_ERROR_TABLE;

    if (!pHandle || !ppszError)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* TODO */

    *ppszError = pszError;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszError);
    goto cleanup;
}

uint32_t
TDNFMetalinkClose(
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
