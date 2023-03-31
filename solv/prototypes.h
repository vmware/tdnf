/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TDNF_ID_DEPENDS "tdnf:depends"
#define TDNF_ID_REQUIRES_PRE "tdnf:requires-pre"

typedef struct _SolvSack
{
    Pool*       pPool;
    uint32_t    dwNumOfCommandPkgs;
    char*       pszCacheDir;
    char*       pszRootDir;
} SolvSack, *PSolvSack;

typedef struct _SolvQuery
{
    PSolvSack   pSack;
    Queue       queueJob;
    Solver      *pSolv;
    Transaction *pTrans;
    Queue       queueRepoFilter;
    char**      ppszPackageNames;
    Queue       queueResult;
    uint32_t    dwNewPackages;
    TDNF_SCOPE  nScope;
} SolvQuery, *PSolvQuery;

typedef struct _SolvPackageList
{
    Queue       queuePackages;
} SolvPackageList, *PSolvPackageList;

typedef struct _SOLV_REPO_INFO_INTERNAL_
{
    Repo*         pRepo;
    unsigned char cookie[SOLV_COOKIE_LEN];
    int           nCookieSet;
    char          *pszRepoCacheDir;
}SOLV_REPO_INFO_INTERNAL, *PSOLV_REPO_INFO_INTERNAL;

extern Id allDepKeyIds[];

// tdnfpackage.c
uint32_t
SolvGetPkgInfoFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t dwWhichInfo,
    char** ppszInfo);

uint32_t
SolvGetPkgNameFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszName);

uint32_t
SolvGetPkgArchFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszArch);

uint32_t
SolvGetPkgVersionFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char ** ppszVersion);

uint32_t
SolvGetPkgReleaseFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszRelease);

uint32_t
SolvGetPkgRepoNameFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppRepoName);

uint32_t
SolvGetPkgInstallSizeFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t * pdwSize);

uint32_t
SolvGetPkgDownloadSizeFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t * pdwSize);

uint32_t
SolvGetPkgSummaryFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszSummary);

uint32_t
SolvGetPkgLicenseFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszLicense);

uint32_t
SolvGetPkgDescriptionFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszDescription);

uint32_t
SolvGetPkgUrlFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszUrl);

uint32_t
SolvGetPkgLocationFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszLocation);

uint32_t
SolvGetPkgNevrFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char** ppszNevr);

uint32_t
SolvSplitEvr(
    const PSolvSack pSack,
    const char *pEVRstring,
    char **ppEpoch,
    char **ppVersion,
    char **ppLease);

uint32_t
SolvCmpEvr(
    PSolvSack pSack,
    Id dwPkg1,
    Id dwPkg2,
    int* pdwResult
    );

uint32_t
SolvCreatePackageList(
    PSolvPackageList* ppSolvPackageList
    );

void
SolvEmptyPackageList(
    PSolvPackageList pPkgList
    );

void
SolvFreePackageList(
    PSolvPackageList pPkgList
    );

