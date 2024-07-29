/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __CLIENT_PROTOTYPES_H__
#define __CLIENT_PROTOTYPES_H__

#include <unistd.h>

extern uid_t gEuid;

//client.c
uint32_t
TDNFApplyScopeFilter(
    PSolvQuery qQuery,
    TDNF_SCOPE nScope
    );

//gpgcheck.c
uint32_t
ReadGPGKeyFile(
    const char* pszFile,
    char** ppszKeyData,
    int* pnSize
   );

uint32_t
AddKeyFileToKeyring(
    const char* pszFile,
    rpmKeyring pKeyring
    );

uint32_t
TDNFImportGPGKeyFile(
    rpmts pTS,
    const char* pszFile
    );

uint32_t
TDNFGPGCheckPackage(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char* pszFilePath,
    Header *pRpmHeader
    );

uint32_t
TDNFFetchRemoteGPGKey(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char* pszUrlGPGKey,
    char** ppszKeyLocation
    );

//init.c
uint32_t
TDNFCloneSetOpts(
    PTDNF_CMD_OPT pCmdOptIn,
    PTDNF_CMD_OPT* ppCmdOpt
    );

uint32_t
TDNFRefreshSack(
    PTDNF pTdnf,
    PSolvSack pSack,
    int nCleanMetadata
    );

//repoutils.c
uint32_t
TDNFRepoGetUserPass(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    char** ppszUserPass
    );

uint32_t
TDNFRepoGetRpmCacheDir(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    char** ppszRpmCacheDir
    );

