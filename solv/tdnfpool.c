/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
SolvCreateSack(
    PSolvSack* ppSack
    )
{
    uint32_t dwError = 0;
    PSolvSack pSack = NULL;

    if(!ppSack)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(SolvSack), (void **)&pSack);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppSack = pSack;
cleanup:
    return dwError;

error:
    if(ppSack)
    {
        *ppSack = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pSack);
    goto cleanup;
}

void
SolvFreeSack(
    PSolvSack pSack
    )
{
    if(pSack)
    {
        Pool* pPool = pSack->pPool;
        if(pPool)
        {
            pool_free(pPool);
        }
        TDNF_SAFE_FREE_MEMORY(pSack->pszCacheDir);
        TDNF_SAFE_FREE_MEMORY(pSack->pszRootDir);
        TDNF_SAFE_FREE_MEMORY(pSack);
    }

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
    BAIL_ON_TDNF_ERROR(dwError);

    if (pszCacheDir != NULL)
    {
        dwError = TDNFAllocateString(pszCacheDir, &pSack->pszCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
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

        dwError = TDNFAllocateString(pszRootDir, &pSack->pszRootDir);
        BAIL_ON_TDNF_ERROR(dwError);
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
