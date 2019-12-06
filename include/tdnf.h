/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef _TDNF_H_
#define _TDNF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tdnftypes.h"
#include "tdnferror.h"

//api.c

//global init.
uint32_t
TDNFInit(
    );

//Open a handle using initial args
//args can define a command, have config overrides
uint32_t
TDNFOpenHandle(
    PTDNF_CMD_ARGS pArgs,
    PTDNF* pTdnf
    );

//check for updates
uint32_t
TDNFCheckUpdates(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

//clean local cache. all is the only type
//currently supported.
uint32_t
TDNFClean(
    PTDNF pTdnf,
    TDNF_CLEANTYPE nCleanType,
    PTDNF_CLEAN_INFO* ppCleanInfo
    );

//show list of packages filtered by scope, name
//globbing supported.
uint32_t
TDNFList(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

//show info on packages filtered by scope, name.
//globbing supported
uint32_t
TDNFInfo(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgListInfo,
    uint32_t* pdwCount
    );

//show information on currently configured repositories
uint32_t
TDNFRepoList(
    PTDNF pTdnf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA* ppRepoData
    );

//refresh cache
uint32_t
TDNFMakeCache(
    PTDNF pTdnf
    );

//check all packages in all enables repositories
uint32_t
TDNFCheckPackages(
    PTDNF pTdnf
    );

//check all packages in a local directory
//using the local directory contents
//for dep resolution.
uint32_t
TDNFCheckLocalPackages(
    PTDNF pTdnf,
    const char* pszLocalPath
    );

//show packages that provide a particular file
uint32_t
TDNFProvides(
    PTDNF pTdnf,
    const char* pszSpec,
    PTDNF_PKG_INFO* ppPkgInfo
    );

//Show update info for specified scope
uint32_t
TDNFUpdateInfo(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO* ppUpdateInfo
    );

//Show update info summary
uint32_t
TDNFUpdateInfoSummary(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO_SUMMARY* ppSummary
    );

//sanity check. displays current installed count.
//should be same as rpm -qa | wc -l
uint32_t
TDNFCountCommand(
    PTDNF pTdnf,
    uint32_t* pdwCount
    );

//version
const char*
TDNFGetVersion(
    );

//package name
const char*
TDNFGetPackageName(
    );

//Search installed and available packages for keywords
//in description, name
uint32_t
TDNFSearchCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

//invoke solv goal dependency resolution
//return solved pkg info which has descriptive
//info about steps to reach current goal.
//usually the SolvedPkgInfo is used to display
//info about changes to the user and upon approval,
//submitted to TDNFAlterCommand
uint32_t
TDNFResolve(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO* ppSolvedPkgInfo
    );

//This function will alter the current
//install state.
//install/update/erase/downgrade are
//represented as altertype.
uint32_t
TDNFAlterCommand(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    );

//Show a descriptive error message
//divided into different areas like
//solv, repo, rpm and generic tdnf errors.
uint32_t
TDNFGetErrorString(
    uint32_t dwErrorCode,
    char** ppszErrorString
    );

void
TDNFCloseHandle(
    PTDNF pTdnf
    );

void
TDNFFreeCleanInfo(
    PTDNF_CLEAN_INFO pCleanInfo
    );

void
TDNFFreeCmdArgs(
    PTDNF_CMD_ARGS pCmdArgs
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
TDNFFreeRepos(
    PTDNF_REPO_DATA pRepos
    );

void
TDNFFreeSolvedPackageInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    );

void
TDNFFreeUpdateInfo(
    PTDNF_UPDATEINFO pUpdateInfo
    );

void
TDNFFreeUpdateInfoSummary(
    PTDNF_UPDATEINFO_SUMMARY pSummary
    );

void
TDNFFreeCmdOpt(
    PTDNF_CMD_OPT pCmdOpt
    );

//free global resources.
void
TDNFUninit(
    );


#ifdef __cplusplus
}
#endif

#endif//TDNF_H_
