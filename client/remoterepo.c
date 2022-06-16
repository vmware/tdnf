/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

typedef struct _hash_op {
    char *hash_type;
    unsigned int length;
} hash_op;

static hash_op hash_ops[TDNF_HASH_SENTINEL] =
    {
       [TDNF_HASH_MD5]    = {"md5", MD5_DIGEST_LENGTH},
       [TDNF_HASH_SHA1]   = {"sha1", SHA_DIGEST_LENGTH},
       [TDNF_HASH_SHA256] = {"sha256", SHA256_DIGEST_LENGTH},
       [TDNF_HASH_SHA512] = {"sha512", SHA512_DIGEST_LENGTH},
    };

typedef struct _hash_type {
    char *hash_name;
    unsigned int hash_value;
}hash_type;

static hash_type hashType[] =
    {
        {"md5", TDNF_HASH_MD5},
        {"sha1", TDNF_HASH_SHA1},
        {"sha-1", TDNF_HASH_SHA1},
        {"sha256", TDNF_HASH_SHA256},
        {"sha-256", TDNF_HASH_SHA256},
        {"sha512", TDNF_HASH_SHA512},
        {"sha-512", TDNF_HASH_SHA512}
    };

static int hashTypeComparator(const void * p1, const void * p2)
{
    return strcmp(*((const char **)p1), *((const char **)p2));
}

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
        pr_info("%s %u% %ld\n", pData->pszData, dPercent, dlNow);
    }
    else
    {
        pr_info("%-35s %10ld %u%\r", pData->pszData, dlNow, dPercent);
    }

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
    static pcb_data pData;

    if(!pCurl || IsNullOrEmptyString(pszData))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    memset(&pData, 0, sizeof(pcb_data));
    strcpy(pData.pszData, pszData);
    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, &pData);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

