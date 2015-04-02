/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Header : prototypes.h
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
*/
//gpgcheck.c
uint32_t
TDNFGPGCheck(
    rpmKeyring pKeyring,
    const char* pszUrlKeyFile,
    const char* pszPackage
    );

//init.c
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
//remoterepo.c
uint32_t
TDNFDownloadPackage(
    PTDNF pTdnf,
    HyPackage hPkg,
    const char* pszCacheDir
    );

//packageutils.c
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

//goal.c
uint32_t
TDNFGoal(
    PTDNF pTdnf,
    HyPackageList hPkgList,
    HySelector hSelector,
    TDNF_ALTERTYPE nResolveFor,
    PTDNF_SOLVED_PKG_INFO pInfo
    );

uint32_t
TDNFGoalReportProblems(
    HyGoal hGoal
    );
//config.c
uint32_t
TDNFReadConfig(
    char* pszConfFile,
    char* pszConfGroup,
    PTDNF_CONF* ppConf
    );

void
TDNFFreeConfig(
    PTDNF_CONF pConf
    );

//repo.c
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
TDNFGetRepoByName(
    PTDNF pTdnf,
    const char* pszName,
    PTDNF_REPO_DATA* ppRepo
    );

//repolist.c
uint32_t
TDNFLoadRepoData(
    PTDNF_CONF pConf,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA* ppReposAll
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

//utils.c
uint32_t
TDNFIsDir(
    const char* pszPath,
    int* pnPathIsDir
    );

uint32_t
TDNFUtilsFormatSize(
    uint32_t dwSize,
    char** ppszFormattedSize
    );
