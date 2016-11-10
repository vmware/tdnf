#pragma once
#ifdef __cplusplus
extern "C" {
#endif
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
} SolvQuery, *PSolvQuery;

typedef struct _SolvPackage
{
    Id dwPkg;
} SolvPackage, *PSolvPackage;

typedef struct _SolvPackageList
{
    Queue       queuePackages;
} SolvPackageList, *PSolvPackageList;

typedef struct _SolvGoal
{
    Queue       queueGoal;
} SolvGoal, *PSolvGoal;

typedef struct _SolvRepo
{
    Repo*       pRepo;
} SolvRepo, *PSolvRepo;


typedef struct _SolvAdvisory
{

} SolvAdvisory, *PSolvAdvisory;

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
    uint32_t  dwPkgId,
    char** ppszNevr);

uint32_t
SolvSplitEvr(
    PSolvSack pSack,
    const char *pEVRstring,
    char **ppEpoch,
    char **ppVersion,
    char **ppLease);

uint32_t
SolvCmpEvr(
    PSolvSack pSack,
    Id  dwPkg1,
    Id  dwPkg2,
    int* pdwResult
    );

uint32_t
SolvCreatePackageList(
    PSolvPackageList* ppSolvPackageList
    );

void
SolvEmptyPackageList(
    PSolvPackageList  pPkgList
    );

void
SolvFreePackageList(
    PSolvPackageList pPkgList
    );

uint32_t
SolvQueueToPackageList(
    Queue* pQueue,
    PSolvPackageList pPkgList
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
    Id     dwPpkg,
    Id*    pdwResult
    );

uint32_t
SolvFindAllInstalled(
    PSolvSack pSack,
    PSolvPackageList pPkgList
    );

uint32_t
SolvFindAvailablePkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList pPkgList
    );

uint32_t
SolvFindInstalledPkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList pPkgList
    );

uint32_t
SolvCountPkgByName(
    PSolvSack pSack,
    const char* pszName,
    uint32_t * pdwCount
    );

uint32_t
SolvGetTransResultsWithType(
    Transaction *pTrans,
    Id          dwType,
    PSolvPackageList pPkgList
    );


uint32_t
SolvFindHighestAvailable(
    PSolvSack   pSack,
    const char* pszPkgName,
    Id*         pdwId
    );

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
    char**      ppszPackageNames
    );

uint32_t
SolvApplySinglePackageFilter(
    PSolvQuery  pQuery,
    const char* pszPackageName
    );

uint32_t
SolvApplyListQuery(
    PSolvQuery pQuery
    );

uint32_t
SolvApplyDistroSyncQuery(
    PSolvQuery pQuery
    );

uint32_t
SolvApplySearch(
    PSolvQuery  pQuery,
    char**      ppszSearchStrings,
    int         dwStartIndex,
    int         dwCount
    );

uint32_t
SolvRunSolv(
    PSolvQuery pQuery,
    uint32_t dwMainMode,
    uint32_t dwMode,
    Queue* queueJobs,
    Solver** ppSolv
    );

uint32_t
SolvApplyAlterQuery(
    PSolvQuery pQuery,
    uint32_t dwMainMode,
    uint32_t dwMode
    );

uint32_t
SolvGenerateCommonJob(
    PSolvQuery  pQuery
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
SolvGetListResult(
    PSolvQuery pQuery,
    PSolvPackageList pPkgList
    );

uint32_t
SolvGetSearchResult(
    PSolvQuery pQuery,
    PSolvPackageList pPkgList
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
    int    dwFlags
    );

uint32_t
SolvAddPkgInstallJob(
    Queue* pQueueJobs,
    Id     dwId
    );

uint32_t
SolvAddPkgDowngradeJob(
    Queue* pQueueJobs,
    Id     dwId
    );

uint32_t
SolvAddPkgEraseJob(
    Queue* pQueueJobs,
    Id     dwId
    );

uint32_t
SolvAddPkgUserInstalledJob(
    Queue* pQueueJobs,
    Id     dwId
    );

// tdnfrepo.c
uint32_t
SolvReadYumRepo(
    PSolvSack pSack,
    const char *pszRepoName,
    const char *pszRepomd,
    const char *pszPrimary,
    const char *pszFilelists,
    const char *pszUpdateinfo
    );

uint32_t
SolvCountPackages(
    PSolvSack pSack,
    uint32_t* pdwCount;
    );

uint32_t
SolvReadInstalledRpms(
    Pool* pPool,
    Repo** ppRepo,
    const char*  pszCacheFileName
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

#ifdef __cplusplus
}
#endif
