/*
 * Copyright (C) 2017 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef _TDNF_CLI_TYPES_H_
#define _TDNF_CLI_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tdnftypes.h"

typedef void * HTDNF;

typedef struct _TDNF_CLI_CONTEXT_ *PTDNF_CLI_CONTEXT;

typedef uint32_t (*PFN_CMD)(PTDNF_CLI_CONTEXT, PTDNF_CMD_ARGS);

//Map command name to client function
typedef struct _TDNF_CLI_CMD_MAP
{
    char* pszCmdName;
    PFN_CMD pFnCmd;
}TDNF_CLI_CMD_MAP, *PTDNF_CLI_CMD_MAP;

typedef struct _TDNF_LIST_ARGS
{
    TDNF_SCOPE nScope;
    char** ppszPackageNameSpecs;
}TDNF_LIST_ARGS, *PTDNF_LIST_ARGS;

typedef struct _TDNF_UPDATEINFO_ARGS
{
    TDNF_UPDATEINFO_OUTPUT nMode;
    TDNF_SCOPE nScope;
    TDNF_UPDATEINFO_TYPE nType;
    char** ppszPackageNameSpecs;
}TDNF_UPDATEINFO_ARGS, *PTDNF_UPDATEINFO_ARGS;

typedef uint32_t
(*PFN_TDNF_ALTER)(
    PTDNF_CLI_CONTEXT,
    TDNF_ALTERTYPE,
    PTDNF_SOLVED_PKG_INFO);

typedef uint32_t
(*PFN_TDNF_CHECK)(
    PTDNF_CLI_CONTEXT);

typedef uint32_t
(*PFN_TDNF_CHECK_LOCAL)(
    PTDNF_CLI_CONTEXT,
    const char *);

typedef uint32_t
(*PFN_TDNF_CHECK_UPDATE)(
    PTDNF_CLI_CONTEXT,
    char **,
    PTDNF_PKG_INFO *,
    uint32_t *
    );

typedef uint32_t
(*PFN_TDNF_CLEAN)(
    PTDNF_CLI_CONTEXT,
    TDNF_CLEANTYPE,
    PTDNF_CLEAN_INFO *
    );

typedef uint32_t
(*PFN_TDNF_COUNT)(
    PTDNF_CLI_CONTEXT,
    uint32_t *);

typedef uint32_t
(*PFN_TDNF_INFO)(
    PTDNF_CLI_CONTEXT,
    PTDNF_LIST_ARGS,
    PTDNF_PKG_INFO *,
    uint32_t *
    );

typedef uint32_t
(*PFN_TDNF_LIST)(
    PTDNF_CLI_CONTEXT,
    PTDNF_LIST_ARGS,
    PTDNF_PKG_INFO *,
    uint32_t *
    );

typedef uint32_t
(*PFN_TDNF_PROVIDES)(
    PTDNF_CLI_CONTEXT,
    const char *,
    PTDNF_PKG_INFO *
    );

typedef uint32_t
(*PFN_TDNF_REPOLIST)(
    PTDNF_CLI_CONTEXT,
    TDNF_REPOLISTFILTER,
    PTDNF_REPO_DATA *
    );

typedef uint32_t
(*PFN_TDNF_REPOSYNC)(
    PTDNF_CLI_CONTEXT,
    PTDNF_REPOSYNC_ARGS
    );

typedef uint32_t
(*PFN_TDNF_REPOQUERY)(
    PTDNF_CLI_CONTEXT,
    PTDNF_REPOQUERY_ARGS,
    PTDNF_PKG_INFO *,
    uint32_t *
    );

typedef uint32_t
(*PFN_TDNF_RESOLVE)(
    PTDNF_CLI_CONTEXT,
    TDNF_ALTERTYPE,
    PTDNF_SOLVED_PKG_INFO *);

typedef uint32_t
(*PFN_TDNF_SEARCH)(
    PTDNF_CLI_CONTEXT,
    PTDNF_CMD_ARGS,
    PTDNF_PKG_INFO *,
    uint32_t *);

typedef uint32_t
(*PFN_TDNF_UPDATEINFO)(
    PTDNF_CLI_CONTEXT,
    PTDNF_UPDATEINFO_ARGS,
    PTDNF_UPDATEINFO *);

typedef uint32_t
(*PFN_TDNF_UPDATEINFO_SUMMARY)(
    PTDNF_CLI_CONTEXT,
    TDNF_AVAIL,
    PTDNF_UPDATEINFO_ARGS,
    PTDNF_UPDATEINFO_SUMMARY *);

typedef struct _TDNF_CLI_CONTEXT_
{
    HTDNF hTdnf;
    void *pUserData;

    PFN_TDNF_ALTER              pFnAlter;
    PFN_TDNF_CHECK_LOCAL        pFnCheckLocal;
    PFN_TDNF_CHECK_UPDATE       pFnCheckUpdate;
    PFN_TDNF_CHECK              pFnCheck;
    PFN_TDNF_CLEAN              pFnClean;
    PFN_TDNF_COUNT              pFnCount;
    PFN_TDNF_INFO               pFnInfo;
    PFN_TDNF_LIST               pFnList;
    PFN_TDNF_PROVIDES           pFnProvides;
    PFN_TDNF_REPOLIST           pFnRepoList;
    PFN_TDNF_REPOSYNC           pFnRepoSync;
    PFN_TDNF_REPOQUERY          pFnRepoQuery;
    PFN_TDNF_RESOLVE            pFnResolve;
    PFN_TDNF_SEARCH             pFnSearch;
    PFN_TDNF_UPDATEINFO         pFnUpdateInfo;
    PFN_TDNF_UPDATEINFO_SUMMARY pFnUpdateInfoSummary;
}TDNF_CLI_CONTEXT, *PTDNF_CLI_CONTEXT;

#ifdef __cplusplus
}
#endif

#endif//TDNF_CLI_TYPES_H_
