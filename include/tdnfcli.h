/*
 * Copyright (C) 2017-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef _TDNF_CLI_H_
#define _TDNF_CLI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "tdnfclitypes.h"
#include "tdnfclierror.h"

//api.c
uint32_t
TDNFCliParseArgs(
    int argc,
    char** argv,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

uint32_t
TDNFCliParseScope(
    const char* pszScope,
    TDNF_SCOPE* pnScope
    );

uint32_t
TDNFCliParseHistoryArgs(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_HISTORY_ARGS* ppHistoryArgs
    );

void
TDNFCliFreeHistoryArgs(
    PTDNF_HISTORY_ARGS pHistoryArgs
    );

uint32_t
TDNFCliParseListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    );

uint32_t
TDNFCliParseInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    );

void
TDNFCliFreeListArgs(
    PTDNF_LIST_ARGS pListArgs
    );

uint32_t
TDNFCliParseCleanType(
    const char* pszCleanType,
    TDNF_CLEANTYPE* pnCleanType
    );

uint32_t
TDNFCliParseCleanArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_CLEANTYPE* pnCleanType
    );

uint32_t
TDNFCliParseFilter(
    const char* pszRepolistFilter,
    TDNF_REPOLISTFILTER* pnFilter
    );

uint32_t
TDNFCliParseRepoListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_REPOLISTFILTER* pnFilter
    );

uint32_t
TDNFCliParseRepoSyncArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_REPOSYNC_ARGS* ppReposyncArgs
    );

uint32_t
TDNFCliParseRepoQueryArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_REPOQUERY_ARGS* ppRepoqueryArgs
    );

uint32_t
TDNFCliParseMode(
    const char* pszOutMode,
    TDNF_UPDATEINFO_OUTPUT* pnOutMode
    );

uint32_t
TDNFCliParseUpdateInfoArgs(
   PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS* ppUpdateInfoArgs
    );

uint32_t
TDNFCliParsePackageArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    char ***pppszPackageArgs,
    const int *pnPackageCount
    );

uint32_t
TDNFCliPrintError(
    uint32_t dwErrorCode,
    int doJson
    );

uint32_t
TDNFCliGetErrorString(
    uint32_t dwErrorCode,
    char** ppszError
    );

void
TDNFCliFreeUpdateInfoArgs(
    PTDNF_UPDATEINFO_ARGS pUpdateInfoArgs
    );

void
TDNFCliFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    );

uint32_t
TDNFCliRefresh(
    PTDNF_CLI_CONTEXT pContext
);

void
TDNFCliFreeRepoSyncArgs(
    PTDNF_REPOSYNC_ARGS pReposyncArgs
    );

void
TDNFCliFreeRepoQueryArgs(
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs
    );

//Commands
uint32_t
TDNFCliAutoEraseCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCleanCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCountCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliListCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliInfoCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliSearchCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliRepoListCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCheckCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCheckLocalCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCheckUpdateCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliMakeCacheCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliProvidesCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliRepoSyncCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliRepoQueryCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliUpdateInfoCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliHelpCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliHistoryCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

//installcmd.c
uint32_t
TDNFCliDowngradeCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliEraseCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliInstallCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliReinstallCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliDistroSyncCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliUpgradeCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliAskAndAlter(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
);

uint32_t
TDNFCliAlterCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nType
    );

uint32_t
PrintSolvedInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    );

uint32_t
PrintSolvedInfoJson(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    );

uint32_t
PrintNotAvailable(
    char** ppszPkgsNotAvailable
    );

uint32_t
PrintExistingPackagesSkipped(
    PTDNF_PKG_INFO pPkgInfos
    );

uint32_t
PrintNotAvailablePackages(
    PTDNF_PKG_INFO pPkgInfos
    );

uint32_t
PrintAction(
    PTDNF_PKG_INFO pPkgInfos,
    TDNF_ALTERTYPE nAlterType
    );

//updateinfocmd.c


//output.c
void
ShowConsoleProps(
    );

uint32_t
GetConsoleWidth(
    int* pConsoleWidth
    );

uint32_t
GetColumnWidths(
    int nCount,
    const int *pnColPercents,
    int *pnColWidths
    );

void
TDNFCliShowUsage(
    void
    );

void
TDNFCliShowHelp(
    void
    );

void
TDNFCliShowNoSuchCommand(
    const char* pszCmd
    );

#ifdef __cplusplus
}
#endif

#endif//TDNF_H_
