#include "includes.h"

uint32_t 
SolvCreateSack(PSolvSack* ppSack)
{
    uint32_t dwError = 0;
    if(!ppSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    PSolvSack pSack = solv_calloc(1, sizeof(SolvSack));
    if(!pSack)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSack->pPool = NULL;;
    pSack->pdwCommandLinePkgs = NULL;
    pSack->dwNumOfCommandPkgs = 0;
    pSack->pszCacheDir = NULL;
    pSack->pszRootDir = NULL;

    *ppSack = pSack;
cleanup: 
    return dwError;

error:
    goto cleanup;
}

void 
SolvFreeSack(
    PSolvSack pSack)
{
    if(pSack)
    {
        Pool* pPool = pSack->pPool;
        if(pPool)
        {
            pool_free(pPool);
        }
        if(pSack->pdwCommandLinePkgs)
        {
            solv_free(pSack->pdwCommandLinePkgs);
        }
        if(pSack->pszCacheDir)
        {
            solv_free(pSack->pszCacheDir);
        }
        if(pSack->pszRootDir)
        {
            solv_free(pSack->pszRootDir);
        }
        solv_free(pSack);
    }

}

static uint32_t 
SolvReadInstalledRpms(
    Pool* pPool,
    Repo** ppRepo,
    const char*  pszCacheFileName
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    FILE *pCacheFile = NULL;
    int  dwFlags = 0;
    if(!pPool || !ppRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pRepo = repo_create(pPool, SYSTEM_REPO_NAME);
    if(pRepo == NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszCacheFileName)
    {
        pCacheFile = fopen(pszCacheFileName, "r");
    }

    dwFlags = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    dwError = repo_add_rpmdb_reffp(pRepo, pCacheFile, dwFlags);

    if (dwError)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    *ppRepo = pRepo;

cleanup: 
    if (pCacheFile)
        fclose(pCacheFile);
    return dwError;

error:
    if(pRepo)
    {
        repo_free(pRepo, 1);
    }
    goto cleanup;
}

uint32_t
SolvInitSack(
    PSolvSack *ppSack,
    const char* pszCacheDir,
    const char* pszRootDir
    )
{
    uint32_t dwError = 0;
    Pool* pPool = NULL;
    Repo *pRepo = NULL;
    struct utsname systemInfo = {};
    PSolvSack pSack = NULL;

    if(!ppSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = SolvCreateSack(&pSack);

    if (pszCacheDir != NULL)
    {
        pSack->pszCacheDir = solv_strdup(pszCacheDir);
        if(!pSack->pszCacheDir)
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    pPool = pool_create();
    if(pPool == NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszRootDir != NULL)
    {
        pool_set_rootdir(pPool, pszRootDir);
    }

    if (uname(&systemInfo))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    
    pool_setarch(pPool, systemInfo.machine);
    pool_set_flag(pPool, POOL_FLAG_ADDFILEPROVIDESFILTERED, 1);

    dwError = SolvReadInstalledRpms(pPool, &pRepo, pszCacheDir);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    pool_set_installed(pPool, pRepo);

    pSack->pPool = pPool;
    *ppSack = pSack;

cleanup: 
    return dwError;

error:
    if(pPool)
    {
        pool_free(pPool);
    }
    if(pSack)
    {
        SolvFreeSack(pSack);
    }
    goto cleanup;
}