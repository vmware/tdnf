/*
 * Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_METALINK_PROTOTYPES_H__
#define __PLUGINS_METALINK_PROTOTYPES_H__

// utils.c

uint32_t
TDNFMetalinkParseFile(
    TDNF_ML_CTX *ml_ctx,
    int fd,
    const char *filename
    );

void
TDNFMetalinkFree(
    TDNF_ML_CTX *ml_ctx
    );

uint32_t
TDNFXmlParseData(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node,
    const char *filename
    );

uint32_t
TDNFParseFileTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node,
    const char *filename
    );

uint32_t
TDNFParseHashTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node
    );


uint32_t
TDNFParseUrlTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node
    );

uint32_t
TDNFCheckRepoMDFileHashFromMetalink(
    const char *pszFile,
    TDNF_ML_CTX *ml_ctx
    );

// api.c

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

// list.c
void
TDNFSortListOnPreference(
    TDNF_ML_LIST** headUrl
);

uint32_t
TDNFAppendList(
    TDNF_ML_LIST** head_ref,
    void *new_data
);

void
TDNFDeleteList(
    TDNF_ML_LIST** head_ref,
    TDNF_ML_FREE_FUNC free_func
);

/* metalink.c */
uint32_t
TDNFMetalinkReadConfig(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFMetalinkRepoMDDownloadStart(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFMetalinkRepoMDDownloadEnd(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

#endif /* __PLUGINS_Metalink_PROTOTYPES_H__ */
