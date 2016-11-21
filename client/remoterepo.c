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
progress_cb(
    void *pUserData,
    curl_off_t dlTotal,
    curl_off_t dlNow,
    curl_off_t ulTotal,
    curl_off_t ulNow
    )
{
    if(dlTotal > 0)
    {
        double dPercent = ((double)dlNow / (double)dlTotal) * 100.0;
        fprintf(
            stdout,
            "%-35s %10ld  %5.0f%%\r",
            (char*)pUserData,
            dlNow,
            dPercent);
        fflush(stdout);
    }
    return 0;
}

uint32_t
set_progress_cb(
    CURL *pCurl,
    const char *pszData
    )
{
    uint32_t dwError = 0;

    if(!pCurl || IsNullOrEmptyString(pszData))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, pszData);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFDownloadFile(
    PTDNF pTdnf,
    const char *pszRepo,
    const char *pszFileUrl,
    const char *pszFile,
    const char *pszProgressData
    )
{
    uint32_t dwError = 0;
    CURL *pCurl = NULL;
    FILE *fp = NULL;
    char *pszUserPass = NULL;

    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszFileUrl) ||
       IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCurl = curl_easy_init();
    if(!pCurl)
    {
        dwError = ERROR_TDNF_CURL_INIT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetUserPass(pTdnf, pszRepo, &pszUserPass);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pszUserPass))
    {
        dwError = curl_easy_setopt(
                      pCurl,
                      CURLOPT_USERPWD,
                      pszUserPass);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_URL, pszFileUrl);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if(pszProgressData)
    {
        dwError = set_progress_cb(pCurl, pszProgressData);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fp = fopen(pszFile, "wb");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_perform(pCurl);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszUserPass);
    if(fp)
    {
        fclose(fp);
    }
    if(pCurl)
    {
        curl_easy_cleanup(pCurl);
    }
    return dwError;

error:
    if(pCurl && TDNFIsCurlError(dwError))
    {
        uint32_t nCurlError = dwError - ERROR_TDNF_CURL_BASE;
        fprintf(stderr,
                "download error: %d - %s\n",
                nCurlError,
                curl_easy_strerror(nCurlError));
    }
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
    char* pszBaseUrl = NULL;
    int nSilent = 0;
    char *pszPackageUrl = NULL;
    char *pszPackageFile = NULL;
    char *pszCopyOfPackageLocation = NULL;

    if(!pTdnf || 
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       IsNullOrEmptyString(pszRepoName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nSilent = pTdnf->pArgs->nQuiet;

    dwError = TDNFRepoGetBaseUrl(pTdnf, pszRepoName, &pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(&pszPackageUrl,
                                       "%s/%s",
                                       pszBaseUrl,
                                       pszPackageLocation);

    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszPackageLocation,
                                 &pszCopyOfPackageLocation);
        BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(&pszPackageFile,
                                       "%s/%s",
                                       pszRpmCacheDir,
                                       basename(pszCopyOfPackageLocation));
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadFile(pTdnf,
                               pszRepoName,
                               pszPackageUrl,
                               pszPackageFile,
                               nSilent ? NULL : pszPkgName);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!nSilent)
    {
        fprintf(stdout, "\n");
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszPackageUrl);
    TDNF_SAFE_FREE_MEMORY(pszCopyOfPackageLocation);
    TDNF_SAFE_FREE_MEMORY(pszPackageFile);
    TDNF_SAFE_FREE_MEMORY(pszBaseUrl);
    return dwError;

error:
    goto cleanup;
}
