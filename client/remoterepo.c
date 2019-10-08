/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
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
    double dPercent;

    UNUSED(ulNow);
    UNUSED(ulTotal);

    if (dlTotal <= 0)
        return 0;

    dPercent = ((double)dlNow / (double)dlTotal) * 100.0;
    if (!isatty(STDOUT_FILENO))
        printf("%s %3.0f%% %ld\n", (char *)pUserData, dPercent, dlNow);
    else
        printf("%-35s %10ld %5.0f%%\r", (char *)pUserData, dlNow, dPercent);

    fflush(stdout);

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

static
uint32_t
_handle_curl_cb(
    PTDNF pTdnf,
    CURL *pCurl,
    const char *pszUrl
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pOpt = NULL;

    dwError = TDNFGetCmdOpt(pTdnf, CMDOPT_CURL_INIT_CB, &pOpt);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pOpt->pfnCurlConfigCB)
    {
        pOpt->pfnCurlConfigCB(pCurl, pszUrl);
    }

error:
    if (dwError == ERROR_TDNF_FILE_NOT_FOUND)
    {
        dwError = 0;/* callback not set */
    }
    return dwError;
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
    /* lStatus reads CURLINFO_RESPONSE_CODE. Must be long */
    long lStatus = 0;

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
    /* if callback present for extra curl options */
    dwError = _handle_curl_cb(pTdnf, pCurl, pszFileUrl);
    BAIL_ON_TDNF_ERROR(dwError);

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

    if(!pTdnf->pArgs->nQuiet && pszProgressData)
    {
        //print progress only if tty or verbose is specified.
        if(isatty(STDOUT_FILENO) || pTdnf->pArgs->nVerbose)
        {
            dwError = set_progress_cb(pCurl, pszProgressData);
            BAIL_ON_TDNF_ERROR(dwError);
        }
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

    dwError = curl_easy_getinfo(pCurl,
                                CURLINFO_RESPONSE_CODE,
                                &lStatus);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if(lStatus >= 400)
    {
        fprintf(stderr,
                "Error: %ld when downloading %s\n. Please check repo url.\n",
                lStatus,
                pszFileUrl);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
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
    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }
    if(!IsNullOrEmptyString(pszFile))
    {
        unlink(pszFile);
    }

    if(pCurl && TDNFIsCurlError(dwError))
    {
        uint32_t nCurlError = dwError - ERROR_TDNF_CURL_BASE;
        fprintf(stderr,
                "curl#%u: %s\n",
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
        printf("\n");
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
