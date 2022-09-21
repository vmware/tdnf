/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_METALINK_PROTOTYPES_H__
#define __PLUGINS_METALINK_PROTOTYPES_H__

const char *
TDNFPluginGetVersion(
    );

const char *
TDNFPluginGetName(
    );

uint32_t
TDNFPluginLoadInterface(
    PTDNF_PLUGIN_INTERFACE pInterface
    );

uint32_t
TDNFMetalinkInitialize(
    const char *pszConfig,
    PTDNF_PLUGIN_HANDLE *ppHandle
    );

uint32_t
TDNFMetalinkEventsNeeded(
    const PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    );

uint32_t
TDNFMetalinkEvent(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFMetalinkGetErrorString(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t nErrorCode,
    char **ppszError
    );

uint32_t
TDNFMetalinkClose(
    PTDNF_PLUGIN_HANDLE pHandle
    );

void
TDNFFreeMetalinkData(
    PTDNF_METALINK_DATA pData
    );

void
FreePluginHandle(
    PTDNF_PLUGIN_HANDLE pHandle
    );

/* Metalink.c */
uint32_t
TDNFMetalinkReadConfig(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

#endif /* __PLUGINS_Metalink_PROTOTYPES_H__ */
