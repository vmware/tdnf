/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFAddEventDataString(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const char *pcszStr
    )
{
    uint32_t dwError = 0;
    PTDNF_EVENT_DATA pEvent = NULL;

    if (!pContext || IsNullOrEmptyString(pcszName) ||
        IsNullOrEmptyString(pcszStr))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(sizeof(*pEvent), 1, (void **)&pEvent);
    BAIL_ON_TDNF_ERROR(dwError);

    pEvent->nType = TDNF_EVENT_ITEM_TYPE_STRING;
    pEvent->pcszName = pcszName;
    pEvent->pcszStr = pcszStr;

    pEvent->pNext = pContext->pData;
    pContext->pData = pEvent;

cleanup:
    return dwError;

error:
    TDNFFreeEventData(pEvent);
    goto cleanup;
}

uint32_t
TDNFAddEventDataPtr(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const void *pPtr
    )
{
    uint32_t dwError = 0;
    PTDNF_EVENT_DATA pEvent = NULL;

    if (!pContext || IsNullOrEmptyString(pcszName) || !pPtr)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(sizeof(*pEvent), 1, (void **)&pEvent);
    BAIL_ON_TDNF_ERROR(dwError);

    pEvent->nType = TDNF_EVENT_ITEM_TYPE_PTR;
    pEvent->pcszName = pcszName;
    pEvent->pPtr = pPtr;

    pEvent->pNext = pContext->pData;
    pContext->pData = pEvent;

cleanup:
    return dwError;

error:
    TDNFFreeEventData(pEvent);
    goto cleanup;
}

uint32_t
TDNFEventContextGetItem(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    PTDNF_EVENT_DATA * const ppData
    )
{
    uint32_t dwError = 0;
    PTDNF_EVENT_DATA pData = NULL;

    if (!pContext || !pContext->pData || IsNullOrEmptyString(pcszName) || !ppData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pData = pContext->pData; pData; pData = pData->pNext)
    {
        if (strcmp(pcszName, pData->pcszName) == 0)
        {
            break;
        }
    }

    if (!pData)
    {
        dwError = ERROR_TDNF_EVENT_CTXT_ITEM_NOT_FOUND;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppData = pData;

error:
    return dwError;
}

uint32_t
TDNFEventContextGetItemPtr(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const void **ppPtr
    )
{
    uint32_t dwError = 0;
    PTDNF_EVENT_DATA pData = NULL;

    if (!pContext || !pContext->pData || IsNullOrEmptyString(pcszName) || !ppPtr)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFEventContextGetItem(pContext, pcszName, &pData);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pData || pData->nType != TDNF_EVENT_ITEM_TYPE_PTR || !pData->pPtr)
    {
        dwError = ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppPtr = pData->pPtr;
error:
    return dwError;
}

uint32_t
TDNFEventContextGetItemString(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const char **ppcszStr
    )
{
    uint32_t dwError = 0;
    PTDNF_EVENT_DATA pData = NULL;

    if (!pContext || !pContext->pData || IsNullOrEmptyString(pcszName) || !ppcszStr)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFEventContextGetItem(pContext, pcszName, &pData);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pData || pData->nType != TDNF_EVENT_ITEM_TYPE_STRING || !pData->pcszStr)
    {
        dwError = ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppcszStr = pData->pcszStr;
error:
    return dwError;
}

void
TDNFFreeEventData(
    PTDNF_EVENT_DATA pData
    )
{
    PTDNF_EVENT_DATA pTemp;
    while(pData)
    {
        pTemp = pData->pNext;
        TDNFFreeMemory(pData);
        pData = pTemp;
    }
}
