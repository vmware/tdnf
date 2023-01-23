/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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
TDNFReAllocateMemory(
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
TDNFStringSepCount(
    char *pszBuf,
    char *pszSep,
    size_t *nSepCount
    );

uint32_t
TDNFSplitStringToArray(
    char *pszBuf,
    char *pszSep,
    char ***pppszTokens
    );

uint32_t
TDNFJoinArrayToString(
    char **ppszArray,
    char *pszSep,
    int count,
    char **ppszResult
);

uint32_t
TDNFAllocateStringPrintf(
    char** ppszDst,
    const char* pszFmt,
    ...
    );

uint32_t
TDNFAllocateStringArray(
    char** ppszSrc,
    char*** pppszDst
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

uint32_t
TDNFStringEndsWith(
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

uint32_t
TDNFStringArrayCount(
    char **ppszStringArray,
    int *pnCount
    );

uint32_t
TDNFStringArraySort(
    char **ppszArray
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

uint32_t
TDNFReadKeyValueStringArray(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    char*** pppszValueList
    );

void
TDNFFreeConfigData(
    PCONF_DATA pData
    );

//utils.c
uint32_t
TDNFCreateAndWriteToFile(
    const char *pszFile,
    const char *data
    );

uint32_t
TDNFFileReadAllText(
    const char *pszFileName,
    char **ppszText,
    int *pnLength
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

uint32_t
TDNFNormalizePath(
    const char* pszPath,
    char** ppszNormalPath);

uint32_t
TDNFRecursivelyRemoveDir(
    const char *pszPath
);

uint32_t
TDNFStringMatchesOneOf(
    const char *pszSearch,
    char **ppszList,
    int *pRet);

uint32_t
TDNFJoinPath(
    char **ppszPath, ...);

uint32_t
TDNFReadFileToStringArray(
    const char *pszFile,
    char ***pppszArray
    );

uint32_t
TDNFIsDir(
    const char* pszPath,
    int* pnPathIsDir
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

void
GlobalSetQuiet(
    int32_t val
    );

void
GlobalSetJson(
    int32_t val
    );

void
log_console(
    int32_t loglevel,
    const char *format,
    ...
    );

int tdnflockAcquire(tdnflock lock);

void tdnflockRelease(tdnflock lock);

tdnflock tdnflockFree(tdnflock lock);

tdnflock
tdnflockNew(
    const char *lock_path,
    const char *descr
    );

tdnflock
tdnflockNewAcquire(
    const char *lock_path,
    const char *descr
    );

#endif /* __COMMON_PROTOTYPES_H__ */
