/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

//invoke tdnf library methods
uint32_t
TDNFCliInvokeAlter(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    );

uint32_t
TDNFCliInvokeCheck(
    PTDNF_CLI_CONTEXT pContext
    );

uint32_t
TDNFCliInvokeCheckLocal(
    PTDNF_CLI_CONTEXT pContext,
    const char *pszFolder
    );

uint32_t
TDNFCliInvokeCheckUpdate(
    PTDNF_CLI_CONTEXT pContext,
    char** ppszPackageArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    );

uint32_t
TDNFCliInvokeClean(
    PTDNF_CLI_CONTEXT pContext,
    uint32_t nCleanType
    );

uint32_t
TDNFCliInvokeCount(
    PTDNF_CLI_CONTEXT pContext,
    uint32_t *pnCount
    );

uint32_t
TDNFCliInvokeInfo(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_LIST_ARGS pInfoArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    );

uint32_t
TDNFCliInvokeList(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_LIST_ARGS pListArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    );

uint32_t
TDNFCliInvokeProvides(
    PTDNF_CLI_CONTEXT pContext,
    const char *pszProvides,
    PTDNF_PKG_INFO *ppPkgInfos
    );

uint32_t
TDNFCliInvokeRepoList(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA *ppRepos
    );

uint32_t
TDNFCliInvokeRepoSync(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_REPOSYNC_ARGS pReposyncArgs
    );

uint32_t
TDNFCliInvokeRepoQuery(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs,
    PTDNF_PKG_INFO *ppPkgInfos,
    uint32_t *pdwCount
    );

uint32_t
TDNFCliInvokeResolve(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO *ppSolvedPkgInfo
    );

uint32_t
TDNFCliInvokeSearch(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    );

uint32_t
TDNFCliInvokeHistoryList(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_HISTORY_ARGS pHistoryArgs,
    PTDNF_HISTORY_INFO *ppHistoryInfo
);

uint32_t
TDNFCliInvokeHistoryResolve(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_HISTORY_ARGS pHistoryArgs,
    PTDNF_SOLVED_PKG_INFO *ppSolvedPkgInfo
);

uint32_t
TDNFCliInvokeHistoryGetId(
    PTDNF_CLI_CONTEXT pContext,
    int *pnId
);

uint32_t
TDNFCliInvokeAlterHistory(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    PTDNF_HISTORY_ARGS pHistoryArgs
    );

uint32_t
TDNFCliInvokeGetPackageUrls(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    char ***pppszUrls,
    int *pnCount
);

void
TDNFCliShowNoSuchOption(
    const char* pszOption
    );

//installcmd.c

uint32_t
TDNFCliInvokeUpdateInfo(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_UPDATEINFO_ARGS pInfoArgs,
    PTDNF_UPDATEINFO *ppUpdateInfo
    );

uint32_t
TDNFCliInvokeUpdateInfoSummary(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_AVAIL nAvail,
    PTDNF_UPDATEINFO_ARGS pInfoArgs,
    PTDNF_UPDATEINFO_SUMMARY *ppSummary
    );

//main.c
void
TDNFCliShowVersion(
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliVerboseShowEnv(
    PTDNF_CMD_ARGS pCmdArgs
    );

//options.c
uint32_t
_TDNFCliGetOptionByName(
    const char* pszName,
    struct option* pKnownOptions,
    struct option** ppOption
    );

uint32_t
TDNFCliValidateOptionName(
    const char* pszOptionName,
    struct option* pKnownOptions
    );

uint32_t
TDNFCliValidateOptionArg(
    const char* pszOption,
    const char* pszArg,
    struct option* pKnownOptions
    );

uint32_t
TDNFCliValidateOptions(
    const char* pszOption,
    const char* pszArg,
    struct option* pKnownOptions
    );

//parseargs.c
uint32_t
TDNFCopyOptions(
    PTDNF_CMD_ARGS pOptionArgs,
    PTDNF_CMD_ARGS pArgs
    );

uint32_t
ParseOption(
    const char* pszName,
    const char* pszArg,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
ParseRpmVerbosity(
    const char* pszVerbosity,
    int* pnVerbosity
    );

uint32_t
HandleOptionsError(
    const char* pszName,
    const char* pszArg,
    struct option* pstOptions
    );

//parsecleanargs.c
uint32_t
ParseCleanType(
    const char* pszCleanType,
    uint32_t* pnCleanType
    );

//parselistargs.c
uint32_t
ParseScope(
    const char* pszScope,
    TDNF_SCOPE* pnScope
    );

//parserepolistargs.c
uint32_t
ParseFilter(
    const char* pszRepolistFilter,
    TDNF_REPOLISTFILTER* pnFilter
    );

//parseupdateinfo.c
uint32_t
ParseMode(
    const char* pszOutMode,
    TDNF_UPDATEINFO_OUTPUT* pnOutMode
    );

//updateinfocmd.c
char*
TDNFGetUpdateInfoType(
    int nType
    );

uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    );

uint32_t
TDNFCliInvokeMark(
    PTDNF_CLI_CONTEXT pContext,
    char **ppszPkgNameSpecs,
    uint32_t nValue
    );
