/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
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
    uint32_t dPercent;
    pcb_data *pData = (pcb_data *)pUserData;

    UNUSED(ulNow);
    UNUSED(ulTotal);

    if (dlTotal <= 0)
    {
        return 0;
    }

    if (dlNow < dlTotal)
    {
        time(&pData->cur_time);
        if (pData->prev_time &&
            difftime(pData->cur_time, pData->prev_time) < 1.0)
        {
            return 0;
        }
        pData->prev_time = pData->cur_time;
        dPercent = (uint32_t)(((double)dlNow / (double)dlTotal) * 100.0);
    }
    else
    {
        pData->prev_time = 0;
        dPercent = 100;
    }

    if (!isatty(STDOUT_FILENO))
    {
        pr_info("%s %u%% %ld\n", pData->pszData, dPercent, dlNow);
    }
    else
    {
        pr_info("%-35s %10ld %u%%\r", pData->pszData, dlNow, dPercent);
    }

    fflush(stdout);

    return 0;
}

static uint32_t
set_progress_cb(
    CURL *pCurl,
    const char *pszData
    )
{
    uint32_t dwError = 0;
    static pcb_data pData;

    if(!pCurl || IsNullOrEmptyString(pszData))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    memset(&pData, 0, sizeof(pcb_data));
    strncpy(pData.pszData, pszData, sizeof(pData.pszData) - 1);
    /* coverity[bad_sizeof] */
    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, &pData);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}


/*
 * Connections, DNS lookups and TLS sessions are shared by all downloads of a
 * handle, so downloading many files from the same server does not need a new
 * connection and TLS handshake for each file. Failing to set up sharing is
 * not an error, downloads just don't share anything then.
 */
static CURLSH *
TDNFGetCurlShare(
    PTDNF pTdnf
    )
{
    CURLSH *pShare = NULL;

    if (pTdnf->pCurlShare)
    {
        return pTdnf->pCurlShare;
    }

    pShare = curl_share_init();
    if (!pShare)
    {
        return NULL;
    }
    if (curl_share_setopt(pShare, CURLSHOPT_SHARE,
                          CURL_LOCK_DATA_DNS) != CURLSHE_OK ||
        curl_share_setopt(pShare, CURLSHOPT_SHARE,
                          CURL_LOCK_DATA_SSL_SESSION) != CURLSHE_OK
#if LIBCURL_VERSION_NUM >= 0x073900 /* 7.57.0 */
        || curl_share_setopt(pShare, CURLSHOPT_SHARE,
                             CURL_LOCK_DATA_CONNECT) != CURLSHE_OK
#endif
        )
    {
        curl_share_cleanup(pShare);
        return NULL;
    }
    pTdnf->pCurlShare = pShare;
    return pShare;
}

void
TDNFFreeCurlShare(
    PTDNF pTdnf
    )
{
    if (pTdnf && pTdnf->pCurlShare)
    {
        curl_share_cleanup(pTdnf->pCurlShare);
        pTdnf->pCurlShare = NULL;
    }
}

