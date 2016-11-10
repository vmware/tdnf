/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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


//checklocal.c
uint32_t
TDNFCheckLocalPackagesInternal(
    PTDNF       pTdnf,
    const char* pszLocalPath
    );

//clean.c
uint32_t
TDNFCopyEnabledRepos(
    PTDNF_REPO_DATA pRepoData,
    char*** pppszReposUsed
    );

//client.c
uint32_t
TDNFApplyScopeFilter(
    PSolvQuery  qQuery,
    TDNF_SCOPE  nScope
    );

//gpgcheck.c
uint32_t
ReadGPGKey(
   const char*  pszFile,
   char**       ppszKeyData
   );

uint32_t
AddKeyToKeyRing(
    const char* pszFile,
    rpmKeyring  pKeyring
    );

uint32_t
VerifyRpmSig(
    rpmKeyring  pKeyring,
    const char* pszPkgFile
    );

uint32_t
TDNFGPGCheck(
    rpmKeyring  pKeyring,
    const char* pszUrlKeyFile,
    const char* pszPackage
    );

//init.c
uint32_t
TDNFCloneCmdArgs(
    PTDNF_CMD_ARGS  pCmdArgsIn,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

uint32_t
TDNFRefreshSack(
    PTDNF           pTdnf,
    PSolvSack       pSack,
    int             nCleanMetadata
    );

//makecache.c
uint32_t
TDNFRefreshCache(
    PTDNF   pTdnf
    );

//repoutils.c
uint32_t
TDNFRepoMakeCacheDirs(
    const char* pszRepo
    );

uint32_t
TDNFRepoGetBaseUrl(
    PTDNF       pTdnf,
    const char* pszRepo,
    char**      ppszBaseUrl
    );

uint32_t
TDNFRepoGetUserPass(
    PTDNF       pTdnf,
    const char* pszRepo,
    char**      ppszUserPass
    );

uint32_t
TDNFRepoGetRpmCacheDir(
    PTDNF       pTdnf,
    const char* pszRepo,
    char**      ppszRpmCacheDir
    );

uint32_t
TDNFRepoRemoveCache(
    PTDNF       pTdnf,
    const char* pszRepoId
    );

uint32_t
TDNFRepoGetKeyValue(
    GKeyFile*   pKeyFile,
    const char* pszGroup,
    const char* pszKeyName,
    const char* pszDefault,
    char**      ppszValue
    );

uint32_t
TDNFRepoGetKeyValueBoolean(
    GKeyFile*   pKeyFile,
    const char* pszGroup,
    const char* pszKeyName,
    int         nDefault,
    int*        pnValue
    );

uint32_t
TDNFRepoApplyProxySettings(
    PTDNF_CONF  pConf,
    LrHandle*   pRepoHandle
    );
//remoterepo.c
uint32_t
TDNFDownloadPackage(
    PTDNF       pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepo,
    const char* pszRpmCacheDir
    );

//packageutils.c

uint32_t
TDNFMatchForReinstall(
    PSolvSack   pSack,
    const char* pszName,
    Queue*      pQueueGoal
    );

uint32_t
TDNFPopulatePkgInfos(
    PSolvSack           pSack,
    PSolvPackageList    pPkgList,
    PTDNF_PKG_INFO*     ppPkgInfo
    );

uint32_t
TDNFPopulatePkgInfoArray(
    PSolvSack           pSack,
    PSolvPackageList    pPkgList,
    TDNF_PKG_DETAIL     nDetail,
    PTDNF_PKG_INFO*     ppPkgInfo,
    uint32_t*           pdwCount
    );

uint32_t
TDNFAppendPackages(
    PTDNF_PKG_INFO*     ppDest,
    PTDNF_PKG_INFO      pSource
    );

uint32_t
TDNFPackageGetDowngrade(
    Id          dwCurrent,
    PSolvSack   pSack,
    Id*         pkgId,
    const char* pszPkgName
    );

uint32_t
TDNFGetGlobPackages(
    PSolvSack   pSack,
    char*       pszPkgGlob,
    Queue*      pQueueGlob
    );

uint32_t
TDNFFilterPackages(
    PSolvSack               pSack,
    PTDNF_SOLVED_PKG_INFO   pSolvedPkgInfo,
    Queue*                  pQueueGoal
    );

uint32_t
TDNFAddPackagesForInstall(
    PSolvSack   pSack,
    Queue*      pQueueGoal,
    const char* pszPkgName
    );

uint32_t
TDNFAddPackagesForErase(
    PSolvSack   pSack,
    Queue*      pQueueGoal,
    const char* pszPkgName
    );

uint32_t
TDNFAddPackagesForUpgrade(
    PSolvSack   pSack,
    Queue*      pQueueGoal,
    const char* pszPkgName
    );

uint32_t
TDNFAddPackagesForDowngrade(
    PSolvSack   pSack,
    Queue*      pQueueGoal,
    const char* pszPkgName
    );
//goal.c

uint32_t
TDNFGoal(
    PTDNF                   pTdnf,
    Queue*                  pkgList,
    PTDNF_SOLVED_PKG_INFO   pInfo
    );

uint32_t
TDNFAddGoal(
    PTDNF   pTdnf,
    int     nAlterType,
    Queue*  pQueueJobs,
    Id      dwId
    );

uint32_t
TDNFGoalGetAllResultsIgnoreNoData(
    int                     nResolveFor,
    Transaction*            pTrans,
    Solver*                 pSolv,
    PTDNF_SOLVED_PKG_INFO*  ppInfo,
    PTDNF                   pTdnf
    );

uint32_t
TDNFGetPackagesWithSpecifiedType(
    Transaction*    pTrans,
    PTDNF           pTdnf, 
    PTDNF_PKG_INFO* pPkgInfo,
    Id              dwType
    );

uint32_t
TDNFGetInstallPackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetReinstallPackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetUpgradePackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo);

