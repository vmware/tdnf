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
TDNFDownloadPackage(
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfo,
    const char* pszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    LrHandle *pRepoHandle = NULL;
    GError* pError = NULL;
    gboolean bRet = FALSE;
    char* ppszUrls[] = {NULL, NULL};
    char* pszHyPackage = NULL;
    const char* pszRepo = NULL;
    char* pszUserPass = NULL;
    char* pszBaseUrl = NULL;
    const char* pszPkgName = NULL;
    
    if(!pTdnf || !pInfo || IsNullOrEmptyString(pszRpmCacheDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pRepoHandle = lr_handle_init();

    if(!pRepoHandle)
    {
        //TODO: Add repo specific
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Get package details
    pszHyPackage = pInfo->pszLocation;
    pszPkgName = pInfo->pszName;
    pszRepo = pInfo->pszRepoName;

    dwError = TDNFRepoGetBaseUrl(pTdnf, pszRepo, &pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    ppszUrls[0] = pszBaseUrl;

    lr_handle_setopt(pRepoHandle, NULL, LRO_URLS, ppszUrls);
    lr_handle_setopt(pRepoHandle, NULL, LRO_REPOTYPE, LR_YUMREPO);
    lr_handle_setopt(pRepoHandle, NULL, LRO_SSLVERIFYPEER, 1);
    lr_handle_setopt(pRepoHandle, NULL, LRO_SSLVERIFYHOST, 2);

    dwError = TDNFRepoGetUserPass(pTdnf, pszRepo, &pszUserPass);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pszUserPass))
    {
        lr_handle_setopt(pRepoHandle, NULL, LRO_USERPWD, pszUserPass);
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

    dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, pRepoHandle);
    BAIL_ON_TDNF_ERROR(dwError);

    bRet = lr_download_package (
                         pRepoHandle,
                         pszHyPackage,
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