/* Apply the settings common to all downloads from pRepo to pCurl. */
static uint32_t
TDNFCurlSetup(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    CURL *pCurl,
    const char *pszUrl
    )
{
    uint32_t dwError = 0;
    char *pszUserPass = NULL;
    CURLSH *pShare = NULL;

    dwError = TDNFRepoGetUserPass(pTdnf, pRepo, &pszUserPass);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pszUserPass))
    {
        dwError = curl_easy_setopt(
                      pCurl,
                      CURLOPT_USERPWD,
                      pszUserPass);
        BAIL_ON_TDNF_CURL_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pTdnf->pConf->pszUserAgentHeader))
    {
        dwError = curl_easy_setopt(
                      pCurl,
                      CURLOPT_USERAGENT,
                      pTdnf->pConf->pszUserAgentHeader);
        BAIL_ON_TDNF_CURL_ERROR(dwError);
    }

    dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplyDownloadSettings(pRepo, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplySSLSettings(pRepo, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_URL, pszUrl);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if (pTdnf->pConf->nConnectTimeout > 0)
    {
        curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT,
                          (long)pTdnf->pConf->nConnectTimeout);
    }

    pShare = TDNFGetCurlShare(pTdnf);
    if (pShare)
    {
        dwError = curl_easy_setopt(pCurl, CURLOPT_SHARE, pShare);
        BAIL_ON_TDNF_CURL_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszUserPass);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadFileFromRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszLocation,
    const char *pszFile,
    const char *pszProgressData
)
{
    uint32_t dwError = 0;
    char *pszUrl = NULL;

    if(!pTdnf ||
       !pTdnf->pArgs || !pRepo ||
       IsNullOrEmptyString(pszLocation) ||
       IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pRepo->ppszBaseUrls && pRepo->ppszBaseUrls[0] &&
        strstr(pszLocation, "://") == NULL) {
        /* Try one base URL after the other until we succeed */
        /* Note: this can be improved:
         * 1) we could start with the last good URL next time instead of
         *    starting with 0 each time
         * 2) we could store a list of known bad/good URLs
         */
        for (int i = 0; pRepo->ppszBaseUrls[i]; i++) {
            dwError = TDNFJoinPath(&pszUrl, pRepo->ppszBaseUrls[i], pszLocation, NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFDownloadFile(pTdnf, pRepo, pszUrl, pszFile, pszProgressData);
            if (dwError == 0) {
                break;
            }
            if (pRepo->ppszBaseUrls[i + 1]) {
                pr_err("Warning: failed to download %s, trying next base URL\n", pszUrl);
            }
            TDNF_SAFE_FREE_MEMORY(pszUrl);
        }
    } else {
        /* pszLocation is already an absolute URL (xml:base was absolute), or
           there is no base URL (command line packages): use it directly. */
        dwError = TDNFDownloadFile(pTdnf, pRepo, pszLocation, pszFile, pszProgressData);
    }
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszUrl);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadFile(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszFileUrl,
    const char *pszFile,
    const char *pszProgressData
    )
{
    uint32_t dwError = 0;
    CURL *pCurl = NULL;
    FILE *fp = NULL;
    char *pszFileTmp = NULL;
    /* lStatus reads CURLINFO_RESPONSE_CODE. Must be long */
    long lStatus = 0;
    int i;
    int fd = -1;
    int nTempCreated = 0;
    int nNoOutput = 1;

    /* TDNFFetchRemoteGPGKey sends pszProgressData as NULL */
    if(!pTdnf ||
       !pRepo ||
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

    dwError = TDNFCurlSetup(pTdnf, pRepo, pCurl, pszFileUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    if (!pTdnf->pArgs->nQuiet && pszProgressData != NULL)
    {
        //print progress only if tty or verbose is specified.
        if (isatty(STDOUT_FILENO) || pTdnf->pArgs->nVerbose)
        {
            dwError = set_progress_cb(pCurl, pszProgressData);
            BAIL_ON_TDNF_ERROR(dwError);
            nNoOutput = 0;
        }
    }

    for(i = 0; i <= pRepo->nRetries; i++)
    {
        TDNF_SAFE_FREE_MEMORY(pszFileTmp);
        dwError = TDNFAllocateStringPrintf(&pszFileTmp, "%s.tmp.XXXXXX", pszFile);
        BAIL_ON_TDNF_ERROR(dwError);

        fd = mkstemp(pszFileTmp);
        if (fd == -1)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }
        nTempCreated = 1;
        fp = fdopen(fd, "wb");
        if(!fp)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }

        fd = -1; /* fp now owns the descriptor. */
        dwError = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
        BAIL_ON_TDNF_CURL_ERROR(dwError);

        if (i > 0)
        {
            pr_info("retrying %d/%d\n", i, pRepo->nRetries);
        }
        dwError = curl_easy_perform(pCurl);
        if (dwError == CURLE_OK)
        {
            break;
        }
        if (i == pRepo->nRetries || TDNFCurlErrorIsFatal(dwError))
        {
            pr_err("Error: failed to download %s: %s\n",
                   pszFileUrl, curl_easy_strerror(dwError));
            BAIL_ON_TDNF_CURL_ERROR(dwError);
        }
        fclose(fp);
        fp = NULL;
        if (unlink(pszFileTmp) == -1)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }
        nTempCreated = 0;
    }

    /* finish progress line output,
       but only if progress was enabled */
    if (!nNoOutput) {
        pr_info("\n");
    }

    dwError = curl_easy_getinfo(pCurl,
                                CURLINFO_RESPONSE_CODE,
                                &lStatus);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if(lStatus >= 400)
    {
        pr_err(
                "Error: %ld when downloading %s. Please check repo url "
                "or refresh metadata with 'tdnf makecache'.\n",
                lStatus,
                pszFileUrl);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        if (!fp)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if (fflush(fp) == EOF ||
            fchmod(fileno(fp), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) == -1)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }
        if (fclose(fp) == EOF)
        {
            fp = NULL;
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }
        fp = NULL;
        if (rename(pszFileTmp, pszFile) == -1)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        nTempCreated = 0;
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFileTmp);
    if(fp)
    {
        /* coverity[dead_error_line] */
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
    if (fd != -1)
    {
        close(fd);
    }
    if(nTempCreated)
    {
        unlink(pszFileTmp);
    }

    goto cleanup;
}

