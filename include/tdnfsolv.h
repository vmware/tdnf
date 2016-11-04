#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <sys/utsname.h>
// libsolv
#include <solv/evr.h>
#include <solv/pool.h>
#include <solv/poolarch.h>
#include <solv/repo.h>
#include <solv/repo_deltainfoxml.h>
#include <solv/repo_repomdxml.h>
#include <solv/repo_updateinfoxml.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_rpmdb.h>
#include <solv/repo_solv.h>
#include <solv/repo_write.h>
#include <solv/solv_xfopen.h>
#include <solv/solver.h>
#include <solv/selection.h>
#include <solv/solverdebug.h>

typedef struct _SolvSack
{
    Pool*       pPool;
    Id*         pdwCommandLinePkgs;
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
    Queue       queueKindFilter;
    Queue       queueArchFilter;
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


uint32_t
SolvGetPkgInfoFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t dwWhichInfo,
    const char** ppszInfo);

uint32_t
SolvGetPkgNameFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszName);

uint32_t
SolvGetPkgArchFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszArch);

uint32_t
SolvGetPkgVersionFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char ** ppszVersion);

uint32_t
SolvGetPkgReleaseFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszRelease);

uint32_t
SolvGetPkgRepoNameFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppRepoName);

uint32_t
SolvGetPkgInstallSizeFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    uint32_t * pdwSize);

uint32_t
SolvGetPkgSummaryFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszSummary);

uint32_t
SolvGetPkgLicenseFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszLicense);

uint32_t
SolvGetPkgDescriptionFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszDescription);


uint32_t
SolvGetPkgUrlFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszUrl);

uint32_t 
SolvGetPkgLocationFromId(
    PSolvSack pSack,
    uint32_t dwPkgId,
    const char** ppszLocation);

uint32_t
SolvGetPkgNevrFromId(
    PSolvSack pSack,
    uint32_t  dwPkgId,
    const char** ppszNevr);

uint32_t
SolvSplitEvr(
    PSolvSack pSack,
    const char *pEVRstring,
    char **ppEpoch,
    char **ppVersion,
    char **ppLease);

uint32_t 
SolvCreateSack(PSolvSack* ppSack);

void
SolvFreeSack(PSolvSack);

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
    PSolvQuery pQuery,
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
SolvApplyDistroSyncQuery(
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
SolvGetNumPkgs(
    PSolvQuery pQuery
    );

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
SolvInitSack(
    PSolvSack *ppSack,
    const char* pszCacheDir,
    const char* pszRootDir
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
    int dwPkgIndex,
    Id* dwPkgId
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
SolvCmpEvr(
    PSolvSack pSack,
    Id  dwPkg1,
    Id  dwPkg2,
    int* pdwResult
    );

uint32_t
SolvGetLatest(
    PSolvSack pSack,
    Queue* pPkgList,
    Id     dwPpkg,
    Id*    pdwResult
    );

uint32_t
SolvRemovePkgWithHigherorEqualEvr(
    PSolvSack pSack,
    Queue* pPkgList,
    Id     dwPkg,
    Queue* pNewQueue
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
    Transaction *trans,
    Id          dwType,
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

#ifdef __cplusplus
}
#endif