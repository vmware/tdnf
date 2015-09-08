/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : prototypes.h
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#pragma once

uint32_t
TDNFCliAutoEraseCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCleanCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCountCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliEraseCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliListCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliInfoCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliInstallCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliSearchCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliRepoListCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliUpgradeCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliDowngradeCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliReinstallCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliAlterCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nType
    );

uint32_t
TDNFCliCheckLocalCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliCheckUpdateCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliMakeCacheCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliProvidesCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

uint32_t
TDNFCliUpdateInfoCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );
//help.c
void
TDNFCliShowUsage(
    );

void
TDNFCliShowHelp(
    );

void
TDNFCliShowOptionsUsage(
    );

void
TDNFCliShowNoSuchCommand(
    const char* pszCmd
    );

void
TDNFCliShowNoSuchOption(
    const char* pszOption
    );

uint32_t
TDNFCliHelpCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    );

//installcmd.c
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

//main.c
uint32_t
PrintError(
    uint32_t dwErrorCode
    );

uint32_t
TDNFCliGetErrorString(
    uint32_t dwErrorCode,
    char** ppszError
    );

void
TDNFCliShowVersion(
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

//output.c
void
ShowConsoleProps(
    );

uint32_t
GetConsoleWidth(
    int* pnWidth
    );

int
CalculateColumnWidth(
    int nTotalWidth,
    int nRequestedPercent,
    int nMinVal
    );

uint32_t
GetColumnWidths(
    int nCount,
    int* pnColPercents,
    int* pnColWidths
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

uint32_t
TDNFCliParseArgs(
    int argc,
    char* const* argv,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

uint32_t
TDNFCliParsePackageArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    char*** pppszPackageArgs,
    int* pnPackageCount
    );

//parsecleanargs.c
uint32_t
ParseCleanType(
    const char* pszCleanType,
    TDNF_CLEANTYPE* pnCleanType
    );

uint32_t
TDNFCliParseCleanArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_CLEANTYPE* pnCleanType
    );

//parselistargs.c
uint32_t
ParseScope(
    const char* pszScope,
    TDNF_SCOPE* pnScope
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

//parserepolistargs.c
uint32_t
ParseFilter(
    const char* pszRepolistFilter,
    TDNF_REPOLISTFILTER* pnFilter
    );

uint32_t
TDNFCliParseRepoListArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_REPOLISTFILTER* pnFilter
    );

//parseupdateinfo.c
uint32_t
ParseMode(
    const char* pszOutMode,
    TDNF_UPDATEINFO_OUTPUT* pnOutMode
    );

uint32_t
TDNFCliParseUpdateInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS* ppUpdateInfoArgs
    );

void
TDNFFreeUpdateInfoArgs(
    PTDNF_UPDATEINFO_ARGS pUpdateInfoArgs
    );

//updateinfocmd.c
char*
TDNFGetUpdateInfoType(
    int nType
    );

uint32_t
TDNFCliUpdateInfoList(
    PTDNF_UPDATEINFO pUpdateInfo
    );

uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    );

void
TDNFFreeListArgs(
    PTDNF_LIST_ARGS pListArgs
    );