/*
 * Parallel downloads with the curl multi interface.
 *
 * Failure handling matches TDNFDownloadFileFromRepo(): a transfer is retried
 * on the same URL up to the repo's retries setting unless the curl error is
 * fatal, then the next base URL is tried. An HTTP error moves on to the next
 * base URL directly. The first file that cannot be downloaded from any URL
 * aborts all other transfers.
 */

typedef struct _TDNF_XFER
{
    PTDNF_DOWNLOAD_ITEM pItem;
    CURL *pCurl;
    FILE *fp;
    char *pszUrl;
    char *pszFileTmp;
    int nBaseUrl;       /* base URL in use, -1 if pszLocation is a URL */
    int nTry;
    curl_off_t nNow;    /* bytes received by the current attempt */
} TDNF_XFER, *PTDNF_XFER;

typedef struct _TDNF_XFER_PROGRESS
{
    int nShow;          /* print per file lines */
    int nTty;           /* also print an updating summary line */
    size_t nItems;
    size_t nDone;
    uint64_t nBytesDone;
    uint64_t nBytesTotal;
    time_t tLast;
    size_t nLineLen;    /* length of the summary line on screen */
} TDNF_XFER_PROGRESS, *PTDNF_XFER_PROGRESS;

static int
xfer_progress_cb(
    void *pUserData,
    curl_off_t dlTotal,
    curl_off_t dlNow,
    curl_off_t ulTotal,
    curl_off_t ulNow
    )
{
    UNUSED(dlTotal);
    UNUSED(ulTotal);
    UNUSED(ulNow);

    ((PTDNF_XFER)pUserData)->nNow = dlNow;
    return 0;
}

static void
TDNFXferClearLine(
    PTDNF_XFER_PROGRESS pProgress
    )
{
    if (pProgress->nLineLen)
    {
        pr_info("\r%*s\r", (int)pProgress->nLineLen, "");
        pProgress->nLineLen = 0;
    }
}

