/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

//Metalink Structures.
typedef struct _TDNF_ML_LIST_
{
    struct _TDNF_ML_LIST_ *next;
    void* data;
} TDNF_ML_LIST, TDNF_ML_URL_LIST, TDNF_ML_HASH_LIST;

//Metalink hash info per hash type.
typedef struct _TDNF_ML_HASH_INFO_
{
    char *type;
    char *value;
} TDNF_ML_HASH_INFO;

//Metalink url info per hash type.
typedef struct _TDNF_ML_URL_INFO_
{
    char *protocol;
    char *type;
    char *location;
    char *url;
    int  preference;
} TDNF_ML_URL_INFO;

//Metalink global parsed info.
typedef struct _TDNF_ML_CTX_
{
    char           *filename;
    signed long    timestamp;
    signed long    size;
    TDNF_ML_LIST   *hashes;
    TDNF_ML_LIST   *urls;
} TDNF_ML_CTX;

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
} TDNF_PLUGIN_HANDLE;