uint32_t
TDNFRepoRemoveCacheDir(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRepoRemoveCache(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRemoveRpmCache(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRemoveLastRefreshMarker(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRemoveMirrorList(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRemoveTmpRepodata(
    const char* pszTmpRepodataDir
    );

uint32_t
TDNFRemoveSolvCache(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRemoveKeysCache(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRepoApplyDownloadSettings(
    PTDNF_REPO_DATA pRepo,
    CURL *pCurl
    );

uint32_t
TDNFRepoApplyProxySettings(
    PTDNF_CONF pConf,
    CURL *pCurl
    );

uint32_t
TDNFRepoApplySSLSettings(
    PTDNF_REPO_DATA pRepo,
    CURL *pCurl
    );

uint32_t
TDNFFindRepoById(
    PTDNF pTdnf,
    const char* pszRepo,
    PTDNF_REPO_DATA* ppRepo
    );

uint32_t
TDNFCurlErrorIsFatal(
    CURLcode curlError
);

void
TDNFFreeHistoryInfoItems(
    PTDNF_HISTORY_INFO_ITEM pHistoryItems,
    int nCount
);

//remoterepo.c
uint32_t
TDNFDownloadFileFromRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszLocation,
    const char *pszFile,
    const char *pszProgressData
);

uint32_t
TDNFDownloadFile(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszFileUrl,
    const char *pszFile,
    const char *pszProgressData
    );

uint32_t
TDNFCreatePackageUrl(
    PTDNF pTdnf,
    const char* pszRepoName,
    const char* pszPackageLocation,
    char **ppszPackageUrl
    );

uint32_t
TDNFDownloadPackage(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    const char* pszRpmCacheDir
    );

uint32_t
TDNFDownloadPackageToCache(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    char** ppszFilePath
    );

uint32_t
TDNFDownloadPackageToTree(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    char* pszNormalRpmCacheDir,
    char** ppszFilePath
    );

uint32_t
TDNFDownloadPackageToDirectory(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    const char* pszDirectory,
    char** ppszFilePath
    );

//packageutils.c
uint32_t
TDNFMatchForReinstall(
    PSolvSack pSack,
    const char* pszName,
    Queue* pQueueGoal
    );

uint32_t
TDNFPopulatePkgInfos(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO* ppPkgInfo
    );

uint32_t
TDNFPopulatePkgInfoForRepoSync(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO* ppPkgInfo
    );

uint32_t
TDNFPkgInfoFilterNewest(
    PSolvSack pSack,
    PTDNF_PKG_INFO pPkgInfos
);

uint32_t
TDNFPopulatePkgInfoQueryFormat(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFPopulatePkgInfoArray(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    TDNF_PKG_DETAIL nDetail,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFPackageGetDowngrade(
    Id dwInstalled,
    PSolvSack pSack,
    PSolvPackageList pAvailabePkgList,
    Id* pdwDowngradePkgId
    );

uint32_t
TDNFGetGlobPackages(
    PSolvSack pSack,
    char* pszPkgGlob,
    Queue* pQueueGlob
    );

uint32_t
TDNFFilterPackages(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    char** ppszPkgsNotResolved,
    Queue* pQueueGoal
    );

uint32_t
TDNFGetAutoInstalledOrphans(
    PTDNF pTdnf,
    Queue* pQueueGoal);

uint32_t
TDNFAddPackagesForInstall(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName,
    int nSource,
    int nInstallOnly
    );

uint32_t
TDNFAddPackagesForErase(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    );

uint32_t
TDNFAddPackagesForUpgrade(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    );

uint32_t
TDNFVerifyUpgradePackage(
    PSolvSack pSack,
    Id dwPkg,
    uint32_t* pdwUpgradePackage
    );

uint32_t
TDNFVerifyInstallPackage(
    PSolvSack pSack,
    Id dwPkg,
    uint32_t* pdwInstallPackage
    );

uint32_t
TDNFAddPackagesForDowngrade(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
    );

uint32_t
TDNFGetAvailableCacheBytes(
    PTDNF_CONF pConf,
    uint64_t* pqwAvailCacheBytes
    );

uint32_t
TDNFCheckDownloadCacheBytes(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    uint64_t qwAvailCacheBytes
    );


uint32_t
TDNFPopulatePkgInfoArrayDependencies(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    REPOQUERY_DEP_KEY depKey,
    PTDNF_PKG_INFO pPkgInfos
    );

uint32_t
TDNFPopulatePkgInfoArrayFileList(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    PTDNF_PKG_INFO pPkgInfos
    );

//goal.c
uint32_t
TDNFGoal(
    PTDNF pTdnf,
    Queue* pkgList,
    PTDNF_SOLVED_PKG_INFO* ppInfo,
    TDNF_ALTERTYPE nAlterType
    );

uint32_t
TDNFGoalNoDeps(
    PTDNF pTdnf,
    Queue* pQueuePkgList,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    );

uint32_t
TDNFHistoryGoal(
    PTDNF pTdnf,
    Queue *pqInstall,
    Queue *pqErase,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    );

uint32_t
TDNFAddUserInstall(
    PTDNF pTdnf,
    const Queue* pQueueGoal,
    PTDNF_SOLVED_PKG_INFO ppInfo
    );

uint32_t
TDNFMarkAutoInstalledSinglePkg(
    PTDNF pTdnf,
    const char *pszPkgName
);

uint32_t
TDNFMarkAutoInstalled(
    PTDNF pTdnf,
    struct history_ctx *pHistoryCtx,
    PTDNF_SOLVED_PKG_INFO ppInfo,
    int nAutoOnly
    );

uint32_t
TDNFAddGoal(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    Queue* pQueueJobs,
    Id dwId,
    uint32_t dwCount,
    char** ppszExcludes
    );

uint32_t
TDNFGetPackagesWithSpecifiedType(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo,
    Id dwType
    );

uint32_t
TDNFGetInstallPackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetReinstallPackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetUpgradePackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo);

uint32_t
TDNFGetErasePackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetObsoletedPackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetDownGradePackages(
    Transaction* pTrans,
    PTDNF pTdnf,
    PTDNF_PKG_INFO* pPkgInfo,
    PTDNF_PKG_INFO* pRemovePkgInfo
    );

uint32_t
TDNFPkgsToExclude(
    PTDNF pTdnf,
    uint32_t *pdwPkgsToExclude,
    char***  pppszExclude
    );

uint32_t
TDNFSolvAddPkgLocks(
    PTDNF pTdnf,
    Queue* pQueueJobs,
    Pool *pPool
    );

uint32_t
TDNFSolvAddInstallOnlyPkgs(
    PTDNF pTdnf,
    Queue* pQueueJobs,
    Pool *pPool
    );

uint32_t
TDNFSolvAddMinVersions(
    PTDNF pTdnf,
    Pool *pPool
    );

uint32_t
TDNFSolvAddProtectPkgs(
    PTDNF pTdnf,
    Queue* pQueueJobs,
    Pool *pPool
    );

uint32_t
TDNFSolvCheckProtectPkgsInTrans(
    PTDNF pTdnf,
    Transaction *pTrans,
    Pool *pPool
    );

uint32_t
TDNFSolvCheckInstallOnlyLimitInTrans(
    PTDNF pTdnf,
    Transaction *pTrans,
    Pool *pPool,
    Queue *pQueueJobs
    );

//config.c
int
TDNFConfGetRpmVerbosity(
    PTDNF pTdnf
    );

uint32_t
TDNFReadConfig(
    PTDNF pTdnf,
    const char* pszConfFile,
    const char* pszConfGroup
    );

uint32_t
TDNFConfigExpandVars(
    PTDNF pTdnf
    );

uint32_t
TDNFConfigReadProxySettings(
    PCONF_SECTION pSection,
    PTDNF_CONF pConf);

void
TDNFFreeConfig(
    PTDNF_CONF pConf
    );

uint32_t
TDNFConfigReplaceVars(
    PTDNF pTdnf,
    char** pszString
    );

uint32_t
TDNFReadConfFilesFromDir(
    char *pszDir,
    char ***pppszMinVersions
    );

//repo.c
uint32_t
TDNFInitRepoFromMetadata(
    Repo *pRepo,
    const char* pszRepoName,
    PTDNF_REPO_METADATA pRepoMD
    );

uint32_t
TDNFInitRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepoData,
    PSolvSack pSack
    );

uint32_t
TDNFInitCmdLineRepo(
    PTDNF pTdnf,
    PSolvSack pSack
    );

uint32_t
TDNFGetGPGCheck(
    PTDNF pTdnf,
    const char* pszRepo,
    int* pnGPGCheck
    );

uint32_t
TDNFGetGPGSignatureCheck(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    int* pnGPGSigCheck,
    char*** ppszUrlGPGKeys
    );

uint32_t
TDNFGetSkipSignatureOption(
    PTDNF pTdnf,
    uint32_t *pdwSkipSignature
    );

uint32_t
TDNFGetSkipDigestOption(
    PTDNF pTdnf,
    uint32_t *pdwSkipDigest
    );

uint32_t
TDNFGetCachePath(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszSubDir,
    const char *pszFileName,
    char **ppszPath
);

uint32_t
TDNFGetRepoById(
    PTDNF pTdnf,
    const char* pszName,
    PTDNF_REPO_DATA* ppRepo
    );

uint32_t
TDNFGetRepoMD(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepoData,
    const char *pszRepoDataDir,
    PTDNF_REPO_METADATA *ppRepoMD
    );

uint32_t
TDNFParseRepoMD(
    PTDNF_REPO_METADATA pRepoMD
    );

uint32_t
TDNFFindRepoMDPart(
    Repo *pSolvRepo,
    const char *pszType,
    char **ppszPart
    );

void
TDNFFreeRepoMetadata(
    PTDNF_REPO_METADATA pRepoMD
    );

uint32_t
TDNFEnsureRepoMDParts(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    PTDNF_REPO_METADATA pRepoMDRel,
    PTDNF_REPO_METADATA *ppRepoMD
    );

uint32_t
TDNFParseRepoMD(
    PTDNF_REPO_METADATA pRepoMD
    );

uint32_t
TDNFFindRepoMDPart(
    Repo *pSolvRepo,
    const char *pszType,
    char **ppszPart
    );

void
TDNFFreeRepoMetadata(
    PTDNF_REPO_METADATA pRepoMD
    );

uint32_t
TDNFReplaceFile(
    const char *pszSrcFile,
    const char *pszDstFile
    );

uint32_t
TDNFDownloadMetadata(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszRepoDir,
    int nPrintOnly
    );

uint32_t
TDNFDownloadRepoMDParts(
    PTDNF pTdnf,
    Repo *pSolvRepo,
    PTDNF_REPO_DATA pRepo,
    const char *pszDir,
    int nPrintOnly
    );

//repolist.c
uint32_t
TDNFLoadReposFromFile(
    PTDNF pTdnf,
    const char* pszRepoFile,
    PTDNF_REPO_DATA* ppRepos
    );

uint32_t
TDNFLoadRepoData(
    PTDNF pTdnf,
    PTDNF_REPO_DATA* ppReposAll
    );

uint32_t
TDNFRepoListFinalize(
    PTDNF pTdnf
    );

uint32_t
TDNFAlterRepoState(
    PTDNF_REPO_DATA pRepos,
    int nEnable,
    const char* pszId
    );

uint32_t
TDNFCloneRepo(
    PTDNF_REPO_DATA pRepoIn,
    PTDNF_REPO_DATA* ppRepo
    );

void
TDNFFreeReposInternal(
    PTDNF_REPO_DATA pRepos
    );

//resolve.c
uint32_t
TDNFPrepareAllPackages(
    PTDNF pTdnf,
    TDNF_ALTERTYPE* pAlterType,
    char** ppszPkgsNotResolved,
    Queue* pQueueGoal
    );

uint32_t
TDNFPrepareSinglePkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    TDNF_ALTERTYPE nAlterType,
    char** ppszPkgsNotResolved,
    Queue* pQueueGoal
    );

uint32_t
TDNFAddNotResolved(
    char** ppszPkgsNotResolved,
    const char* pszPkgName
    );

uint32_t
TDNFResolveBuildDependencies(
    PTDNF pTdnf,
    char **ppszPackageNameSpecs,
    char **ppszPkgsNotResolved,
    Queue* queueGoal
    );

//rpmtrans.c
uint32_t
TDNFRpmExecTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pInfo
    );

uint32_t
TDNFRpmExecHistoryTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    PTDNF_HISTORY_ARGS pHistoryArgs
    );

void*
TDNFRpmCB(
    const void* pArg,
    const rpmCallbackType what,
    const rpm_loff_t amount,
    const rpm_loff_t total,
    fnpyKey key,
    void* data
    );

uint32_t
TDNFPopulateTransaction(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pInfo
    );

uint32_t
TDNFTransAddErasePkgs(
    PTDNFRPMTS pTS,
    PTDNF_PKG_INFO pInfo
    );

uint32_t
TDNFTransAddErasePkg(
    PTDNFRPMTS pTS,
    const char* pszPkgName
    );

uint32_t
TDNFTransAddInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfo,
    int nUpgrade
    );

uint32_t
TDNFTransAddInstallPkg(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfo,
    PTDNF_REPO_DATA pRepo,
    int nUpgrade
    );

uint32_t
TDNFRunTransaction(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFRemoveCachedRpms(
    PTDNF_CACHED_RPM_LIST pCachedRpmsList
    );

void
TDNFFreeCachedRpmsArray(
    PTDNF_CACHED_RPM_LIST pArray
    );

//updateinfo.c
uint32_t
TDNFGetUpdateInfoPackages(
    PSolvSack pSack,
    Id dwPkgId,
    PTDNF_UPDATEINFO_PKG* ppUpdateInfoPkg
    );

void
TDNFFreeUpdateInfoReferences(
    PTDNF_UPDATEINFO_REF pRef
    );

void
TDNFFreeUpdateInfoPackages(
    PTDNF_UPDATEINFO_PKG pPkg
    );

uint32_t
TDNFPopulateUpdateInfoOfOneAdvisory(
    PSolvSack pSack,
    Id dwAdvId,
    uint32_t dwSecurity,
    const char*  pszSeverity,
    uint32_t dwRebootRequired,
    PTDNF_UPDATEINFO* ppInfo
    );

uint32_t
TDNFGetSecuritySeverityOption(
    PTDNF pTdnf,
    uint32_t *pdwSecurity,
    char **ppszSeverity
    );

uint32_t
TDNFNumUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    uint32_t *pdwCount
    );

uint32_t
TDNFGetUpdatePkgs(
    PTDNF pTdnf,
    char*** pppszPkgs,
    uint32_t *pdwCount
    );

uint32_t
TDNFGetRebootRequiredOption(
    PTDNF pTdnf,
    uint32_t *pdwRebootRequired
    );

//utils.c
uint32_t
TDNFIsCurlError(
    uint32_t dwError
    );

uint32_t
TDNFIsSystemError(
    uint32_t dwError
    );

uint32_t
TDNFIsCurlError(
    uint32_t dwError
    );

uint32_t
TDNFGetSystemError(
    uint32_t dwError
    );

uint32_t
TDNFGetCurlError(
    uint32_t dwError
    );

uint32_t
TDNFIsFileOrSymlink(
    const char* pszPath,
    int* pnPathIsFile
    );

uint32_t
TDNFGetFileSize(
    const char* pszPath,
    int *pnSize
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
TDNFTouchFile(
    const char* pszFile
    );

uint32_t
TDNFGetReleaseVersion(
   const char* pszRootDir,
   const char* pszDistroVerPkg,
   char** ppszVersion
   );

uint32_t
TDNFGetKernelArch(
    char** ppszArch
    );

uint32_t
TDNFParseMetadataExpire(
    const char* pszMetadataExpire,
    long* plMetadataExpire
    );

uint32_t
TDNFShouldSyncMetadata(
    const char* pszRepoDataFolder,
    long lMetadataExpire,
    int* pnShouldSync
    );


uint32_t
TDNFAppendPath(
    const char *pszBase,
    const char *pszPart,
    char **ppszPath
    );

//validate.c
uint32_t
TDNFGetSkipProblemOption(
    PTDNF pTdnf,
    TDNF_SKIPPROBLEM_TYPE *pdwSkipProblem
    );

/* plugins.c */
uint32_t
TDNFLoadPlugins(
    PTDNF pTdnf
    );

uint32_t
TDNFPluginRaiseEvent(
    PTDNF pTdnf,
    PTDNF_EVENT_CONTEXT pContext
    );

void
TDNFFreePlugins(
    PTDNF_PLUGIN pPlugins
    );

void
TDNFShowPluginError(
    PTDNF pTdnf,
    PTDNF_PLUGIN pPlugin,
    uint32_t nErrorCode
    );
/* eventdata.c */

uint32_t
TDNFAddEventDataString(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const char *pcszStr
    );

uint32_t
TDNFAddEventDataPtr(
    PTDNF_EVENT_CONTEXT pContext,
    const char *pcszName,
    const void *pPtr
    );

void
TDNFFreeEventData(
    PTDNF_EVENT_DATA pData
    );

/* api.c */
uint32_t
TDNFListInternal(
    PTDNF pTdnf,
    TDNF_SCOPE nScope,
    char** ppszPackageNameSpecs,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount,
    TDNF_PKG_DETAIL nDetail
    );


uint32_t
TDNFGetHistoryCtx(
    PTDNF pTdnf,
    struct history_ctx **ppCtx,
    int nMustExist
);


struct cnfnode *parse_varsdirs(char *dirs[]);
char *replace_vars(struct cnfnode *cn_vars, const char *source);

#endif /* __CLIENT_PROTOTYPES_H__ */
