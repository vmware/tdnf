/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

typedef struct _TDNF_REPO_GPG_CHECK_DATA_
{
    char *pszRepoId;
    struct _TDNF_REPO_GPG_CHECK_DATA_ *pNext;
}TDNF_REPO_GPG_CHECK_DATA, *PTDNF_REPO_GPG_CHECK_DATA;

typedef struct _TDNF_PLUGIN_HANDLE_
{
    PTDNF pTdnf;
    uint32_t nError; /* last error set by this plugin */
    uint32_t nGPGError; /* gpg specific error. gpgerror will provide details */
    PTDNF_REPO_GPG_CHECK_DATA pData;
}TDNF_PLUGIN_HANDLE;
