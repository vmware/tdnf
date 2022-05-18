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
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#ifndef __CLIENT_PROTOTYPES_H__
#define __CLIENT_PROTOTYPES_H__

#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/evp.h>

extern uid_t gEuid;

//clean.c
uint32_t
TDNFCopyEnabledRepos(
    PTDNF_REPO_DATA pRepoData,
    char*** pppszReposUsed
    );

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
AddKeyPktToKeyring(
    rpmKeyring pKeyring,
    uint8_t* pPkt,
    size_t nPktLen
    );

uint32_t
VerifyRpmSig(
    rpmKeyring pKeyring,
    const char* pszPkgFile
    );

uint32_t
TDNFGPGCheck(
    rpmKeyring pKeyring,
    const char* pszUrlKeyFile,
    const char* pszPackage
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
    const char* pszRepoName,
    const char* pszFilePath,
    Header *pRpmHeader
    );

uint32_t
TDNFFetchRemoteGPGKey(
    PTDNF pTdnf,
    const char* pszRepoName,
    const char* pszUrlGPGKey,
    char** ppszKeyLocation
    );

//init.c
uint32_t
TDNFCloneCmdArgs(
    PTDNF_CMD_ARGS pCmdArgsIn,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

uint32_t
TDNFCloneSetOpts(
    PTDNF_CMD_OPT pCmdOptIn,
    PTDNF_CMD_OPT* ppCmdOpt
    );

uint32_t
TDNFRefreshRepo(
    PTDNF pTdnf,
    int nCleanMetadata,
    PTDNF_REPO_DATA pRepo
    );

uint32_t
TDNFRefreshSack(
    PTDNF pTdnf,
    PSolvSack pSack,
    int nCleanMetadata
    );

//makecache.c
uint32_t
TDNFRefreshCache(
    PTDNF pTdnf
    );

//repoutils.c
uint32_t
TDNFRepoMakeCacheDirs(
    const char* pszRepo
    );

uint32_t
TDNFRepoGetBaseUrl(
    PTDNF pTdnf,
    const char* pszRepo,
    char** ppszBaseUrl
    );

uint32_t
TDNFRepoSetBaseUrl(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pszRepo,
    const char *pszBaseUrlFile
    );

uint32_t
TDNFRepoGetUserPass(
    PTDNF pTdnf,
    const char* pszRepo,
    char** ppszUserPass
    );

uint32_t
TDNFRepoGetRpmCacheDir(
    PTDNF pTdnf,
    const char* pszRepo,
    char** ppszRpmCacheDir
    );

uint32_t
TDNFRepoRemoveCache(
    PTDNF pTdnf,
    const char* pszRepoId
    );

uint32_t
TDNFRemoveRpmCache(
    PTDNF pTdnf,
    const char* pszRepoId
    );

uint32_t
TDNFRemoveLastRefreshMarker(
    PTDNF pTdnf,
    const char* pszRepoId
    );

uint32_t
TDNFRemoveTmpRepodata(
    const char* pszTmpRepodataDir
    );

uint32_t
TDNFRemoveSolvCache(
    PTDNF pTdnf,
    const char* pszRepoId
    );

uint32_t
TDNFRemoveKeysCache(
    PTDNF pTdnf,
    const char* pszRepoId
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
TDNFCheckHexDigest(
    const char *hex_digest,
    int digest_length
    );

uint32_t
TDNFChecksumFromHexDigest(
    const char *hex_digest,
    unsigned char *ppdigest
    );

uint32_t
TDNFCheckRepoMDFileHashFromMetalink(
    char *pszFile,
    TDNF_ML_CTX *ml_ctx
    );

uint32_t
TDNFParseAndGetURLFromMetalink(
    PTDNF pTdnf,
    const char *pszRepo,
    const char *pszFile,
    TDNF_ML_CTX *ml_ctx
    );

uint32_t
TDNFDownloadFile(
    PTDNF pTdnf,
    const char *pszRepo,
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
    const char* pszRepo,
    const char* pszRpmCacheDir
    );

uint32_t
TDNFDownloadPackageToCache(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    char** ppszFilePath
    );

uint32_t
TDNFDownloadPackageToTree(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    char* pszNormalRpmCacheDir,
    char** ppszFilePath
    );

uint32_t
TDNFDownloadPackageToDirectory(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
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
TDNFPopulatePkgInfoArray(
    PSolvSack pSack,
    PSolvPackageList pPkgList,
    TDNF_PKG_DETAIL nDetail,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFAppendPackages(
    PTDNF_PKG_INFO* ppDest,
    PTDNF_PKG_INFO pSource
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
TDNFAddPackagesForInstall(
    PSolvSack pSack,
    Queue* pQueueGoal,
    const char* pszPkgName
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
TDNFCheckProtectedPkgs(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
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
TDNFHistoryGoal(
    PTDNF pTdnf,
    Queue *pqInstall,
    Queue *pqErase,
    PTDNF_SOLVED_PKG_INFO* ppInfo
    );

uint32_t
TDNFAddUserInstall(
    PTDNF pTdnf,
    Queue* pQueueGoal,
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
    PTDNF_SOLVED_PKG_INFO ppInfo
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
TDNFGoalGetAllResultsIgnoreNoData(
    Transaction* pTrans,
    Solver* pSolv,
    PTDNF_SOLVED_PKG_INFO* ppInfo,
    PTDNF pTdnf
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
TDNFGetUnneededPackages(
    Solver* pSolv,
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
TDNFSolvAddMinVersions(
    PTDNF pTdnf,
    Queue* pQueueJobs,
    Pool *pPool
    );

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
    const char* pszRepo,
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
    char* pszRepoFile,
    PTDNF_REPO_DATA* ppRepos
    );

uint32_t
TDNFCreateCmdLineRepo(
    PTDNF_REPO_DATA* ppRepo
    );

uint32_t
TDNFCreateRepoFromPath(
    PTDNF_REPO_DATA* ppRepo,
    const char *pzsId,
    const char *pszPath
    );

uint32_t
TDNFCreateRepo(
    PTDNF_REPO_DATA* ppRepo,
    const char *pszId
    );

uint32_t
TDNFLoadRepoData(
    PTDNF pTdnf,
    TDNF_REPOLISTFILTER nFilter,
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
TDNFPrepareAndAddPkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    TDNF_ALTERTYPE nAlterType,
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
TDNFAddFilteredPkgs(
    PTDNF pTdnf,
    int nScope,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    Queue* pQueueGoal
    );

uint32_t
TDNFAddNotResolved(
    char** ppszPkgsNotResolved,
    const char* pszPkgName
    );

//rpmtrans.c
uint32_t
TDNFRpmExecTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pInfo,
    TDNF_ALTERTYPE nAlterType
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
TDNFTransAddObsoletedPkgs(
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
    PTDNF_PKG_INFO pInfo
    );

uint32_t
TDNFTransAddReInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfo
    );

uint32_t
TDNFTransAddInstallPkg(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    int nUpgrade
    );

uint32_t
TDNFTransAddUpgradePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfo
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

uint32_t
TDNFIsDir(
    const char* pszPath,
    int* pnPathIsDir
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
TDNFRawGetPackageVersion(
   const char* pszRootDir,
   const char* pszPkg,
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

uint32_t
TDNFReadAutoInstalled(
    PTDNF pTdnf,
    char ***pppszAutoInstalled
    );

//validate.c
uint32_t
TDNFValidateCmdArgs(
    PTDNF pTdnf
    );

uint32_t
TDNFIsInitialized(
    );

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

// metalink.c

uint32_t
TDNFMetalinkParseFile(
    TDNF_ML_CTX *ml_ctx,
    int fd,
    const char *filename
    );

void
TDNFMetalinkFree(
    TDNF_ML_CTX *ml_ctx
    );

uint32_t
TDNFXmlParseData(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node,
    const char *filename
    );

uint32_t
TDNFParseFileTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node,
    const char *filename
    );

uint32_t
TDNFParseHashTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node
    );


uint32_t
TDNFParseUrlTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node
    );

// list.c
void
TDNFSortListOnPreference(
    TDNF_ML_LIST** headUrl
);

uint32_t
TDNFAppendList(
    TDNF_ML_LIST** head_ref,
    void *new_data
);

void
TDNFDeleteList(
    TDNF_ML_LIST** head_ref,
    TDNF_ML_FREE_FUNC free_func
);


uint32_t
TDNFGetHistoryCtx(
    PTDNF pTdnf,
    struct history_ctx **ppCtx,
    int nMustExist
);

#endif /* __CLIENT_PROTOTYPES_H__ */
