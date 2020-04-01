/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static uint32_t
FreePluginHandle(
    PTDNF_PLUGIN_HANDLE pHandle
    );

TDNF_PLUGIN_INTERFACE _interface = {0};

const char *TDNFPluginGetVersion(void)
{
    return PLUGIN_VERSION;
}

const char *TDNFPluginGetName(void)
{
    return PLUGIN_NAME;
}

uint32_t
TDNFPluginLoadInterface(
    PTDNF_PLUGIN_INTERFACE pInterface
    )
{
    if (!pInterface)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    pInterface->pFnEvent = TDNFMvKernelEvent;
    pInterface->pFnInitialize = TDNFMvKernelInit;
    pInterface->pFnCloseHandle = TDNFMvKernelClose;
    pInterface->pFnGetErrorString = TDNFMvKernelGetErrStr;
    pInterface->pFnEventsNeeded = TDNFMvKernelEventsNeeded;

    return 0;
}

uint32_t
TDNFMvKernelInit(
    const char *pszConfig,
    PTDNF_PLUGIN_HANDLE *ppHandle
    )
{
    uint32_t dwError = 0;
    PTDNF_PLUGIN_HANDLE pHandle = NULL;

    /* plugin does not expect config */
    if (!ppHandle)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    UNUSED(pszConfig);

    dwError = TDNFAllocateMemory(sizeof(*pHandle), 1, (void **)&pHandle);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppHandle = pHandle;

error:
    if (dwError && pHandle)
    {
        FreePluginHandle(pHandle);
    }

    return dwError;
}

static uint32_t
FreePluginHandle(
    PTDNF_PLUGIN_HANDLE pHandle
    )
{
    if (!pHandle)
    {
        return 1;
    }

    TDNF_SAFE_FREE_MEMORY(pHandle);
    return 0;
}

uint32_t
TDNFMvKernelEventsNeeded(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    )
{
    if (!pHandle || !pnEvents)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }
    *pnEvents = TDNF_PLUGIN_EVENT_TYPE_KERN_INSTL;

    return 0;
}

uint32_t
TDNFMvKernelEvent(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    TDNF_PLUGIN_EVENT_TYPE nEventType;
    TDNF_PLUGIN_EVENT_STATE nEventState;
    TDNF_PLUGIN_EVENT_PHASE nEventPhase;

    if (!pHandle || !pContext)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
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
    }
    else if (nEventType == TDNF_PLUGIN_EVENT_TYPE_KERN_INSTL)
    {
        if (nEventState == TDNF_PLUGIN_EVENT_STATE_MOVE &&
            nEventPhase == TDNF_PLUGIN_EVENT_PHASE_START)
        {
            char *pkgname = NULL;

            dwError = TDNFEventContextGetItemString(
                            pContext,
                            TDNF_EVENT_ITEM_KERN_UPDATE,
                            (const char **)&pkgname);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = mv_running_kernel(pkgname);
        } else if (nEventState == TDNF_PLUGIN_EVENT_STATE_MOUNT &&
                   nEventPhase == TDNF_PLUGIN_EVENT_PHASE_END)
        {
            dwError = bindmount();
        }
    }
    else
    {
        fprintf(stderr, "Unexpected event %d in %s plugin\n",
                pContext->nEvent, PLUGIN_NAME);
        return ERR_TDNF_MVKERNEL_UNKNWN;
    }

error:
    return dwError;
}

uint32_t
TDNFMvKernelGetErrStr(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t nErrorCode,
    char **ppszError
    )
{
    size_t i = 0;
    uint32_t dwError = 0;
    char *pszError = NULL;
    char *pszErrorPre = NULL;
    TDNF_ERROR_DESC arErrorDesc[] = MVKERNEL_ERR_TABLE;

    if (!pHandle || !ppszError)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    for (i = 0; i < sizeof(arErrorDesc)/sizeof(arErrorDesc[0]); i++)
    {
        if (nErrorCode == (uint32_t)arErrorDesc[i].nCode)
        {
            pszErrorPre = arErrorDesc[i].pszDesc;
            break;
        }
    }

    if (!pszErrorPre)
    {
        pszErrorPre = "unknown error";
    }

    dwError = TDNFAllocateStringPrintf(&pszError, "%s: %s\n",
                                       MVKERNEL_PLUGIN_ERR, pszErrorPre);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszError = pszError;

error:
    if (dwError)
    {
        TDNF_SAFE_FREE_MEMORY(pszError);
    }

    return dwError;
}

uint32_t
TDNFMvKernelClose(
    PTDNF_PLUGIN_HANDLE pHandle
    )
{
    if (!pHandle)
    {
        return 1;
    }

    return FreePluginHandle(pHandle);
}
