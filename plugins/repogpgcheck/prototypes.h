/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_REPOGPGCHECK_PROTOTYPES_H__
#define __PLUGINS_REPOGPGCHECK_PROTOTYPES_H__

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
TDNFRepoGPGCheckInitialize(
    const char *pszConfig,
    PTDNF_PLUGIN_HANDLE *ppHandle
    );

uint32_t
TDNFRepoGPGCheckEventsNeeded(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    );

uint32_t
TDNFRepoGPGCheckEvent(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFRepoGPGCheckGetErrorString(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t nErrorCode,
    char **ppszError
    );

uint32_t
TDNFRepoGPGCheckClose(
    PTDNF_PLUGIN_HANDLE pHandle
    );

void
TDNFFreeRepoGPGCheckData(
    PTDNF_REPO_GPG_CHECK_DATA pData
    );

void
FreePluginHandle(
    PTDNF_PLUGIN_HANDLE pHandle
    );

/* repogpgcheck.c */
uint32_t
TDNFRepoGPGCheckVerifyVersion(
    );

uint32_t
TDNFRepoMDCheckSignature(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFRepoGPGCheckReadConfig(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFDownloadFile(
    PTDNF pTdnf,
    const char *pszRepo,
    const char *pszFileUrl,
    const char *pszFile,
    const char *pszProgressData
    );

#endif /* __PLUGINS_REPOGPGCHECK_PROTOTYPES_H__ */