static void
TDNFXferShowSummary(
    PTDNF_XFER_PROGRESS pProgress,
    PTDNF_XFER pXfers,
    size_t nStarted,
    int nForce
    )
{
    time_t tNow = time(NULL);
    uint64_t nBytes = pProgress->nBytesDone;
    char *pszNow = NULL;
    char *pszTotal = NULL;
    char szLine[80];
    int nLen;
    size_t i;

    if (!pProgress->nTty ||
        (!nForce && tNow == pProgress->tLast))
    {
        return;
    }
    pProgress->tLast = tNow;

    for (i = 0; i < nStarted; i++)
    {
        if (pXfers[i].pCurl && pXfers[i].nNow > 0)
        {
            nBytes += (uint64_t)pXfers[i].nNow;
        }
    }

    if (TDNFUtilsFormatSize(nBytes, &pszNow) ||
        TDNFUtilsFormatSize(pProgress->nBytesTotal, &pszTotal))
    {
        goto cleanup;
    }

    if (pProgress->nBytesTotal)
    {
        nLen = snprintf(szLine, sizeof(szLine),
                        "Downloading %zu/%zu: %s / %s",
                        pProgress->nDone, pProgress->nItems,
                        pszNow, pszTotal);
    }
    else
    {
        nLen = snprintf(szLine, sizeof(szLine),
                        "Downloading %zu/%zu: %s",
                        pProgress->nDone, pProgress->nItems, pszNow);
    }
    if (nLen < 0)
    {
        goto cleanup;
    }
    TDNFXferClearLine(pProgress);
    pr_info("%s\r", szLine);
    fflush(stdout);
    pProgress->nLineLen = strlen(szLine);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszNow);
    TDNF_SAFE_FREE_MEMORY(pszTotal);
}

static void
TDNFXferShowDone(
    PTDNF_XFER_PROGRESS pProgress,
    PTDNF_XFER pXfer
    )
{
    if (!pProgress->nShow)
    {
        return;
    }
    TDNFXferClearLine(pProgress);
    /* same format as progress_cb() uses for a finished download */
    if (pProgress->nTty)
    {
        pr_info("%-35s %10ld 100%%\n", pXfer->pItem->pszName, (long)pXfer->nNow);
    }
    else
    {
        pr_info("%s 100%% %ld\n", pXfer->pItem->pszName, (long)pXfer->nNow);
    }
}

/* Close and remove the temporary file of the current attempt, if any. */
static void
TDNFXferDiscardFile(
    PTDNF_XFER pXfer
    )
{
    if (pXfer->fp)
    {
        fclose(pXfer->fp);
        pXfer->fp = NULL;
    }
    if (pXfer->pszFileTmp)
    {
        unlink(pXfer->pszFileTmp);
        TDNF_SAFE_FREE_MEMORY(pXfer->pszFileTmp);
    }
}

static void
TDNFXferFree(
    CURLM *pMulti,
    PTDNF_XFER pXfer
    )
{
    if (pXfer->pCurl)
    {
        curl_multi_remove_handle(pMulti, pXfer->pCurl);
        curl_easy_cleanup(pXfer->pCurl);
        pXfer->pCurl = NULL;
    }
    TDNFXferDiscardFile(pXfer);
    TDNF_SAFE_FREE_MEMORY(pXfer->pszUrl);
}

