/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFHistorySyncState(
    PTDNF pTdnf,
    struct history_ctx *ctx
)
{
    uint32_t dwError = 0;
    rpmts ts = NULL;
    int rc = 0;

    ts = rpmtsCreate();
    if(!ts)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (rpmtsOpenDB(ts, O_RDONLY))
    {
        dwError = ERROR_TDNF_RPMTS_OPENDB_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(rpmtsSetRootDir(ts, pTdnf->pArgs->pszInstallRoot))
    {
        dwError = ERROR_TDNF_RPMTS_BAD_ROOT_DIR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    rc = history_sync(ctx, ts);
    if (rc != 0)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if (ts)
    {
        rpmtsCloseDB(ts);
        rpmtsFree(ts);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFGetHistoryCtx(
    PTDNF pTdnf,
    struct history_ctx **ppCtx,
    int nMustExist
)
{
    uint32_t dwError = 0;
    char *pszDataDir = NULL;
    char *pszHistoryDb = NULL;
    struct history_ctx *ctx = NULL;
    int nExists = 0;

    if(!pTdnf || !ppCtx)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszDataDir,
                           pTdnf->pArgs->pszInstallRoot,
                           pTdnf->pConf->pszPersistDir,
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszHistoryDb,
            pszDataDir,
            HISTORY_DB_FILE,
            NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    nExists = 0;
    dwError = TDNFIsFileOrSymlink(pszHistoryDb, &nExists);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nMustExist && !nExists)
    {
        dwError = ERROR_TDNF_HISTORY_NODB;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFUtilsMakeDirs(pszDataDir);
    if (dwError == ERROR_TDNF_ALREADY_EXISTS)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    ctx = create_history_ctx(pszHistoryDb);
    if (ctx == NULL)
    {
        dwError = ERROR_TDNF_HISTORY_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppCtx = ctx;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszDataDir);
    TDNF_SAFE_FREE_MEMORY(pszHistoryDb);
    return dwError;
error:
    goto cleanup;
}