uint32_t
TDNFGetErasePackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetObsoletedPackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetUnneededPackages(
    Solver*         pSolv,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo
    );

uint32_t
TDNFGetDownGradePackages(
    Transaction*    pTrans,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO* pPkgInfo,
    PTDNF_PKG_INFO* pRemovePkgInfo
    );

//config.c
int
TDNFConfGetRpmVerbosity(
    PTDNF   pTdnf
    );

uint32_t
TDNFConfSetFlag(
    TDNF_CONF_FLAG  nFlag,
    int             nValue //0 or 1
    );

uint32_t
TDNFConfGetFlag(
    TDNF_CONF_FLAG  nFlag,
    int             nValue //0 or 1
    );

uint32_t
TDNFConfSetValue(
    TDNF_CONF_TYPE  nType,
    const char*     pszValue
    );

uint32_t
TDNFConfGetValue(
    TDNF_CONF_TYPE  nType,
    char**          ppszValue
    );
uint32_t
TDNFReadKeyValue(
    GKeyFile*       pKeyFile,
    char*           pszGroupName,
    char*           pszKeyName,
    char*           pszDefault,
    char**          packageutilspszValue
    );

uint32_t
TDNFReadConfig(
    PTDNF       pTdnf,
    char*       pszConfFile,
    char*       pszConfGroup
    );

uint32_t
TDNFConfigExpandVars(
    PTDNF   pTdnf
    );

uint32_t
TDNFConfigReadProxySettings(
    GKeyFile*   pKeyFile,
    char*       pszGroup,
    PTDNF_CONF  pConf);

void
TDNFFreeConfig(
    PTDNF_CONF  pConf
    );

uint32_t
TDNFConfigReplaceVars(
    PTDNF       pTdnf,
    char**      pszString
    );

//repo.c
uint32_t
TDNFPrintRepoMetadata(
    LrYumRepoMd* pRepoMD
    );

uint32_t
TDNFInitRepoFromMetaData(
    PSolvSack   pSack,
    const char* pszRepoName,
    LrYumRepo*  pRepo);

uint32_t
TDNFInitRepo(
    PTDNF           pTdnf,
    PTDNF_REPO_DATA pRepoData,
    PSolvSack       pSack
    );

uint32_t
TDNFGetGPGCheck(
    PTDNF       pTdnf,
    const char* pszRepo,
    int*        pnGPGCheck,
    char**      ppszUrlGPGKey
    );

