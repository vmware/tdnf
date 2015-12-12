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
    PTDNF pTdnf,
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
    HyQuery hQuery,
    TDNF_SCOPE nScope
    );


uint32_t
TDNFApplyPackageFilter(
    HyQuery hQuery,
    char** ppszPackageNameSpecs
    );

//gpgcheck.c
uint32_t
ReadAllBytes(
    const char* pszFileName,
    char** pszFileContents
    );

uint32_t
ReadGPGKey(
   const char* pszFile,
   char** ppszKeyData
   );

uint32_t
AddKeyToKeyRing(
    const char* pszFile,
    rpmKeyring pKeyring
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

//init.c
uint32_t
TDNFCloneCmdArgs(
    PTDNF_CMD_ARGS pCmdArgsIn,
    PTDNF_CMD_ARGS* ppCmdArgs
    );

uint32_t
TDNFInitSack(
    PTDNF pTdnf,
    HySack* phSack,
    int nFlags
    );

uint32_t
TDNFLoadYumRepo(
    HySack hSack,
    HyRepo hRepo,
    int nFlags
    );

uint32_t
TDNFRefreshSack(
    PTDNF pTdnf,
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
TDNFRepoGetKeyValue(
    GKeyFile* pKeyFile,
    const char* pszGroup,
    const char* pszKeyName,
    const char* pszDefault,
    char** ppszValue
    );

uint32_t
TDNFRepoGetKeyValueBoolean(
    GKeyFile* pKeyFile,
    const char* pszGroup,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    );

uint32_t
TDNFRepoApplyProxySettings(
    PTDNF_CONF pConf,
    LrHandle* pRepoHandle
    );
//remoterepo.c
uint32_t
TDNFDownloadPackage(
    PTDNF pTdnf,
    HyPackage hPkg,
    const char* pszCacheDir
    );

//packageutils.c
uint32_t
TDNFFindAvailablePkgByPkg(
    HySack hSack,
    HyPackage hPkgToFind,
    HyPackage* phPkg
    );

uint32_t
TDNFFindInstalledPkgByName(
    HySack hSack,
    const char* pszName,
    HyPackage* phPkg
    );

uint32_t
TDNFGetInstalled(
    HySack hSack,
    HyPackageList* phPkgList
    );

uint32_t
TDNFMatchForReinstall(
    HySack hSack,
    const char* pszName,
    HyPackageList* phPkgList
    );

uint32_t
TDNFPopulatePkgInfos(
    HyPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfos
    );

uint32_t
TDNFPopulatePkgInfoArray(
    HyPackageList hPkgList,
    TDNF_PKG_DETAIL nDetail,
    PTDNF_PKG_INFO* ppPkgInfo,
    uint32_t* pdwCount
    );

uint32_t
TDNFPackageGetLatest(
    HyPackageList hPkgList,
    HyPackage* phPkgLatest
    );

uint32_t
TDNFAppendPackages(
    PTDNF_PKG_INFO* ppDest,
    PTDNF_PKG_INFO pSource
    );

uint32_t
TDNFPackageGetDowngrade(
    HyPackageList hPkgList,
    HyPackage hPkgCurrent,
    HyPackage* phPkgDowngrade
    );

uint32_t
TDNFGetGlobPackages(
    PTDNF pTdnf,
    char* pszPkgGlob,
    HyPackageList* phPkgList
    );

uint32_t
TDNFFilterPackages(
    PTDNF pTdnf,
    int nScope,
    HyPackageList* phPkgList
    );

uint32_t
TDNFAddPackagesForInstall(
    HyPackageList hPkgListSource,
    HyPackageList hPkgListGoal
    );

uint32_t
TDNFAddPackagesForUpgrade(
    HyPackageList hPkgListSource,
    HyPackageList hPkgListGoal
    );

uint32_t
TDNFAddPackagesForDowngrade(
    HyPackageList hPkgListSource,
    HyPackageList hPkgListGoal
    );

//goal.c
uint32_t
TDNFGoalGetResultsIgnoreNoData(
    HyPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfoResults
    );

uint32_t
TDNFGoalGetResults(
    HyPackageList hPkgList,
    PTDNF_PKG_INFO* ppPkgInfoResults
    );

uint32_t
TDNFGoalSetUserInstalled(
    HyGoal hGoal,
    HyPackageList hPkgList
    );

uint32_t
TDNFGoal(
    PTDNF pTdnf,
    HyPackageList hPkgList,
    PTDNF_SOLVED_PKG_INFO pInfo
    );

uint32_t
TDNFGoalReportProblems(
    HyGoal hGoal
    );

uint32_t
TDNFAddGoal(
    PTDNF pTdnf,
    int nAlterType,
    HyGoal hGoal,
    HyPackage hPkg
    );

uint32_t
TDNFGoalGetAllResultsIgnoreNoData(
    int nResolveFor,
    HyGoal hGoal,
    PTDNF_SOLVED_PKG_INFO* ppInfo
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
TDNFReadKeyValue(
    GKeyFile* pKeyFile,
    char* pszGroupName,
    char* pszKeyName,
    char* pszDefault,
    char** ppszValue
    );

uint32_t
TDNFReadConfig(
    PTDNF pTdnf,
    char* pszConfFile,
    char* pszConfGroup
    );

uint32_t
TDNFConfigExpandVars(
    PTDNF pTdnf
    );

uint32_t
TDNFConfigReadProxySettings(
    GKeyFile* pKeyFile,
    char* pszGroup,
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

//repo.c
uint32_t
TDNFPrintRepoMetadata(
    LrYumRepoMd* pRepoMD
    );

uint32_t
TDNFInitRepoFromMetaData(
    HyRepo hRepo,
    LrYumRepo* pRepo);

uint32_t
TDNFInitRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepoData,
    HyRepo* phRepo
    );

uint32_t
TDNFGetGPGCheck(
    PTDNF pTdnf,
    const char* pszRepo,
    int* pnGPGCheck,
    char** ppszUrlGPGKey
    );

uint32_t
TDNFGetRepoById(
    PTDNF pTdnf,
    const char* pszName,
    PTDNF_REPO_DATA* ppRepo
    );

//repolist.c
uint32_t
TDNFLoadReposFromFile(
    PTDNF pTdnf,
    char* pszRepoFile,
    PTDNF_REPO_DATA* ppRepos
    );

uint32_t
TDNFLoadRepoData(
    PTDNF pTdnf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA* ppReposAll
    );

//resolve.c
HySubject
hy_subject_create(
    const char * pattern
    );

HyPossibilities
hy_subject_nevra_possibilities_real(
    HySubject subject,
    HyForm *forms, HySack sack, int flags
    );

int
hy_possibilities_next_nevra(
    HyPossibilities iter,
    HyNevra *out_nevra
    );

void
hy_possibilities_free(
    HyPossibilities iter
    );

void
hy_subject_free(
    HySubject subject
    );

uint32_t
TDNFGetMatchingInstalledAndAvailable(
    PTDNF pTdnf,
    int nAlterType,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList* phPkgListGoal
    );

uint32_t
TDNFGetMatching(
    PTDNF pTdnf,
    int nSystem,
    const char* pszName,
    HyPackageList* phPkgList
    );

uint32_t
TDNFGetGoalPackageList(
    TDNF_ALTERTYPE nAlterType,
    HyPackageList hPkgsInstalled,
    HyPackageList hPkgsAvailable,
    HyPackageList* phPkgList
    );

uint32_t
TDNFGetSelector(
    PTDNF pTdnf,
    const char* pszPkg,
    HySelector* phSelector
    );

uint32_t
TDNFPrepareAllPackages(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList* phPkgListGoal
    );

uint32_t
TDNFPrepareAndAddPkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList hPkgListGoal
    );

uint32_t
TDNFPrepareSinglePkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList* phPkgListGoal
    );

uint32_t
TDNFAddFilteredPkgs(
    PTDNF pTdnf,
    int nScope,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo,
    HyPackageList hPkgListGoal
    );

uint32_t
TDNFAddNotResolved(
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    const char* pszPkgName
    );

//rpmtrans.c
uint32_t
TDNFRpmExecTransaction(
    PTDNF pTdnf,
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
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
    PTDNF_SOLVED_PKG_INFO pSolvedInfo
    );

uint32_t
TDNFTransAddDowngradePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFTransAddErasePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFTransAddObsoletedPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFTransAddErasePkg(
    PTDNFRPMTS pTS,
    HyPackage hPkg
    );

uint32_t
TDNFTransAddInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFTransAddReInstallPkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFTransAddInstallPkg(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    HyPackage hPkg,
    int nUpgrade
    );

uint32_t
TDNFTransAddUpgradePkgs(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFRunTransaction(
    PTDNFRPMTS pTS,
    PTDNF pTdnf
    );

uint32_t
TDNFRemoveCachedRpms(
    GArray* pCachedRpmsArray
    );

void
TDNFFreeCachedRpmsArray(
    GArray* pArray
    );

//memory.c
void
TDNFFreePackageInfoContents(
    PTDNF_PKG_INFO pPkgInfo
    );

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
TDNFFreeCmdOpt(
    PTDNF_CMD_OPT pCmdOpt
    );

//search.c
uint32_t
TDNFQueryTermsHelper(
    HyPackageList hAccumPkgList,
    HyQuery hQuery,
    int nKeyId,
    const char* pszMatch
    );

uint32_t
TDNFCopyWithWildCards(
    const char* pszSrc,
    const char** ppszDst
    );

uint32_t
QueryTermsInNameSummary(
    HyPackageList,
    HyQuery,
    const char*
    );

uint32_t
QueryTermsInDescUrl(
    HyPackageList,
    HyQuery,
    const char*
    );

uint32_t
TDNFQueryTerms(
    HyPackageList hAccumPkgList,
    PTDNF_CMD_ARGS pCmdArgs,
    HyQuery hQuery,
    int nStartArgIndex,
    TDNFQueryTermsFunction pfQueryTerms
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
//updateinfo.c
uint32_t
TDNFGetUpdateInfoPackages(
    HyAdvisory hAdv,
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
