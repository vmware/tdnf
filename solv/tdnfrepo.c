
/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static uint32_t
SolvLoadRepomd(
    Repo* pRepo,
    const char* pszRepomd
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || !pszRepomd)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = fopen(pszRepomd, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_repomdxml(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup: 
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

static uint32_t
SolvLoadRepomdPrimary(
    Repo* pRepo,
    const char* pszPrimary
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || !pszPrimary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = solv_xfopen(pszPrimary, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, 0, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup: 
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;

}

static uint32_t
SolvLoadRepomdFilelists(
    Repo* pRepo,
    const char* pszFilelists
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if(!pRepo || !pszFilelists)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = solv_xfopen(pszFilelists, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, "FL", REPO_EXTEND_SOLVABLES))
    {
        dwError = ERROR_TDNF_SOLV_FAILED;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup: 
    if(fp != NULL)
        fclose(fp);
    return dwError;

error:
    goto cleanup;
}

static uint32_t
SolvLoadRepomdUpdateinfo(
    Repo* pRepo,
    const char* pszUpdateinfo)
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || !pszUpdateinfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = solv_xfopen(pszUpdateinfo, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_updateinfoxml(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup: 
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvReadYumRepo(
    PSolvSack pSack,
    const char *pszRepoName,
    const char *pszRepomd,
    const char *pszPrimary,
    const char *pszFilelists,
    const char *pszUpdateinfo
    )
{
    uint32_t dwError = 0;
    Repo* pRepo = NULL;
    Pool* pPool = NULL;
    if(!pSack || !pSack->pPool || !pszRepoName || !pszRepomd || !pszPrimary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pPool = pSack->pPool;
    pRepo = repo_create(pPool, pszRepoName);
    if( !pRepo )
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = SolvLoadRepomd(pRepo, pszRepomd);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);


    dwError = SolvLoadRepomdPrimary(pRepo, pszPrimary);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if(pszFilelists)
    {
        dwError = SolvLoadRepomdFilelists(pRepo, pszFilelists);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszUpdateinfo)
    {
        dwError = SolvLoadRepomdUpdateinfo(pRepo, pszUpdateinfo);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pool_createwhatprovides(pPool);

cleanup: 

    return dwError;

error:
    if(pRepo)
    {
        repo_free(pRepo, 1);
    }
    goto cleanup;
}

uint32_t
SolvCountPackages(
    PSolvSack pSack,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t cnt = 0;
    Id p = 0;
    if(!pSack || !pSack->pPool || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    Pool* pool = pSack->pPool;
    FOR_POOL_SOLVABLES(p)
    {
        cnt++;
    }
    *pdwCount = cnt;
cleanup: 
    return dwError;
error:
    goto cleanup;

}

