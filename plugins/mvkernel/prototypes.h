/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __PLUGINS_MVKERNEL_PROTOTYPES_H__
#define __PLUGINS_MVKERNEL_PROTOTYPES_H__

int32_t bindmount(void);
int32_t removedir(const char *path);
int32_t mv_running_kernel(const char *pkgname);
int32_t mvdir(const char *src, const char *dst);
int32_t copy_file(const char *src, const char *dst);
int32_t get_kern_version(char *buf, int32_t bufsize);

const char *TDNFPluginGetName(void);
const char *TDNFPluginGetVersion(void);

uint32_t
TDNFPluginLoadInterface(
    PTDNF_PLUGIN_INTERFACE pInterface
    );

uint32_t
TDNFMvKernelInit(
    const char *pszConfig,
    PTDNF_PLUGIN_HANDLE *ppHandle
    );

uint32_t
TDNFMvKernelEventsNeeded(
    PTDNF_PLUGIN_HANDLE pHandle,
    TDNF_PLUGIN_EVENT_TYPE *pnEvents
    );

uint32_t
TDNFMvKernelEvent(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    );

uint32_t
TDNFMvKernelGetErrStr(
    PTDNF_PLUGIN_HANDLE pHandle,
    uint32_t nErrorCode,
    char **ppszError
    );

uint32_t
TDNFMvKernelClose(
    PTDNF_PLUGIN_HANDLE pHandle
    );

#endif /* __PLUGINS_MVKERNEL_PROTOTYPES_H__ */
