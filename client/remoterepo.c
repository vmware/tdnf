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

    if (dlTotal <= 0) {
        return 0;
    }

    dPercent = ((double)dlNow / (double)dlTotal) * 100.0;
    if (!isatty(STDOUT_FILENO)) {
        pr_info("%s %3.0f%% %ld\n", (char *)pUserData, dPercent, dlNow);
    } else {
        pr_info("%-35s %10ld %5.0f%%\r", (char *)pUserData, dlNow, dPercent);
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
    if (dwError == ERROR_TDNF_OPT_NOT_FOUND)
    {
        dwError = 0;/* callback not set */
    }
    return dwError;
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
    if (fd == -1)
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
        /*MD5 is not approved in FIPS mode. So, overrriding
          the dwError to show the right error to the user */
        if (FIPS_mode() && !strcasecmp(hash->hash_type, "md5"))
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
    if (fd != -1)
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
    TDNF_METALINK_FILE *ml_file
    )
{

    uint32_t dwError = 0;
    if(IsNullOrEmptyString(pszFile) ||
       !ml_file)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFCheckHash(pszFile, ml_file->digest, ml_file->type);
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

int
TDNFGetResourceType(
    const char *resource_type,
    int *type
    )
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(resource_type) ||
       !type)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (!strcasecmp(resource_type, "sha512") ||
       !strcasecmp(resource_type, "sha-512"))
    {
        *type = TDNF_HASH_SHA512;
    }
    else if (!strcasecmp(resource_type, "sha256") ||
       !strcasecmp(resource_type, "sha-256"))
    {
        *type = TDNF_HASH_SHA256;
    }
    else if (!strcasecmp(resource_type, "sha1") ||
            !strcasecmp(resource_type, "sha-1"))
    {
        *type = TDNF_HASH_SHA1;
    }
    else if (!strcasecmp(resource_type, "md5"))
    {
        *type = TDNF_HASH_MD5;
    }
    else
    {
        //In case metalink file have resource type which we
        //do not support yet, we should not report error.
        //We should instead skip and verify the hash for the
        //supported resource type.
        *type = -1;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFGetFileHashFromMetalink(
    metalink_file_t *fileinfo,
    TDNF_METALINK_FILE **ml_file
    )
{
    TDNF_METALINK_FILE *metalink_file = NULL;
    uint32_t dwError = 0;
    int i = 0;
    int offset = -1;
    int type = 0;
    metalink_checksum_t **metalink_checksum;
    metalink_resource_t **metalink_resource;

    if (!fileinfo || !ml_file)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_METALINK_FILE), (void **)&metalink_file);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(fileinfo->name, &(metalink_file->filename));
    BAIL_ON_TDNF_ERROR(dwError);
    //set type to -1
    metalink_file->type = -1;
    //set digest to NULL
    memset(metalink_file->digest, 0, EVP_MAX_MD_SIZE);
    if(!fileinfo->checksums)
    {
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    metalink_checksum = fileinfo->checksums;
    for(i = 0; metalink_checksum[i] ; ++i)
    {
        dwError = TDNFGetResourceType(metalink_checksum[i]->type,
                                         &type);
        BAIL_ON_TDNF_ERROR(dwError);
        if(metalink_file->type > type)
        {
            continue;
        }
        if(!TDNFCheckHexDigest(metalink_checksum[i]->hash, hash_ops[type].length))
        {
            continue;
        }
        offset = i;
        metalink_file->type = type;
    }

    if(offset != -1)
    {
        dwError = TDNFChecksumFromHexDigest(metalink_checksum[offset]->hash,
                                                metalink_file->digest);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!fileinfo->resources)
    {
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    metalink_resource = fileinfo->resources;

    if(!metalink_resource || (*metalink_resource == NULL))
    {
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if((*metalink_resource)->url == NULL)
    {
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ml_file = metalink_file;

cleanup:
    return dwError;
error:
    if(metalink_file)
    {
        TDNF_SAFE_FREE_MEMORY(metalink_file->filename);
        TDNF_SAFE_FREE_MEMORY(metalink_file);
    }
    goto cleanup;
}

uint32_t
TDNFAllocateResourceURL(
    TDNF_METALINK_URLS **metalink_url,
    char *url
    )
{
    uint32_t dwError = 0;
    TDNF_METALINK_URLS *new_metalink_url = NULL;

    if (IsNullOrEmptyString(url) || !metalink_url)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_METALINK_URLS), (void **)&new_metalink_url);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(url, &(new_metalink_url->url));
    BAIL_ON_TDNF_ERROR(dwError);

    new_metalink_url->next = NULL;

    *metalink_url = new_metalink_url;

cleanup:
    return dwError;
error:
    if (new_metalink_url)
    {
        TDNF_SAFE_FREE_MEMORY(new_metalink_url->url);
        TDNF_SAFE_FREE_MEMORY(new_metalink_url);
    }
    goto cleanup;
}

uint32_t
TDNFParseAndGetURLFromMetalink(
    PTDNF pTdnf,
    const char *pszRepo,
    const char *pszFile,
    TDNF_METALINK_FILE **ml_file
    )
{
    metalink_error_t metalink_error;
    metalink_t* metalink = NULL;
    metalink_file_t **files;
    metalink_parser_context_t *metalink_context = NULL;
    metalink_resource_t** resources;
    char buf[BUFSIZ] = {0};
    int length = 0;
    int fd = -1;
    uint32_t dwError = 0;
    TDNF_METALINK_URLS *urls_head = NULL;
    TDNF_METALINK_URLS *urls_curr = NULL;
    TDNF_METALINK_URLS *urls_prev = NULL;

    if (!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszRepo) ||
       IsNullOrEmptyString(pszFile) ||
       !ml_file)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    metalink_context = metalink_parser_context_new();
    if (metalink_context == NULL)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fd = open(pszFile, O_RDONLY);
    if (fd == -1)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }
    while((length = read(fd, buf, (sizeof(buf)-1))) > 0)
    {
        metalink_error = metalink_parse_update(metalink_context, buf, length);
        memset(buf, 0, BUFSIZ);
        if (metalink_error != 0)
        {
            if (metalink_context)
            {
                metalink_parser_context_delete(metalink_context);
            }
            pr_err("Unable to parse metalink, ERROR: code=%d\n", metalink_error);
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    if (length == -1)
    {
        if (metalink_context)
        {
            metalink_parser_context_delete(metalink_context);
        }
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    metalink_error = metalink_parse_final(metalink_context, NULL, 0, &metalink);
    if ((metalink_error != 0) || (metalink == NULL))
    {
        pr_err("metalink_parse_final failed, ERROR: code=%d\n", metalink_error);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if (metalink->files == NULL)
    {
        pr_err("Metalink does not contain any valid file.\n");
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(files = metalink->files; files && *files; ++files)
    {
        resources = (*files)->resources;
        if (IsNullOrEmptyString(resources))
        {
            pr_err("File %s does not have any resource.\n", (*files)->name);
            dwError = ERROR_TDNF_METALINK_RESOURCE_VALIDATION_FAILED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        while(*resources)
        {
            dwError = TDNFAllocateResourceURL(&urls_curr, (*resources)->url);
            BAIL_ON_TDNF_ERROR(dwError);

            if (!urls_head)
            {
                urls_head = urls_curr;
            }
            else
            {
                urls_prev->next = urls_curr;
            }
            urls_prev = urls_curr;
            ++resources;
        }

        dwError = TDNFGetFileHashFromMetalink((*files), ml_file);
        BAIL_ON_TDNF_ERROR(dwError);
        (*ml_file)->urls = urls_head;
    }
cleanup:
    if (fd != -1)
    {
        close(fd);
    }
    /* delete metalink_t */
    if (metalink)
    {
        metalink_delete(metalink);
    }
    return dwError;
error:
    TDNFFreeMetalinkUrlsList(urls_head);
    if (ml_file && *ml_file)
    {
        TDNF_SAFE_FREE_MEMORY((*ml_file)->filename);
        TDNF_SAFE_FREE_MEMORY(*ml_file);
    }
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
    PTDNF_REPO_DATA_INTERNAL pRepo;
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
                "Error: %ld when downloading %s\n. Please check repo url.\n",
                lStatus,
                pszFileUrl);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = rename(pszFileTmp, pszFile);
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
        dwError = TDNFAllocateStringPrintf(&pszPackageUrl,
                                           "%s/%s",
                                           pszBaseUrl,
                                           pszPackageLocation);
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

    dwError = TDNFAllocateStringPrintf(&pszPackageFile,
                                       "%s/%s",
                                       pszRpmCacheDir,
                                       basename(pszCopyOfPackageLocation));
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

    dwError = TDNFAllocateStringPrintf(
                  &pszRpmCacheDir,
                  "%s/%s/%s",
                  pTdnf->pConf->pszCacheDir,
                  pszRepoName,
                  "rpms");
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
 * if pszPackageLocation is "RPMS/x86_64/foo-1.2-3.rpm, the destination will
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

    dwError = TDNFAllocateStringPrintf(
                  &pszFilePath,
                  "%s/%s",
                  pszNormalRpmCacheDir,
                  pszRemotePath);
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
 * if pszPackageLocation is "RPMS/x86_64/foo-1.2-3.rpm, the destination will
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

    dwError = TDNFAllocateStringPrintf(
                  &pszFilePath,
                  "%s/%s",
                  pszDirectory,
                  pszFileName);
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