int
TDNFGetResourceType(
    const char *resource_type,
    int *type
    )
{
    uint32_t dwError = 0;
    static _Bool sorted;
    hash_type *currHash = NULL;

    if (IsNullOrEmptyString(resource_type) ||
       !type)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!sorted)
    {
        qsort(hashType, sizeOfStruct(hashType), sizeof(*hashType), hashTypeComparator);
        sorted = 1;
    }

    currHash = bsearch(&resource_type, hashType, sizeOfStruct(hashType),
                       sizeof(*hashType), hashTypeComparator);

    /* In case metalink file have resource type which we
     * do not support yet, we should not report error.
     * We should instead skip and verify the hash for the
     * supported resource type.
     */
    if(!currHash)
    {
        *type = -1;
    }
    else
    {
        *type = currHash->hash_value;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFGetDigestForFile(
    const char *filename,
    hash_op *hash,
    uint8_t *digest
    )
{
    uint32_t dwError = 0;
    int fd = -1;
    char buf[BUFSIZ] = {0};
    int length = 0;
    EVP_MD_CTX *ctx = NULL;
    const EVP_MD *digest_type = NULL;
    unsigned int digest_length = 0;

    if (IsNullOrEmptyString(filename) || !hash || !digest)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        pr_err("Metalink: validating (%s) FAILED\n", filename);
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }

    digest_type = EVP_get_digestbyname(hash->hash_type);

    if (!digest_type)
    {
        pr_err("Unknown message digest %s\n", hash->hash_type);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    ctx = EVP_MD_CTX_create();
    if (!ctx)
    {
        pr_err("Context Create Failed\n");
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = EVP_DigestInit_ex(ctx, digest_type, NULL);
    if (!dwError)
    {
        pr_err("Digest Init Failed\n");
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        /* MD5 is not approved in FIPS mode. So, overrriding
           the dwError to show the right error to the user */
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
        if (EVP_default_properties_is_fips_enabled(NULL) && !strcasecmp(hash->hash_type, "md5"))
#else
        if (FIPS_mode() && !strcasecmp(hash->hash_type, "md5"))
#endif
        {
            dwError = ERROR_TDNF_FIPS_MODE_FORBIDDEN;
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while ((length = read(fd, buf, (sizeof(buf)-1))) > 0)
    {
        dwError = EVP_DigestUpdate(ctx, buf, length);
        if (!dwError)
        {
            pr_err("Digest Update Failed\n");
            dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        memset(buf, 0, BUFSIZ);
    }

    if (length == -1)
    {
        pr_err("Metalink: validating (%s) FAILED\n", filename);
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = EVP_DigestFinal_ex(ctx, digest, &digest_length);
    if (!dwError)
    {
        pr_err("Digest Final Failed\n");
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = 0;

cleanup:
    if (fd >= 0)
    {
        close(fd);
    }
    if (ctx)
    {
        EVP_MD_CTX_destroy(ctx);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFCheckHash(
    const char *filename,
    unsigned char *digest,
    int type
    )
{

    uint32_t dwError = 0;
    uint8_t digest_from_file[EVP_MAX_MD_SIZE] = {0};
    hash_op *hash = NULL;

    if (IsNullOrEmptyString(filename) ||
       !digest)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (type  < TDNF_HASH_MD5 || type >= TDNF_HASH_SENTINEL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    hash = hash_ops + type;

    dwError = TDNFGetDigestForFile(filename, hash, digest_from_file);
    BAIL_ON_TDNF_ERROR(dwError);

    if (memcmp(digest_from_file, digest, hash->length))
    {
        dwError = ERROR_TDNF_CHECKSUM_VALIDATION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    if (!IsNullOrEmptyString(filename))
    {
        pr_err("Error: Validating metalink (%s) FAILED (digest mismatch)\n", filename);
    }
    goto cleanup;
}


uint32_t
TDNFCheckRepoMDFileHashFromMetalink(
    char *pszFile,
    TDNF_ML_CTX *ml_ctx
    )
{
    uint32_t dwError = 0;
    TDNF_ML_HASH_LIST *hashList = NULL;
    TDNF_ML_HASH_INFO *hashInfo = NULL;
    unsigned char digest[EVP_MAX_MD_SIZE] = {0};
    int hash_Type = -1;
    TDNF_ML_HASH_INFO *currHashInfo = NULL;

    if(IsNullOrEmptyString(pszFile) ||
       !ml_ctx)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(hashList = ml_ctx->hashes; hashList; hashList = hashList->next)
    {
        int currHashType = TDNF_HASH_SENTINEL;
        currHashInfo = hashList->data;

        if(currHashInfo == NULL)
        {
            dwError = ERROR_TDNF_INVALID_REPO_FILE;
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwError = TDNFGetResourceType(currHashInfo->type, &currHashType);
        BAIL_ON_TDNF_ERROR(dwError);

        if ((hash_Type > currHashType)||
           (!TDNFCheckHexDigest(currHashInfo->value, hash_ops[currHashType].length)))
        {
            continue;
        }
        hash_Type = currHashType;
        hashInfo = currHashInfo;
    }

    dwError = TDNFChecksumFromHexDigest(hashInfo->value, digest);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFCheckHash(pszFile, digest, hash_Type);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

/* Returns nonzero if hex_digest is properly formatted; that is each
   letter is in [0-9A-Za-z] and the length of the string equals to the
   result length of digest * 2. */
uint32_t
TDNFCheckHexDigest(
    const char *hex_digest,
    int digest_length
    )
{
    int i = 0;
    if(IsNullOrEmptyString(hex_digest) ||
       (digest_length <= 0))
    {
        return 0;
    }
    for(i = 0; hex_digest[i]; ++i)
    {
        if(!isxdigit(hex_digest[i]))
        {
            return 0;
        }
    }
    return digest_length * 2 == i;
}

uint32_t
TDNFHexToUint(
    const char *hex_digest,
    unsigned char *uintValue
    )
{
    uint32_t dwError = 0;
    char buf[3] = {0};
    unsigned long val = 0;

    if(IsNullOrEmptyString(hex_digest) ||
       !uintValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    buf[0] = hex_digest[0];
    buf[1] = hex_digest[1];

    errno = 0;
    val = strtoul(buf, NULL, 16);
    if(errno)
    {
        pr_err("Error: strtoul call failed\n");
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    *uintValue = (unsigned char)(val&0xff);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFChecksumFromHexDigest(
    const char *hex_digest,
    unsigned char *ppdigest
    )
{
    uint32_t dwError = 0;
    unsigned char *pdigest = NULL;
    size_t i = 0;
    size_t len = 0;
    unsigned char uintValue = 0;

    if(IsNullOrEmptyString(hex_digest) ||
       !ppdigest)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    len = strlen(hex_digest);

    dwError = TDNFAllocateMemory(1, len/2, (void **)&pdigest);
    BAIL_ON_TDNF_ERROR(dwError);

    for(i = 0; i < len; i += 2)
    {
        dwError = TDNFHexToUint(hex_digest + i, &uintValue);
        BAIL_ON_TDNF_ERROR(dwError);

        pdigest[i>>1] = uintValue;
    }
    memcpy( ppdigest, pdigest, len>>1 );

cleanup:
    TDNF_SAFE_FREE_MEMORY(pdigest);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFParseAndGetURLFromMetalink(
    PTDNF pTdnf,
    const char *pszRepo,
    const char *pszFile,
    TDNF_ML_CTX *ml_ctx
    )
{
    int fd = -1;
    uint32_t dwError = 0;

    if (!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszRepo) ||
       IsNullOrEmptyString(pszFile) ||
       !ml_ctx)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fd = open(pszFile, O_RDONLY);
    if (fd < 0)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }

    dwError = TDNFMetalinkParseFile(ml_ctx, fd, TDNF_REPO_METADATA_FILE_NAME);
    if (dwError)
    {
        pr_err("Unable to parse metalink, ERROR: code=%d\n", dwError);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //sort the URL's in List based on preference.
    TDNFSortListOnPreference(&ml_ctx->urls);

cleanup:
    if (fd >= 0)
    {
        close(fd);
    }
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
    char *pszFileTmp = NULL;
    /* lStatus reads CURLINFO_RESPONSE_CODE. Must be long */
    long lStatus = 0;
    PTDNF_REPO_DATA pRepo;
    int i;

    /* TDNFFetchRemoteGPGKey sends pszProgressData as NULL */
    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszFileUrl) ||
       IsNullOrEmptyString(pszFile) ||
       IsNullOrEmptyString(pszRepo))
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

    dwError = TDNFFindRepoById(pTdnf, pszRepo, &pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplyDownloadSettings(pRepo, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplySSLSettings(pRepo, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_URL, pszFileUrl);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if (!pTdnf->pArgs->nQuiet && pszProgressData != NULL)
    {
        //print progress only if tty or verbose is specified.
        if (isatty(STDOUT_FILENO) || pTdnf->pArgs->nVerbose)
        {
            dwError = set_progress_cb(pCurl, pszProgressData);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    dwError = TDNFAllocateStringPrintf(&pszFileTmp,
                                       "%s.tmp",
                                       pszFile);
    BAIL_ON_TDNF_ERROR(dwError);

    for(i = 0; i <= pRepo->nRetries; i++)
    {
        fp = fopen(pszFileTmp, "wb");
        if(!fp)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }

        dwError = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
        BAIL_ON_TDNF_CURL_ERROR(dwError);

        if (i > 0)
        {
            pr_info("retrying %d/%d\n", i, pRepo->nRetries);
        }
        dwError = curl_easy_perform(pCurl);
        if (dwError == CURLE_OK)
        {
            fclose(fp);
            fp = NULL;
            break;
        }
        if (i == pRepo->nRetries || TDNFCurlErrorIsFatal(dwError))
        {
            BAIL_ON_TDNF_CURL_ERROR(dwError);
        }
        fclose(fp);
        fp = NULL;
    }

    dwError = curl_easy_getinfo(pCurl,
                                CURLINFO_RESPONSE_CODE,
                                &lStatus);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if(lStatus >= 400)
    {
        pr_err(
                "Error: %ld when downloading %s\n. Please check repo url "
                "or refresh metadata with 'tdnf makecache'.\n",
                lStatus,
                pszFileUrl);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = rename(pszFileTmp, pszFile);
        BAIL_ON_TDNF_ERROR(dwError);
        dwError = chmod(pszFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszUserPass);
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
    if(!IsNullOrEmptyString(pszFileTmp))
    {
        unlink(pszFileTmp);
    }

    goto cleanup;
}

uint32_t
TDNFCreatePackageUrl(
    PTDNF pTdnf,
    const char* pszRepoName,
    const char* pszPackageLocation,
    char **ppszPackageUrl
    )
{
    uint32_t dwError = 0;
    char* pszBaseUrl = NULL;
    char *pszPackageUrl = NULL;

    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszRepoName) ||
       !ppszPackageUrl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetBaseUrl(pTdnf, pszRepoName, &pszBaseUrl);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pszBaseUrl) {
        dwError = TDNFJoinPath(&pszPackageUrl, pszBaseUrl, pszPackageLocation, NULL);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateString(pszPackageLocation, &pszPackageUrl);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszPackageUrl = pszPackageUrl;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszBaseUrl);
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
    const char* pszRepoName,
    const char* pszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    char *pszPackageUrl = NULL;
    char *pszPackageFile = NULL;
    char *pszCopyOfPackageLocation = NULL;
    int nSize;

    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       IsNullOrEmptyString(pszRepoName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCreatePackageUrl(pTdnf, pszRepoName, pszPackageLocation, &pszPackageUrl);
    BAIL_ON_TDNF_ERROR(dwError);

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
        dwError = TDNFDownloadFile(pTdnf,
                                   pszRepoName,
                                   pszPackageUrl,
                                   pszPackageFile,
                                   pszPkgName);
    }
    else
    {
        pr_info("%s package already downloaded", pszPkgName);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    pr_info("\n");

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszPackageUrl);
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
    const char* pszRepoName,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;
    char* pszNormalRpmCacheDir = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       IsNullOrEmptyString(pszRepoName) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszRpmCacheDir,
                           pTdnf->pConf->pszCacheDir,
                           pszRepoName,
                           "rpms",
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(pszRpmCacheDir,
                                &pszNormalRpmCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadPackageToTree(pTdnf,
                                        pszPackageLocation,
                                        pszPkgName,
                                        pszRepoName,
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
TDNFDownloadPackageToTree(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    char* pszNormalRpmCacheDir,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;
    char* pszNormalPath = NULL;
    char* pszFilePathCopy = NULL;
    char* pszDownloadCacheDir = NULL;
    char* pszRemotePath = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       IsNullOrEmptyString(pszRepoName) ||
       IsNullOrEmptyString(pszNormalRpmCacheDir) ||
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

    dwError = TDNFJoinPath(&pszFilePath, pszNormalRpmCacheDir, pszRemotePath, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(
                  pszFilePath,
                  &pszNormalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    if (strncmp(pszNormalRpmCacheDir, pszNormalPath,
                strlen(pszNormalRpmCacheDir)))
    {
        dwError = ERROR_TDNF_URL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    // dirname() may modify the contents of path, so it may be desirable to
    // pass a copy when calling this function.
    dwError = TDNFAllocateString(pszNormalPath, &pszFilePathCopy);
    BAIL_ON_TDNF_ERROR(dwError);
    pszDownloadCacheDir = dirname(pszFilePathCopy);
    if(!pszDownloadCacheDir)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

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
            pszRepoName, pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszFilePath = pszNormalPath;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszFilePathCopy);
    TDNF_SAFE_FREE_MEMORY(pszRemotePath);
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

uint32_t
TDNFDownloadPackageToDirectory(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    const char* pszRepoName,
    const char* pszDirectory,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;
    char* pszRemotePath = NULL;
    char* pszFileName = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       IsNullOrEmptyString(pszRepoName) ||
       IsNullOrEmptyString(pszDirectory) ||
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

    pszFileName = basename(pszRemotePath);

    TDNFJoinPath(&pszFilePath, pszDirectory, pszFileName, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadPackage(pTdnf, pszPackageLocation, pszPkgName,
                                  pszRepoName, pszDirectory);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszFilePath = pszFilePath;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRemotePath);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    goto cleanup;
}
