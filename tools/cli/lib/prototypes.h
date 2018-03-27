/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
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
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_UPDATEINFO_ARGS pInfoArgs
    );

uint32_t
TDNFCliUpdateInfoInfo(
    PTDNF_UPDATEINFO pInfo
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

//setopt.c
uint32_t
AddSetOpt(
    PTDNF_CMD_ARGS pCmdArgs,
    const char* pszOptArg
    );

uint32_t
AddSetOptWithValues(
    PTDNF_CMD_ARGS pCmdArgs,
    int nType,
    const char* pszOptArg,
    const char* pszOptValue
    );

uint32_t
GetOptionAndValue(
    const char* pszOptArg,
    PTDNF_CMD_OPT* ppCmdOpt
    );

uint32_t
NumPkgsToExclude(
    const char* pszExculde,
    uint32_t *pdwLength
    );

uint32_t
ParseExcludes(
    const char* pszExclude,
    uint32_t* pdwPkgsToExclude,
    char*** pppszExclude
    );
