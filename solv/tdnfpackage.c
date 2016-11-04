#include "includes.h"

PSolvPackageList
SolvCreatePackageList()
{
    uint32_t dwError = 0;
    PSolvPackageList pPkgList = NULL;

    pPkgList = solv_calloc(1, sizeof(SolvPackageList));
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_init(&pPkgList->packages);

cleanup: 
    return pPkgList;

error:
    goto cleanup;
}

void
SolvEmptyPackageList(
    PSolvPackageList  pPkgList
    )
{
    if(pPkgList)
    {
        queue_free(&pPkgList->packages);
    }
}

void
SolvFreePackageList(
    PSolvPackageList pPkgList
    )
{
    if(pPkgList)
    {
        queue_free(&pPkgList->packages);
        solv_free(pPkgList);
    }
}

uint32_t
SolvQueueToPackageList(
    Queue* pQueue,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || !pQueue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_insertn(&pPkgList->packages, pPkgList->packages.count, pQueue->count, pQueue->elements);
cleanup: 
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvGetListResult(
    PSolvQuery pQuery,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(pQuery->result.count == 0)
    {
        dwError = ERROR_TDNF_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_insertn(&pPkgList->packages, pPkgList->packages.count, pQuery->result.count, pQuery->result.elements);

cleanup: 
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvGetSearchResult(
    PSolvQuery pQuery,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || !pQuery)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_insertn(&pPkgList->packages, pPkgList->packages.count, pQuery->result.count, pQuery->result.elements);

cleanup: 
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvGetPackageListSize(
    PSolvPackageList pPkgList
    )
{
    uint32_t dwCount = 0;
    if(pPkgList)
    {
        dwCount = pPkgList->packages.count;
    }
    return dwCount;
}

const char*
SolvGetPkgNameFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pName = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pName = pool_id2str(pSack->pPool, pSolv->name);
    }

cleanup: 
    return pName;

error:
    goto cleanup;
}

const char*
SolvGetPkgArchFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pArch = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pArch = solvable_lookup_str(pSolv, SOLVABLE_ARCH);
    }

cleanup: 
    return pArch;

error:
    goto cleanup;
}

const char*
SolvGetPkgVersionFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    char* pVersion = NULL;
    char* pEpoch = NULL;
    char* pRelease = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        dwError = SolvSplitEvr(pSack, solvable_lookup_str(pSolv, SOLVABLE_EVR),
                    &pEpoch, &pVersion, &pRelease);
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup: 
    return pVersion;

error:
    pVersion = NULL;
    goto cleanup;

}
const char*
SolvGetPkgReleaseFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    char* pVersion = NULL;
    char* pEpoch = NULL;
    char* pRelease = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        dwError = SolvSplitEvr(pSack, solvable_lookup_str(pSolv, SOLVABLE_EVR),
                    &pEpoch, &pVersion, &pRelease);
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup: 
    return pRelease;

error:
    pRelease = NULL;
    goto cleanup;

}

const char*
SolvGetPkgRepoNameFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pRepoName = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pRepoName = pSolv->repo->name;
    }

cleanup: 
    return pRepoName;

error:
    goto cleanup;

}

uint32_t
SolvGetPkgInstallSizeFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    uint32_t installSize = 0;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        installSize = solvable_lookup_num(pSolv, SOLVABLE_INSTALLSIZE, 0);
    }

cleanup: 
    return installSize;

error:
    goto cleanup;;
}

const char*
SolvGetPkgSummaryFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pSummary = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pSummary = solvable_lookup_str(pSolv, SOLVABLE_SUMMARY);
    }

cleanup: 
    return pSummary;

error:
    goto cleanup;

}

const char*
SolvGetPkgLicenseFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pLicense = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pLicense = solvable_lookup_str(pSolv, SOLVABLE_LICENSE);
    }

cleanup: 
    return pLicense;

error:
    goto cleanup;
}

const char*
SolvGetPkgDescriptionFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pDescription = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pDescription = solvable_lookup_str(pSolv, SOLVABLE_DESCRIPTION);
    }

cleanup: 
    return pDescription;

error:
    goto cleanup;

}

const char*
SolvGetPkgUrlFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pUrl = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pUrl = solvable_lookup_str(pSolv, SOLVABLE_URL);
    }

cleanup: 
    return pUrl;

error:
    goto cleanup;

}

const char*
SolvGetPkgNevrFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pNevr = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pNevr = pool_solvable2str(pSack->pPool, pSolv);
    }

cleanup: 
    return pNevr;

error:
    goto cleanup;
}

const char*
SolvGetPkgLocationFromId(
    PSolvSack pSack,
    int pkgId)
{
    uint32_t dwError = 0;
    const char* pUrl = NULL;
    Solvable *pSolv = NULL;
    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv = pool_id2solvable(pSack->pPool, pkgId);
    if(pSolv)
    {
        pUrl = solvable_get_location(pSolv, NULL);
    }

cleanup: 
    return pUrl;

error:
    goto cleanup;
}

