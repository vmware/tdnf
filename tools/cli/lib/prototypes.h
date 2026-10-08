/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

//installcmd.c
uint32_t
TDNFCliAlterWithPkgList(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    const char *pszCmdName,
    char **ppszPkgs,
    const char *pszDownloadDir,
    int nAlterType
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

//versionlock.c
uint32_t
DeriveLockPaths(
    PTDNF_CMD_ARGS pCmdArgs,
    char **ppszLocksDir,
    char **ppszLockFile
    );

uint32_t
TDNFCliLockList(
    const char *pszLockFile
    );

uint32_t
TDNFCliLockAdd(
    const char *pszLocksDir,
    const char *pszLockFile,
    char **ppszPkgs,
    int nPkgCount
    );

uint32_t
TDNFCliLockDelete(
    const char *pszLocksDir,
    const char *pszLockFile,
    char **ppszPkgs,
    int nPkgCount
    );

uint32_t
TDNFCliLockClear(
    const char *pszLocksDir,
    const char *pszLockFile
    );

//updateinfocmd.c
const char*
TDNFGetUpdateInfoType(
    int nType
    );

uint32_t
TDNFCliUpdateInfoSummary(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    );

uint32_t
TDNFCliUpdateInfoOutput(
    PTDNF_UPDATEINFO pInfo,
    TDNF_UPDATEINFO_OUTPUT mode
    );

uint32_t
TDNFCliUpdateInfoOutputJson(
    PTDNF_UPDATEINFO pInfo,
    TDNF_UPDATEINFO_OUTPUT mode
    );

//help.c
void
TDNFCliShowNoSuchOption(
    const char* pszOption
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