uint32_t
SolvQueueToPackageList(
    Queue* pQueue,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvGetPackageListSize(
    PSolvPackageList pPkgList,
    uint32_t* pdwSize
    );

uint32_t
SolvGetPackageId(
    PSolvPackageList pPkgList,
    uint32_t dwPkgIndex,
    Id* dwPkgId
    );


uint32_t
SolvGetLatest(
    PSolvSack pSack,
    Queue* pPkgList,
    Id dwPpkg,
    Id* pdwResult
    );

uint32_t
SolvFindAllInstalled(
    PSolvSack pSack,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvFindAvailablePkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvFindAvailableSrcPkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvFindInstalledPkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvFindInstalledPkgByMultipleNames(
    PSolvSack pSack,
    char** ppszName,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvCountPkgByName(
    PSolvSack pSack,
    const char* pszName,
    int nSource,
    uint32_t * pdwCount
    );

uint32_t
SolvGetTransResultsWithType(
    Transaction *pTrans,
    Id dwType,
    PSolvPackageList* ppPkgList
    );


uint32_t
SolvFindHighestAvailable(
    PSolvSack pSack,
    const char* pszPkgName,
    int nSource,
    Id* pdwId
    );

uint32_t
SolvFindLowestInstalled(
    PSolvSack pSack,
    const char* pszPkgName,
    Id* pdwId
    );

uint32_t
SolvFindHighestInstalled(
    PSolvSack pSack,
    const char* pszPkgName,
    Id* pdwId
    );

uint32_t
SolvFindHighestOrLowestInstalled(
    PSolvSack pSack,
    const char* pszPkgName,
    Id* pdwId,
    uint32_t dwFindHighest
    );

uint32_t
SolvGetNevraFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t *pdwEpoch,
    char **ppszName,
    char **ppszVersion,
    char **ppszRelease,
    char **ppszArch,
    char **ppszEVR
    );

uint32_t
SolvGetDependenciesFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    REPOQUERY_DEP_KEY depKey,
    char ***pppszDependencies);

uint32_t
SolvGetFileListFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char ***pppszFiles);

uint32_t
SolvGetSourceFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    char **ppszName,
    char **ppszArch,
    char **ppszEVR
    );

uint32_t
SolvGetChangeLogFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    PTDNF_PKG_CHANGELOG_ENTRY *ppEntries
    );

uint32_t
SolvIdIsOrphaned(
    PSolvSack pSack,
    Id p,
    int *pnIsOrphan
);

uint32_t
SolvGetAutoInstalledOrphans(
    PSolvSack pSack,
    struct history_ctx *pHistoryCtx,
    Queue *pQueueAutoInstalled);

// tdnfpool.c
uint32_t
SolvCreateSack(
    PSolvSack* ppSack
    );

void
SolvFreeSack(
    PSolvSack);

uint32_t
SolvInitSack(
    PSolvSack *ppSack,
    const char* pszCacheDir,
    const char* pszRootDir
);

// tdnfquery.c
uint32_t
SolvCreateQuery(
    PSolvSack pSack,
    PSolvQuery* ppQuery
    );

void
SolvFreeQuery(
    PSolvQuery pQuery
    );

uint32_t
SolvApplyPackageFilter(
    PSolvQuery  pQuery,
    char** ppszPackageNames
    );

uint32_t
SolvApplySinglePackageFilter(
    PSolvQuery pQuery,
    const char* pszPackageName
    );

uint32_t
SolvApplyListQuery(
    PSolvQuery pQuery
    );

uint32_t
SolvApplyProvidesQuery(
    PSolvQuery pQuery
    );

uint32_t
SolvApplySearch(
    PSolvQuery pQuery,
    char** ppszSearchStrings,
    int dwStartIndex,
    int dwCount
    );

uint32_t
SolvGenerateCommonJob(
    PSolvQuery pQuery,
    uint32_t dwSelectFlags
    );

uint32_t
SolvAddSystemRepoFilter(
    PSolvQuery pQuery
    );

uint32_t
SolvAddAvailableRepoFilter(
    PSolvQuery pQuery
    );

uint32_t
SolvGetQueryResult(
    PSolvQuery pQuery,
    PSolvPackageList* ppPkgList
    );

uint32_t
SolvAddUpgradeAllJob(
    Queue* pQueueJobs
    );

uint32_t
SolvAddDistUpgradeJob(
    Queue* pQueueJobs
    );

uint32_t
SolvAddFlagsToJobs(
    Queue* pQueueJobs,
    int dwFlags
    );

uint32_t
SolvAddPkgInstallJob(
    Queue* pQueueJobs,
    Id dwId
    );

uint32_t
SolvAddPkgDowngradeJob(
    Queue* pQueueJobs,
    Id dwId
    );

uint32_t
SolvAddPkgEraseJob(
    Queue* pQueueJobs,
    Id dwId
    );