/* Start (or restart) the transfer from the current URL. */
static uint32_t
TDNFXferStart(
    PTDNF pTdnf,
    CURLM *pMulti,
    PTDNF_XFER pXfer
    )
{
    uint32_t dwError = 0;
    PTDNF_DOWNLOAD_ITEM pItem = pXfer->pItem;
    PTDNF_REPO_DATA pRepo = pItem->pRepo;
    int fd = -1;

    TDNF_SAFE_FREE_MEMORY(pXfer->pszUrl);
    if (pXfer->nBaseUrl >= 0)
    {
        dwError = TDNFJoinPath(&pXfer->pszUrl,
                               pRepo->ppszBaseUrls[pXfer->nBaseUrl],
                               pItem->pszLocation, NULL);
    }
    else
    {
        dwError = TDNFAllocateString(pItem->pszLocation, &pXfer->pszUrl);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if (pXfer->pCurl)
    {
        curl_easy_reset(pXfer->pCurl);
    }
    else
    {
        pXfer->pCurl = curl_easy_init();
        if (!pXfer->pCurl)
        {
            dwError = ERROR_TDNF_CURL_INIT;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    dwError = TDNFCurlSetup(pTdnf, pRepo, pXfer->pCurl, pXfer->pszUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(&pXfer->pszFileTmp, "%s.tmp.XXXXXX",
                                       pItem->pszFile);
    BAIL_ON_TDNF_ERROR(dwError);

    fd = mkstemp(pXfer->pszFileTmp);
    if (fd == -1)
    {
        dwError = errno;
        TDNF_SAFE_FREE_MEMORY(pXfer->pszFileTmp);
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }
    pXfer->fp = fdopen(fd, "wb");
    if (!pXfer->fp)
    {
        dwError = errno;
        close(fd);
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }

    dwError = curl_easy_setopt(pXfer->pCurl, CURLOPT_WRITEDATA, pXfer->fp);
    BAIL_ON_TDNF_CURL_ERROR(dwError);
    dwError = curl_easy_setopt(pXfer->pCurl, CURLOPT_XFERINFOFUNCTION,
                               xfer_progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);
    dwError = curl_easy_setopt(pXfer->pCurl, CURLOPT_XFERINFODATA, pXfer);
    BAIL_ON_TDNF_CURL_ERROR(dwError);
    dwError = curl_easy_setopt(pXfer->pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);
    dwError = curl_easy_setopt(pXfer->pCurl, CURLOPT_PRIVATE, (char *)pXfer);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    pXfer->nNow = 0;
    if (curl_multi_add_handle(pMulti, pXfer->pCurl) != CURLM_OK)
    {
        dwError = ERROR_TDNF_CURL_INIT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    TDNFXferDiscardFile(pXfer);
    goto cleanup;
}

/* Move a completely downloaded file into place. */
static uint32_t
TDNFXferCommit(
    PTDNF_XFER pXfer
    )
{
    uint32_t dwError = 0;
    FILE *fp = pXfer->fp;

    pXfer->fp = NULL;
    if (fflush(fp) == EOF ||
        fchmod(fileno(fp), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) == -1)
    {
        dwError = errno;
        fclose(fp);
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }
    if (fclose(fp) == EOF)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }
    if (rename(pXfer->pszFileTmp, pXfer->pItem->pszFile) == -1)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    TDNF_SAFE_FREE_MEMORY(pXfer->pszFileTmp);

cleanup:
    return dwError;
error:
    goto cleanup;
}

/*
 * Handle a finished transfer. Sets *pnRestarted if the transfer was restarted
 * with a retry or the next base URL, otherwise the transfer is complete.
 */
static uint32_t
TDNFXferDone(
    PTDNF pTdnf,
    CURLM *pMulti,
    PTDNF_XFER pXfer,
    CURLcode result,
    int *pnRestarted
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_DATA pRepo = pXfer->pItem->pRepo;
    long lStatus = 0;

    *pnRestarted = 0;
    curl_multi_remove_handle(pMulti, pXfer->pCurl);

    if (result == CURLE_OK)
    {
        dwError = curl_easy_getinfo(pXfer->pCurl, CURLINFO_RESPONSE_CODE,
                                    &lStatus);
        BAIL_ON_TDNF_CURL_ERROR(dwError);

        if (lStatus < 400)
        {
            curl_off_t nSize = 0;

            if (curl_easy_getinfo(pXfer->pCurl, CURLINFO_SIZE_DOWNLOAD_T,
                                  &nSize) == CURLE_OK)
            {
                pXfer->nNow = nSize;
            }
            dwError = TDNFXferCommit(pXfer);
            BAIL_ON_TDNF_ERROR(dwError);
            goto cleanup;
        }
        pr_err("Error: %ld when downloading %s. Please check repo url "
               "or refresh metadata with 'tdnf makecache'.\n",
               lStatus, pXfer->pszUrl);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
    }
    else
    {
        TDNFXferDiscardFile(pXfer);
        if (pXfer->nTry < pRepo->nRetries && !TDNFCurlErrorIsFatal(result))
        {
            pXfer->nTry++;
            pr_info("retrying %d/%d\n", pXfer->nTry, pRepo->nRetries);
            dwError = TDNFXferStart(pTdnf, pMulti, pXfer);
            BAIL_ON_TDNF_ERROR(dwError);
            *pnRestarted = 1;
            goto cleanup;
        }
        pr_err("Error: failed to download %s: %s\n",
               pXfer->pszUrl, curl_easy_strerror(result));
        dwError = ERROR_TDNF_CURL_BASE + result;
    }

    TDNFXferDiscardFile(pXfer);
    if (pXfer->nBaseUrl >= 0 && pRepo->ppszBaseUrls[pXfer->nBaseUrl + 1])
    {
        pr_err("Warning: failed to download %s, trying next base URL\n",
               pXfer->pszUrl);
        pXfer->nBaseUrl++;
        pXfer->nTry = 0;
        dwError = TDNFXferStart(pTdnf, pMulti, pXfer);
        BAIL_ON_TDNF_ERROR(dwError);
        *pnRestarted = 1;
    }
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadFiles(
    PTDNF pTdnf,
    PTDNF_DOWNLOAD_ITEM pItems,
    size_t nItems
    )
{
    uint32_t dwError = 0;
    CURLM *pMulti = NULL;
    PTDNF_XFER pXfers = NULL;
    TDNF_XFER_PROGRESS progress = {0};
    size_t nMax;
    size_t nStarted = 0;
    size_t nActive = 0;
    size_t i;

    if (!pTdnf || !pTdnf->pArgs || !pTdnf->pConf || (nItems && !pItems))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if (nItems == 0)
    {
        goto cleanup;
    }

    for (i = 0; i < nItems; i++)
    {
        if (!pItems[i].pRepo ||
            IsNullOrEmptyString(pItems[i].pszLocation) ||
            IsNullOrEmptyString(pItems[i].pszFile) ||
            IsNullOrEmptyString(pItems[i].pszName))
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        progress.nBytesTotal += pItems[i].nSize;
    }

    nMax = pTdnf->pConf->nMaxParallelDownloads > 0 ?
           (size_t)pTdnf->pConf->nMaxParallelDownloads : 1;
    if (nMax > TDNF_CONF_MAX_PARALLEL_DOWNLOADS_LIMIT)
    {
        nMax = TDNF_CONF_MAX_PARALLEL_DOWNLOADS_LIMIT;
    }

    progress.nItems = nItems;
    progress.nShow = !pTdnf->pArgs->nQuiet &&
                     (isatty(STDOUT_FILENO) || pTdnf->pArgs->nVerbose);
    progress.nTty = progress.nShow && isatty(STDOUT_FILENO);

    dwError = TDNFAllocateMemory(nItems, sizeof(TDNF_XFER), (void **)&pXfers);
    BAIL_ON_TDNF_ERROR(dwError);

    pMulti = curl_multi_init();
    if (!pMulti)
    {
        dwError = ERROR_TDNF_CURL_INIT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (;;)
    {
        CURLMsg *pMsg = NULL;
        int nMsgs = 0;
        int nRunning = 0;

        while (nActive < nMax && nStarted < nItems)
        {
            PTDNF_XFER pXfer = &pXfers[nStarted];
            PTDNF_DOWNLOAD_ITEM pItem = &pItems[nStarted];

            nStarted++;
            pXfer->pItem = pItem;
            pXfer->nBaseUrl =
                (pItem->pRepo->ppszBaseUrls && pItem->pRepo->ppszBaseUrls[0] &&
                 strstr(pItem->pszLocation, "://") == NULL) ? 0 : -1;
            dwError = TDNFXferStart(pTdnf, pMulti, pXfer);
            BAIL_ON_TDNF_ERROR(dwError);
            nActive++;
        }
        if (nActive == 0)
        {
            break;
        }

        if (curl_multi_perform(pMulti, &nRunning) != CURLM_OK)
        {
            dwError = ERROR_TDNF_CURL_INIT;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        while ((pMsg = curl_multi_info_read(pMulti, &nMsgs)) != NULL)
        {
            /* pMsg is invalid once its handle is removed, copy what we need */
            CURL *pCurl = pMsg->easy_handle;
            CURLcode result = pMsg->data.result;
            PTDNF_XFER pXfer = NULL;
            int nRestarted = 0;

            if (pMsg->msg != CURLMSG_DONE)
            {
                continue;
            }
            curl_easy_getinfo(pCurl, CURLINFO_PRIVATE, (char **)&pXfer);

            TDNFXferClearLine(&progress);
            dwError = TDNFXferDone(pTdnf, pMulti, pXfer, result, &nRestarted);
            BAIL_ON_TDNF_ERROR(dwError);
            if (!nRestarted)
            {
                TDNFXferShowDone(&progress, pXfer);
                progress.nDone++;
                progress.nBytesDone += (uint64_t)pXfer->nNow;
                TDNFXferFree(pMulti, pXfer);
                nActive--;
            }
            TDNFXferShowSummary(&progress, pXfers, nStarted, 1);
        }

        TDNFXferShowSummary(&progress, pXfers, nStarted, 0);

        if (nActive)
        {
#if LIBCURL_VERSION_NUM >= 0x074200 /* 7.66.0 */
            if (curl_multi_poll(pMulti, NULL, 0, 1000, NULL) != CURLM_OK)
#else
            if (curl_multi_wait(pMulti, NULL, 0, 1000, NULL) != CURLM_OK)
#endif
            {
                dwError = ERROR_TDNF_CURL_INIT;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
    }

cleanup:
    TDNFXferClearLine(&progress);
    if (pXfers)
    {
        for (i = 0; i < nStarted; i++)
        {
            TDNFXferFree(pMulti, &pXfers[i]);
        }
        TDNFFreeMemory(pXfers);
    }
    if (pMulti)
    {
        curl_multi_cleanup(pMulti);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCreatePackageUrl(
    PTDNF_REPO_DATA pRepo,
    const char* pszPackageLocation,
    char **ppszPackageUrl
    )
{
    uint32_t dwError = 0;
    char *pszPackageUrl = NULL;

    if(!pRepo ||
       IsNullOrEmptyString(pszPackageLocation) ||
       !ppszPackageUrl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pRepo->ppszBaseUrls && pRepo->ppszBaseUrls[0] &&
        strstr(pszPackageLocation, "://") == NULL) {
        dwError = TDNFJoinPath(&pszPackageUrl, pRepo->ppszBaseUrls[0], pszPackageLocation, NULL);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        /* pszPackageLocation is already an absolute URL (xml:base was absolute),
           or there is no base URL: use it as-is. */
        dwError = TDNFAllocateString(pszPackageLocation, &pszPackageUrl);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszPackageUrl = pszPackageUrl;

cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszPackageUrl);
    goto cleanup;
}

uint32_t
TDNFDownloadPackage(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    const char* pszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    char *pszPackageFile = NULL;
    char *pszCopyOfPackageLocation = NULL;
    int nSize = 0;

    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszPackageLocation,
                                 &pszCopyOfPackageLocation);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszPackageFile,
                           pszRpmCacheDir,
                           basename(pszCopyOfPackageLocation),
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    /* don't download if file is already there. Older versions may have left
       size 0 files, so check for those too */
    dwError = TDNFGetFileSize(pszPackageFile, &nSize);
    if ((dwError == ERROR_TDNF_FILE_NOT_FOUND) || (nSize == 0))
    {
        dwError = TDNFDownloadFileFromRepo(pTdnf,
                                   pRepo,
                                   pszPackageLocation,
                                   pszPackageFile,
                                   pszPkgName);
    }
    else if(dwError == 0)
    {
        pr_info("%s package already downloaded\n", pszPkgName);
    }
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCopyOfPackageLocation);
    TDNF_SAFE_FREE_MEMORY(pszPackageFile);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFDownloadPackageToCache(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;
    char* pszNormalRpmCacheDir = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszRpmCacheDir,
                           pTdnf->pConf->pszCacheDir,
                           pRepo->pszId,
                           "rpms",
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(pszRpmCacheDir,
                                &pszNormalRpmCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadPackageToTree(pTdnf,
                                        pszPackageLocation,
                                        pszPkgName,
                                        pRepo,
                                        pszNormalRpmCacheDir,
                                        ppszFilePath);
    BAIL_ON_TDNF_ERROR(dwError);
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszNormalRpmCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    return dwError;
error:
    goto cleanup;
}

/*
 * TDNFDownloadPackageToTree()
 *
 * Download a package while preserving the directory path. For example,
 * if pszPackageLocation is "RPMS/x86_64/foo-1.2-3.rpm", the destination will
 * be downloaded under the destination directory in RPMS/x86_64/foo-1.2-3.rpm
 * (so 'RPMS/x86_64/' will be preserved).
*/

uint32_t
TDNFGetPackageTreePath(
    const char* pszPackageLocation,
    const char* pszNormalRootDir,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;
    char* pszNormalPath = NULL;
    char* pszRemotePath = NULL;

    if(IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszNormalRootDir) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPathFromUri(pszPackageLocation, &pszRemotePath);
    if (dwError == ERROR_TDNF_URL_INVALID)
    {
        dwError = TDNFAllocateString(pszPackageLocation, &pszRemotePath);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszFilePath, pszNormalRootDir, pszRemotePath, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(
                  pszFilePath,
                  &pszNormalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    if (strncmp(pszNormalRootDir, pszNormalPath,
                strlen(pszNormalRootDir)))
    {
        dwError = ERROR_TDNF_URL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszFilePath = pszNormalPath;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszRemotePath);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszNormalPath);
    goto cleanup;
}

uint32_t
TDNFGetPackageCachePath(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    PTDNF_REPO_DATA pRepo,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;
    char* pszNormalRpmCacheDir = NULL;

    if(!pTdnf || !pTdnf->pConf || !pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszRpmCacheDir,
                           pTdnf->pConf->pszCacheDir,
                           pRepo->pszId,
                           TDNF_RPM_CACHE_DIR_NAME,
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(pszRpmCacheDir,
                                &pszNormalRpmCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetPackageTreePath(pszPackageLocation,
                                     pszNormalRpmCacheDir,
                                     ppszFilePath);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszNormalRpmCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadPackageToTree(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    char* pszNormalRpmCacheDir,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszNormalPath = NULL;
    char* pszDownloadCacheDir = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo ||
       IsNullOrEmptyString(pszNormalRpmCacheDir) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetPackageTreePath(pszPackageLocation,
                                     pszNormalRpmCacheDir,
                                     &pszNormalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDirName(pszNormalPath, &pszDownloadCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if(access(pszDownloadCacheDir, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
        }
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(access(pszNormalPath, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        dwError = TDNFDownloadPackage(pTdnf, pszPackageLocation, pszPkgName,
            pRepo, pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszFilePath = pszNormalPath;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszDownloadCacheDir);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszNormalPath);
    goto cleanup;

}

/*
 * TDNFDownloadPackageToDirectory()
 *
 * Download a package withou preserving the directory path. For example,
 * if pszPackageLocation is "RPMS/x86_64/foo-1.2-3.rpm", the destination will
 * be downloaded under the destination directory (pszDirectory) as foo-1.2-3.rpm
 * (so RPMS/x86_64/ will be stripped).
*/

/*
 * The path TDNFDownloadPackageToDirectory() downloads to. TDNFDownloadPackage()
 * uses the basename of the location as is, so this must do the same.
 */
uint32_t
TDNFGetPackageDirectoryPath(
    const char* pszPackageLocation,
    const char* pszDirectory,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszCopyOfLocation = NULL;

    if(IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszDirectory) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszPackageLocation, &pszCopyOfLocation);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(ppszFilePath, pszDirectory,
                           basename(pszCopyOfLocation), NULL);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCopyOfLocation);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadPackageToDirectory(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    const char* pszDirectory,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo ||
       IsNullOrEmptyString(pszDirectory) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetPackageDirectoryPath(pszPackageLocation, pszDirectory,
                                          &pszFilePath);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadPackage(pTdnf, pszPackageLocation, pszPkgName,
                                  pRepo, pszDirectory);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszFilePath = pszFilePath;
cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    goto cleanup;
}
