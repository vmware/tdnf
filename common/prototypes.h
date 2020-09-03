/*
 * Copyright (C) 2015-2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header   : prototypes.h
 *
 * Abstract :
 *
 *            commonlib
 *
 *            common library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#ifndef __COMMON_PROTOTYPES_H__
#define __COMMON_PROTOTYPES_H__

//memory.c
uint32_t
TDNFAllocateMemory(
    size_t nNumElements,
    size_t nSize,
    void** ppMemory
    );

uint32_t
TDNFAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

void
TDNFFreeMemory(
    void* pMemory
    );

//strings.c
uint32_t
TDNFAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

uint32_t
TDNFSafeAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

uint32_t
TDNFAllocateStringPrintf(
    char** ppszDst,
    const char* pszFmt,
    ...
    );

uint32_t
TDNFAllocateStringN(
    const char* pszSrc,
    uint32_t dwNumElements,
    char** ppszDst
    );

uint32_t
TDNFReplaceString(
    const char* pszSource,
    const char* pszSearch,
    const char* pszReplace,
    char** ppszDst
    );

uint32_t
TDNFTrimSuffix(
    char* pszSource,
    const char* pszSuffix
    );

void
TDNFFreeStringArray(
    char** ppszArray
    );

void
TDNFFreeStringArrayWithCount(
    char **ppszArray,
    int nCount
    );

//configreader.c
void
TDNFPrintConfigData(
    PCONF_DATA pData
    );

uint32_t
TDNFReadConfigFile(
    const char *pszFile,
    const int nMaxLineLength,
    PCONF_DATA *ppData
    );

uint32_t
TDNFConfigGetSection(
    PCONF_DATA pData,
    const char *pszGroup,
    PCONF_SECTION *ppSection
    );

uint32_t
TDNFReadKeyValue(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    const char* pszDefault,
    char** ppszValue
    );

uint32_t
TDNFReadKeyValueBoolean(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    );

uint32_t
TDNFReadKeyValueInt(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    );

void
TDNFFreeConfigData(
    PCONF_DATA pData
    );

//utils.c
uint32_t
TDNFFileReadAllText(
    const char *pszFileName,
    char **ppszText
    );

const char *
TDNFLeftTrim(
    const char *pszStr
    );

const char *
TDNFRightTrim(
    const char *pszStart,
    const char *pszEnd
    );

uint32_t
TDNFUtilsFormatSize(
    uint64_t unSize,
    char** ppszFormattedSize
    );

void
TDNFFreePackageInfoContents(
    PTDNF_PKG_INFO pPkgInfo
    );

uint32_t
TDNFUtilsMakeDirs(
    const char* pszPath
    );

uint32_t
TDNFYesOrNo(
    PTDNF_CMD_ARGS pArgs,
    const char *pszQuestion,
    int *pAnswer
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
TDNFHasOpt(
    PTDNF_CMD_ARGS pArgs,
    const char *pszOptName,
    int *pnHasOpt
    );

uint32_t
TDNFSetOpt(
    PTDNF_CMD_ARGS pArgs,
    const char *pszOptName,
    const char *pszOptValue
    );

uint32_t
TDNFSetOptNoOverwrite(
    PTDNF_CMD_ARGS pArgs,
    const char *pszOptName,
    const char *pszOptValue
    );

uint32_t
TDNFGetCmdOptValue(
    PTDNF_CMD_ARGS pArgs,
    const char *pszOptName,
    char **ppszOptValue
    );

uint32_t
TDNFGetOptWithDefault(
    PTDNF_CMD_ARGS pArgs,
    const char *pszOptName,
    const char *pszDefault,
    char **ppszOptValue
    );
#endif /* __COMMON_PROTOTYPES_H__ */
