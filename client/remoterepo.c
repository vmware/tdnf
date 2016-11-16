/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : remoterepo.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#include "includes.h"

static int
lrProgressCB(
    void *pUserData,
    gdouble dTotalToDownload,
    gdouble dDownloaded
    )
{
    if(dTotalToDownload > 0.00)
    {
        gdouble dPercent = (dDownloaded/dTotalToDownload) * 100.0;
        fprintf(
            stdout,
            "%-35s %10.0f  %5.0f%%\r",
            (char*)pUserData,
            dDownloaded,
            dPercent);
        fflush(stdout);
    }
    return 0;
}

uint32_t
set_progress_cb(
    LrHandle *pRepoHandle,
    const char *pszPkgName
    )
{
    uint32_t dwError = 0;
    gboolean bRet = FALSE;

    if(!pRepoHandle || IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    bRet = lr_handle_setopt(pRepoHandle, NULL, LRO_PROGRESSCB, lrProgressCB);
    if(bRet == FALSE)
    {
        //TODO: Add repo specific
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    bRet = lr_handle_setopt(pRepoHandle, NULL, LRO_PROGRESSDATA, pszPkgName);
    if(bRet == FALSE)
    {
        //TODO: Add repo specific
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFDownloadPackage(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    const char* pszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    LrHandle *pRepoHandle = NULL;
    GError* pError = NULL;
    gboolean bRet = FALSE;
    char* ppszUrls[] = {NULL, NULL};
    char* pszUserPass = NULL;
    char* pszBaseUrl = NULL;
    int nSilent = 0;

    if(!pTdnf || 
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       IsNullOrEmptyString(pszRepoName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSilent = pTdnf->pArgs->nNoOutput;

    pRepoHandle = lr_handle_init();

    if(!pRepoHandle)
    {
        //TODO: Add repo specific
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetBaseUrl(pTdnf, pszRepoName, &pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    ppszUrls[0] = pszBaseUrl;

    lr_handle_setopt(pRepoHandle, NULL, LRO_URLS, ppszUrls);
    lr_handle_setopt(pRepoHandle, NULL, LRO_REPOTYPE, LR_YUMREPO);
    lr_handle_setopt(pRepoHandle, NULL, LRO_SSLVERIFYPEER, 1);
    lr_handle_setopt(pRepoHandle, NULL, LRO_SSLVERIFYHOST, 2);

    dwError = TDNFRepoGetUserPass(pTdnf, pszRepoName, &pszUserPass);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pszUserPass))
    {
        lr_handle_setopt(pRepoHandle, NULL, LRO_USERPWD, pszUserPass);
    }

    if(!nSilent)
    {
        dwError = set_progress_cb(pRepoHandle, pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, pRepoHandle);
    BAIL_ON_TDNF_ERROR(dwError);

    bRet = lr_download_package (
               pRepoHandle,
               pszPackageLocation,
               pszRpmCacheDir,
               LR_CHECKSUM_UNKNOWN,
               NULL,
               0,
               NULL,
               TRUE,
               &pError);

    if(bRet == FALSE)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    fprintf(stdout, "\n");
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszBaseUrl);
    TDNF_SAFE_FREE_MEMORY(pszUserPass);
    if(pRepoHandle)
    {
        lr_handle_free(pRepoHandle);
    }
    return dwError;

error:
    if(pError)
    {
        fprintf(
            stderr,
            "Error during download: %d: %s\n",
            pError->code,
            pError->message
            );
        g_error_free(pError);
    }
    goto cleanup;
}
