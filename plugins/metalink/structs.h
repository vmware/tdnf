/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

/* per repo */
typedef struct _TDNF_METALINK_DATA_
{
    struct _TDNF_METALINK_DATA_ *pNext;
    char *pszRepoId;
    char *pszMetalink;
    TDNF_ML_CTX *ml_ctx;
} TDNF_METALINK_DATA, *PTDNF_METALINK_DATA;

typedef struct _TDNF_PLUGIN_HANDLE_
{
    PTDNF pTdnf;
    uint32_t nError; /* last error set by this plugin */
    PTDNF_METALINK_DATA pData;
} TDNF_PLUGIN_HANDLE, *PTDNF_PLUGIN_HANDLE;
