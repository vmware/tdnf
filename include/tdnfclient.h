/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Header : tdnfclient.h
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            public header
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
*/
#ifndef _TDNF_CLIENT_H_
#define _TDNF_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tdnftypes.h"
#include "tdnferror.h"

//config.c
int
TDNFConfGetRpmVerbosity(
    PTDNF pTdnf
    );

uint32_t
TDNFConfSetFlag(
    TDNF_CONF_FLAG nFlag,
    int nValue //0 or 1
    );

uint32_t
TDNFConfGetFlag(
    TDNF_CONF_FLAG nFlag,
    int nValue //0 or 1
    );

uint32_t
TDNFConfSetValue(
    TDNF_CONF_TYPE nType,
    const char* pszValue
    );

uint32_t
TDNFConfGetValue(
    TDNF_CONF_TYPE nType,
    char** ppszValue
    );

//init.c
uint32_t
TDNFOpenHandle(
    PTDNF_CMD_ARGS pArgs,
    PTDNF* pTdnf
    );

void
TDNFCloseHandle(
    PTDNF pTdnf
    );

uint32_t
TDNFCloneCmdArgs(
    PTDNF_CMD_ARGS pCmdArgsIn,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

void
TDNFFreeCmdArgs(
    PTDNF_CMD_ARGS pCmdArgs
    );

//command APIs
uint32_t
TDNFCheckUpdates(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFClean(
    PTDNF pTdnf,
    TDNF_CLEANTYPE nCleanType,
    PTDNF_CLEAN_INFO* ppCleanInfo
    );

uint32_t
TDNFList(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFInfo(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgListInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFRepoList(
    PTDNF pTdnf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA* ppRepoData
    );

uint32_t
TDNFMakeCache(
    PTDNF pTdnf
    );

uint32_t
TDNFCheckLocalPackages(
    PTDNF pTdnf,
    const char* pszLocalPath
    );

uint32_t
TDNFProvides(
    PTDNF pTdnf,
    const char* pszSpec,
    PTDNF_PKG_INFO* ppPkgInfo
    );

uint32_t
TDNFUpdateInfoSummary(
    PTDNF pTdnf,
    TDNF_AVAIL nAvail,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO_SUMMARY* ppSummary
    );

uint32_t
TDNFUpdateInfo(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    TDNF_AVAIL nAvail,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO* ppUpdateInfo
    );

void
TDNFFreeCleanInfo(
    PTDNF_CLEAN_INFO pCleanInfo
    );
//client.c
uint32_t
TDNFCountCommand(
    PTDNF pTdnf,
    uint32_t* pdwCount
    );

uint32_t
TDNFSearchCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFResolve(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO* ppSolvedPkgInfo
    );

uint32_t
TDNFInstallCommand(
    PTDNF pTdnf
    );

uint32_t
TDNFAlterCommand(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    ); 

//rpmtrans
uint32_t
TDNFRpmExecTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    );

//validate
uint32_t
TDNFValidateCmdArgs(
    PTDNF pTdnf
    );

//memory.c
uint32_t
TDNFAllocateMemory(
    size_t size,
    void** ppMemory
    );

void
TDNFFreeMemory(
    void* pMemory
    );

void
TDNFFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    );

void
TDNFFreePackageInfo(
    PTDNF_PKG_INFO pPkgInfo
    );

void
TDNFFreePackageInfoArray(
    PTDNF_PKG_INFO pPkgInfo,
    uint32_t dwLength
    );

void
TDNFFreePackageInfo(
    PTDNF_PKG_INFO pPkgInfo
    );

void
TDNFFreeRepos(
    PTDNF_REPO_DATA pRepos
    );

void
TDNFFreeUpdateInfoSummary(
    PTDNF_UPDATEINFO_SUMMARY pSummary
    );

void
TDNFFreeUpdateInfo(
    PTDNF_UPDATEINFO pUpdateInfo
    );

//utils
uint32_t
TDNFIsSystemError(
    uint32_t dwError
    );

uint32_t
TDNFGetSystemError(
    uint32_t dwError
    );

uint32_t
TDNFGetErrorString(
    uint32_t dwErrorCode,
    char** ppszErrorString
    );

uint32_t
TDNFUtilsFormatSize(
    uint32_t dwSize,
    char** ppszFormattedSize
    );
int
TDNFIsGlob(
    const char* pszString
    );

uint32_t
TDNFUtilsMakeDir(
    const char* pszPath
    );

uint32_t
TDNFUtilsMakeDirs(
    const char* pszPath
    );
//strings.c
uint32_t
TDNFAllocateString(
    const char* pszSrc,
    char** ppszDst
    );

void
TDNFFreeStringArray(
    char** ppszArray
    );
#ifdef __cplusplus
}
#endif

#endif//TDNF_CLIENT_H_