uint32_t
TDNFGetRepoById(
    PTDNF               pTdnf,
    const char*         pszName,
    PTDNF_REPO_DATA*    ppRepo
    );

//repolist.c
uint32_t
TDNFLoadReposFromFile(
    PTDNF               pTdnf,
    char*               pszRepoFile,
    PTDNF_REPO_DATA*    ppRepos
    );

uint32_t
TDNFLoadRepoData(
    PTDNF               pTdnf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA*    ppReposAll
    );

//resolve.c
uint32_t
TDNFPrepareAllPackages(
    PTDNF                   pTdnf,
    PTDNF_SOLVED_PKG_INFO   pSolvedPkgInfo,
    Queue*                  pQueueGoal
    );

uint32_t
TDNFPrepareAndAddPkg(
    PSolvSack               pSack,
    const char*             pszPkgName,
    PTDNF_SOLVED_PKG_INFO   pSolvedPkgInfo,
    Queue*                  pQueueGoal
    );

uint32_t
TDNFPrepareSinglePkg(
    PSolvSack               pSack,
    const char*             pszPkgName,
    PTDNF_SOLVED_PKG_INFO   pSolvedPkgInfo,
    Queue*                  pQueueGoal
    );

uint32_t
TDNFAddFilteredPkgs(
    PTDNF                   pTdnf,
    int                     nScope,
    PTDNF_SOLVED_PKG_INFO   pSolvedPkgInfo,
    Queue*                  pQueueGoal
    );

uint32_t
TDNFAddNotResolved(
    PTDNF_SOLVED_PKG_INFO   pSolvedInfo,
    const char*             pszPkgName
    );

//rpmtrans.c
uint32_t
TDNFRpmExecTransaction(
    PTDNF                   pTdnf,
    PTDNF_SOLVED_PKG_INFO   pInfo
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
    PTDNFRPMTS              pTS,
    PTDNF                   pTdnf,
    PTDNF_SOLVED_PKG_INFO   pInfo
    );

uint32_t
TDNFTransAddErasePkgs(
    PTDNFRPMTS      pTS,
    PTDNF_PKG_INFO  pInfo
    );

uint32_t
TDNFTransAddObsoletedPkgs(
    PTDNFRPMTS      pTS,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO  pInfo
    );

uint32_t
TDNFTransAddErasePkg(
    PTDNFRPMTS      pTS,
    const char*     pszPkgName
    );

uint32_t
TDNFTransAddInstallPkgs(
    PTDNFRPMTS      pTS,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO  pInfo
    );

uint32_t
TDNFTransAddReInstallPkgs(
    PTDNFRPMTS      pTS,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO  pInfo
    );

uint32_t
TDNFTransAddInstallPkg(
    PTDNFRPMTS  pTS,
    PTDNF       pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    int         nUpgrade
    );

uint32_t
TDNFTransAddUpgradePkgs(
    PTDNFRPMTS      pTS,
    PTDNF           pTdnf,
    PTDNF_PKG_INFO  pInfo
    );

uint32_t
TDNFRunTransaction(
    PTDNFRPMTS      pTS,
    PTDNF           pTdnf
    );

uint32_t
TDNFRemoveCachedRpms(
    GArray*     pCachedRpmsArray
    );

void
TDNFFreeCachedRpmsArray(
    GArray*     pArray
    );

//memory.c
void
TDNFFreePackageInfoContents(
    PTDNF_PKG_INFO pPkgInfo
    );

void
TDNFFreeCmdOpt(
    PTDNF_CMD_OPT pCmdOpt
    );

//search.c

//updateinfo.c
uint32_t
TDNFGetUpdateInfoPackages(
    PSolvAdvisory hAdv,
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

//utils.c
uint32_t
TDNFIsSystemError(
    uint32_t dwError
    );

uint32_t
TDNFGetSystemError(
    uint32_t dwError
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
TDNFUtilsMakeDirs(
    const char* pszPath
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

//validate.c
uint32_t
TDNFValidateCmdArgs(
    PTDNF pTdnf
    );
