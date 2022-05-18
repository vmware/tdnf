/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

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

    if(!pTdnf || !ppCtx)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszDataDir,
                           pTdnf->pArgs->pszInstallRoot,
                           TDNF_DEFAULT_DATA_LOCATION,
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszHistoryDb,
            pszDataDir,
            "history.db",
            NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nMustExist)
    {
        int nExists = 0;
        dwError = TDNFIsFileOrSymlink(pszHistoryDb, &nExists);
        BAIL_ON_TDNF_ERROR(dwError);
        if (!nExists)
        {
            dwError = ERROR_TDNF_HISTORY_NODB;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    dwError = TDNFUtilsMakeDir(pszDataDir);
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