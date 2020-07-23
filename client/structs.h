/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : structs.h
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#pragma once

typedef struct _TDNF_PLUGIN_
{
    char *pszName;
    int nEnabled;
    void *pModule;
    PTDNF_PLUGIN_HANDLE pHandle;
    TDNF_PLUGIN_EVENT RegisterdEvts;
    TDNF_PLUGIN_INTERFACE stInterface;
    struct _TDNF_PLUGIN_ *pNext;
} TDNF_PLUGIN, *PTDNF_PLUGIN;

typedef struct _TDNF_REPO_DATA_INTERNAL_
{
    int nEnabled;
    int nSkipIfUnavailable;
    int nGPGCheck;
    long lMetadataExpire;
    char* pszId;
    char* pszName;
    char* pszBaseUrl;
    char* pszMetaLink;
    char* pszUrlGPGKey;
    char* pszUser;
    char* pszPass;

    struct _TDNF_REPO_DATA_INTERNAL_* pNext;
}TDNF_REPO_DATA_INTERNAL, *PTDNF_REPO_DATA_INTERNAL;

typedef struct _TDNF_
{
    PSolvSack pSack;
    PTDNF_CMD_ARGS pArgs;
    PTDNF_CONF pConf;
    PTDNF_REPO_DATA_INTERNAL pRepos;
    PTDNF_PLUGIN pPlugins;
}TDNF;

typedef struct _TDNF_CACHED_RPM_ENTRY
{
    char* pszFilePath;
    struct _TDNF_CACHED_RPM_ENTRY *pNext;
}TDNF_CACHED_RPM_ENTRY, *PTDNF_CACHED_RPM_ENTRY;

typedef struct _TDNF_CACHED_RPM_LIST
{
    int nSize;
    PTDNF_CACHED_RPM_ENTRY pHead;
}TDNF_CACHED_RPM_LIST, *PTDNF_CACHED_RPM_LIST;

typedef struct _TDNF_RPM_TS_
{
    int                     nQuiet;
    rpmts                   pTS;
    rpmtransFlags           nTransFlags;
    rpmprobFilterFlags      nProbFilterFlags;
    FD_t                    pFD;
    PTDNF_CACHED_RPM_LIST   pCachedRpmsArray;
}TDNFRPMTS, *PTDNFRPMTS;

typedef struct _TDNF_ENV_
{
    pthread_mutex_t mutexInitialize;
    int nInitialized;
}TDNF_ENV, *PTDNF_ENV;

typedef struct _TDNF_REPO_METADATA
{
    char *pszRepoCacheDir;
    char *pszRepo;
    char *pszRepoMD;
    char *pszPrimary;
    char *pszFileLists;
    char *pszUpdateInfo;
}TDNF_REPO_METADATA,*PTDNF_REPO_METADATA;

typedef struct _TDNF_EVENT_DATA_
{
    union
    {
        int nInt;
        const char *pcszStr;
        const void *pPtr;
    };
    TDNF_EVENT_ITEM_TYPE nType;
    const char *pcszName;
    struct _TDNF_EVENT_DATA_ *pNext;
}TDNF_EVENT_DATA, *PTDNF_EVENT_DATA;