uint32_t
SolvGetPackageId(
    PSolvPackageList pPkgList,
    int pkgIndex,
    Id* pkgId
    )
{
    uint32_t dwError = 0;
    if(!pPkgList || pkgIndex < 0 || pkgIndex >= pPkgList->packages.count)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    *pkgId = pPkgList->packages.elements[pkgIndex];

cleanup: 
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCmpEvr(
    PSolvSack pSack,
    Id pkg1,
    Id pkg2,
    int* result)
{
    uint32_t    dwError = 0;
    Solvable    *pSolv1 = NULL;
    Solvable    *pSolv2 = NULL;
    const char  *pszEvr1 = NULL;
    const char  *pszEvr2 = NULL;

    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv1 = pool_id2solvable(pSack->pPool, pkg1);
    pSolv2 = pool_id2solvable(pSack->pPool, pkg2);
    if(!pSolv1 || !pSolv2)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszEvr1 = solvable_lookup_str(pSolv1, SOLVABLE_EVR);
    pszEvr2 = solvable_lookup_str(pSolv2, SOLVABLE_EVR);

    *result = pool_evrcmp_str(pSack->pPool, pszEvr1, pszEvr2, EVRCMP_COMPARE);
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvGetLatest(
    PSolvSack pSack,
    Queue* pPkgList,
    Id pkg,
    Id* result)
{
    uint32_t dwError = 0;
    Solvable    *pSolv1 = NULL;
    Solvable    *pSolv2 = NULL;
    const char  *pszEvr1 = NULL;
    const char  *pszEvr2 = NULL;
    const char  *pszName1  = NULL;
    const char  *pszName2  = NULL;
    int pkgIter  = 0;
    int compareResult = 0;
    *result = pkg;

    if(!pSack || pkg <= 0 || !pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pSolv1 = pool_id2solvable(pSack->pPool, pkg);
    if(!pSolv1)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszName1 = pool_id2str(pSack->pPool, pSolv1->name);
    pszEvr1 = solvable_lookup_str(pSolv1, SOLVABLE_EVR);
    for( ; pkgIter < pPkgList->count;  pkgIter++)
    {
        pSolv2 = pool_id2solvable(pSack->pPool, pPkgList->elements[pkgIter]);
        if(!pSolv2)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
        }

        pszName2 = pool_id2str(pSack->pPool, pSolv2->name);

        if(!strcmp(pszName1, pszName2))
        {
            pszEvr2 = solvable_lookup_str(pSolv2, SOLVABLE_EVR);

            compareResult = pool_evrcmp_str(pSack->pPool, pszEvr2, pszEvr1, EVRCMP_COMPARE);
            if(compareResult == 1)
            {
                *result = pPkgList->elements[pkgIter];
                pSolv1 = pSolv2;
                pszEvr1 = pszEvr2;
            }
        }
    }
cleanup:
    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvFindAllInstalled(
    PSolvSack pSack,
    PSolvPackageList pPkgList)
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack || !pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery = SolvCreateQuery(pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvAddSystemRepoFilter(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvCountPkgByName(
    PSolvSack pSack,
    const char* pszName,
    int* count
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;
    PSolvPackageList pPkgList = NULL;

    if(!pSack || !pszName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery = SolvCreateQuery(pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvApplySinglePackageFilter(pQuery, pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    pPkgList = SolvCreatePackageList();
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

    *count = SolvGetPackageListSize(pPkgList);

cleanup:
    if(pPkgList)
    {
        SolvFreePackageList(pPkgList);
    }
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvFindInstalledPkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack || !pszName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery = SolvCreateQuery(pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvAddSystemRepoFilter(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySinglePackageFilter(pQuery, pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvFindAvailablePkgByName(
    PSolvSack pSack,
    const char* pszName,
    PSolvPackageList pPkgList
    )
{
    uint32_t dwError = 0;
    PSolvQuery pQuery = NULL;

    if(!pSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery = SolvCreateQuery(pSack);
    if(!pQuery)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = SolvAddAvailableRepoFilter(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplySinglePackageFilter(pQuery, pszName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvApplyListQuery(pQuery);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = SolvGetListResult(pQuery, pPkgList);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(pQuery)
    {
        SolvFreeQuery(pQuery);
    }
    return dwError;

error:
    goto cleanup;;
}

uint32_t
SolvGetTransResultsWithType(
    Transaction *trans,
    Id type,
    PSolvPackageList pPkgList
    )
{
    uint32_t  dwError = 0;
    Id pkg = 0;
    Id pkgType = 0; 
    Queue solvedPackages;
    queue_init(&solvedPackages);
    if(!pPkgList)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if (!trans)
    {
        dwError = ERROR_TDNF_SOLV_NO_SOLUTION;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (int i = 0; i < trans->steps.count; ++i)
    {
        pkg = trans->steps.elements[i];

        switch (type)
        {
            case SOLVER_TRANSACTION_OBSOLETED:
                pkgType =  transaction_type(trans, pkg, SOLVER_TRANSACTION_SHOW_OBSOLETES);
                break;
            default:
                pkgType  = transaction_type(trans, pkg, 
                     SOLVER_TRANSACTION_SHOW_ACTIVE|
                     SOLVER_TRANSACTION_SHOW_ALL);
                break;
        }

        if (type == pkgType)
            queue_push(&solvedPackages, pkg);
    }
    queue_insertn(&pPkgList->packages, pPkgList->packages.count,
             solvedPackages.count, solvedPackages.elements);
cleanup:
    queue_free(&solvedPackages);
    return dwError;

error:
    goto cleanup;
}