uint32_t
SolvAddUserInstalledToJobs(
    Queue* pQueueJobs,
    Pool *pPool,
    struct history_ctx *pHistoryCtx
    );

uint32_t
SolvGetUpdateAdvisories(
    PSolvSack pSack,
    Id dwPkgIdpkg,
    PSolvPackageList* ppPkgList);

uint32_t
SolvFindAllUpDownCandidates(
    PSolvSack pSack,
    PSolvPackageList  pInstalledPackages,
    int up,
    Queue *pQueueResult
    );

uint32_t
SolvGetUpdateAdvisories(
    PSolvSack pSack,
    Id dwPkgIdpkg,
    PSolvPackageList* ppPkgList);

uint32_t
SolvApplyDepsFilter(
    PSolvQuery pQuery,
    char **ppszDeps,
    REPOQUERY_WHAT_KEY whatKey);

uint32_t
SolvApplyExtrasFilter(
    PSolvQuery pQuery);

uint32_t
SolvApplyDuplicatesFilter(
    PSolvQuery pQuery);

uint32_t
SolvApplyFileProvidesFilter(
    PSolvQuery pQuery,
    char *pszFile);

uint32_t
SolvApplyUserInstalledFilter(
    PSolvQuery pQuery,
    struct history_ctx *pHistoryCtx);

uint32_t
SolvApplyArchFilter(
    PSolvQuery pQuery,
    char **ppszArchs);

// tdnfrepo.c
uint32_t
SolvReadYumRepo(
    Repo *pRepo,
    const char *pszRepoName,
    const char *pszRepomd,
    const char *pszPrimary,
    const char *pszFilelists,
    const char *pszUpdateinfo,
    const char *pszOther
    );

uint32_t
SolvCountPackages(
    PSolvSack pSack,
    uint32_t* pdwCount
    );

uint32_t
SolvReadRpmsFromDirectory(
    Repo *pRepo,
    const char *pszDir
);

uint32_t
SolvReadInstalledRpms(
    Repo* pRepo,
    const char *pszCacheFileName
);

uint32_t
SolvLoadRepomd(
    Repo* pRepo,
    const char* pszRepomd
    );

uint32_t
SolvLoadRepomdPrimary(
    Repo* pRepo,
    const char* pszPrimary
    );

uint32_t
SolvLoadRepomdFilelists(
    Repo* pRepo,
    const char* pszFilelists
    );

uint32_t
SolvLoadRepomdUpdateinfo(
    Repo* pRepo,
    const char* pszUpdateinfo
    );

uint32_t
SolvReportProblems(
    PSolvSack pSack,
    Solver* pSolv,
    TDNF_SKIPPROBLEM_TYPE dwSkipProblem
    );

uint32_t
SolvAddExcludes(
    Pool* pPool,
    char** ppszExcludes
    );

uint32_t
SolvDataIterator(
     Pool* pPool,
     char** ppszExcludes,
     Map* pMap
     );

int
SolvIsGlob(
    const char* pszString
    );

uint32_t
SolvCalculateCookieForFile(
    char* pszRepoMD,
    unsigned char* pszCookie
    );

uint32_t
SolvCreateRepoCacheName(
    const char *pszName,
    const char *pszUrl,
    char **ppszCacheName
    );

uint32_t
SolvGetMetaDataCachePath(
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    char** ppszCachePath
    );

uint32_t
SolvAddSolvMetaData(
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    char *pszTempSolvFile
    );

uint32_t
SolvCreateMetaDataCache(
    PSolvSack pSack,
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo
    );

uint32_t
SolvUseMetaDataCache(
    PSolvSack pSack,
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    int       *nUseMetaDataCache
    );

uint32_t
SolvFindSolvablesByNevraStr(
    Pool *pool,
    const char *nevra,
    Queue* qresult,
    int installed
    );

uint32_t
SolvRequiresFromQueue(
    Pool *pool,
    Queue *pq_pkgs,  /* solvable ids */
    Queue *pq_deps   /* string ids */
);

#ifdef __cplusplus
}
#endif
