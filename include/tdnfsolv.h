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
    Pool* pPool;
    Id*   pCommandLinePkgs;
    int   numOfCommandPkgs;
    char* pszCacheDir;
    char* pszRootDir;
} SolvSack, *PSolvSack;

typedef struct _SolvQuery
{
    PSolvSack  pSack;
    Queue  job;
    Solver *pSolv;
    Transaction *pTrans;
    Queue repoFilter;
    Queue kindFilter;
    Queue archFilter;
    char** packageNames;
    Queue result;
    int   nNewPackages;
} SolvQuery, *PSolvQuery;

typedef struct _SolvPackage
{
    Id pkg;
} SolvPackage, *PSolvPackage;

typedef struct _SolvPackageList
{
    Queue packages;
} SolvPackageList, *PSolvPackageList;

typedef struct _SolvGoal
{
    Queue goal;
} SolvGoal, *PSolvGoal;

typedef struct _SolvRepo
{
    Repo* repo;
} SolvRepo, *PSolvRepo;

typedef struct _SolvSubject
{

} SolvSubject, *PSolvSubject;

typedef struct _SolvPossibilities
{

} SolvPossibilities, *PSolvPossibilities;

typedef struct _SolvForm
{

} SolvForm, *PSolvForm;

typedef struct _SolvNevra
{

} SolvNevra, PSolvNevra;

typedef struct _SolvSelector
{

} SolvSelector, *PSolvSelector;

typedef struct _SolvAdvisory
{

} SolvAdvisory, *PSolvAdvisory;

// CreateSack()
// FreeSack()
// tdnf_init_pool     done.
// SolvReadYumRepo    done.
// add_cmdline_package need more work.
// addfileprovides(pool);
// pool_createwhatprovides(pool);


// ApplyQuery
// RunSolver
// RunTransaction
const char*
SolvGetPkgNameFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgArchFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgVersionFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgReleaseFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgRepoNameFromId(
    PSolvSack pSack,
    int pkgId);

uint32_t
SolvGetPkgInstallSizeFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgSummaryFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgLicenseFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgDescriptionFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgUrlFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgLocationFromId(
    PSolvSack pSack,
    int pkgId);

const char*
SolvGetPkgNevrFromId(
    PSolvSack pSack,
    int pkgId);

uint32_t
SolvSplitEvr(
    PSolvSack pSack,
    const char *pEVRstring,
    char **ppEpoch,
    char **ppVersion,
    char **ppLease);

PSolvSack
SolvCreateSack();

void
SolvFreeSack(PSolvSack);

PSolvQuery
SolvCreateQuery(
    PSolvSack pSack
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
    char** searchStrings,
    int startIndex,
    int count
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
    PSolvSack pSack
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

PSolvPackageList
SolvCreatePackageList();

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
    PSolvPackageList pPkgList
    );

uint32_t
SolvGetPackageId(
    PSolvPackageList pPkgList,
    int pkgIndex,
    Id* pkgId
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
    Id pkg1,
    Id pkg2,
    int* result);

uint32_t
SolvGetLatest(
    PSolvSack pSack,
    Queue* pPkgList,
    Id pkg,
    Id* result);

uint32_t
SolvRemovePkgWithHigherorEqualEvr(
    PSolvSack pSack,
    Queue* pPkgList,
    Id pkg,
    Queue* pNewQueue);

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
    int* count
    );

uint32_t
SolvGetTransResultsWithType(
    Transaction *trans,
    Id type,
    PSolvPackageList pPkgList
    );

uint32_t
SolvAddUpgradeAllJob(
    Queue* jobs
    );

uint32_t
SolvAddDistUpgradeJob(
    Queue* jobs
    );

uint32_t
SolvAddFlagsToJobs(
    Queue* jobs,
    int flags);

uint32_t
SolvAddPkgInstallJob(
    Queue* jobs,
    Id id);

uint32_t
SolvAddPkgDowngradeJob(
    Queue* jobs,
    Id id);

uint32_t
SolvAddPkgEraseJob(
    Queue* jobs,
    Id id);

uint32_t
SolvAddPkgUserInstalledJob(
    Queue* jobs,
    Id id);

#ifdef __cplusplus
}
#endif