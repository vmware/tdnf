/*
 * Copyright (C) 2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : metalink.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Nitesh Kumar (kunitesh@vmware.com)
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

/* Returns nonzero if hex_digest is properly formatted; that is each
   letter is in [0-9A-Za-z] and the length of the string equals to the
   result length of digest * 2. */
static
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

static
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

static
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
TDNFCheckRepoMDFileHashFromMetalink(
    const char *pszFile,
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

    if (hashInfo != NULL)
    {
        dwError = TDNFChecksumFromHexDigest(hashInfo->value, digest);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFCheckHash(pszFile, digest, hash_Type);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_TDNF_INVALID_REPO_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

static void
TDNFMetalinkHashFree(
    TDNF_ML_HASH_INFO *ml_hash_info
    )
{
    if (!ml_hash_info)
    {
        return;
    }

    TDNF_SAFE_FREE_MEMORY(ml_hash_info->type);
    TDNF_SAFE_FREE_MEMORY(ml_hash_info->value);
    TDNF_SAFE_FREE_MEMORY(ml_hash_info);
}

static void
TDNFMetalinkUrlFree(
    TDNF_ML_URL_INFO *ml_url_info
    )
{
    if (!ml_url_info)
    {
        return;
    }

    TDNF_SAFE_FREE_MEMORY(ml_url_info->protocol);
    TDNF_SAFE_FREE_MEMORY(ml_url_info->type);
    TDNF_SAFE_FREE_MEMORY(ml_url_info->location);
    TDNF_SAFE_FREE_MEMORY(ml_url_info->url);
    TDNF_SAFE_FREE_MEMORY(ml_url_info);
}

void
TDNFMetalinkFree(
    TDNF_ML_CTX *ml_ctx
    )
{
    if (!ml_ctx)
        return;

    TDNF_SAFE_FREE_MEMORY(ml_ctx->filename);
    TDNFDeleteList(&ml_ctx->hashes, (TDNF_ML_FREE_FUNC)TDNFMetalinkHashFree);
    TDNFDeleteList(&ml_ctx->urls, (TDNF_ML_FREE_FUNC)TDNFMetalinkUrlFree);
    TDNF_SAFE_FREE_MEMORY(ml_ctx);
}

uint32_t
TDNFParseFileTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node,
    const char *filename
    )
{
    uint32_t dwError = 0;
    xmlChar* xmlPropValue = NULL;
    const char *name = NULL;

    if(!ml_ctx || !node || IsNullOrEmptyString(filename))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    xmlPropValue = xmlGetProp(node, ATTR_NAME);
    if (!xmlPropValue)
    {
        pr_err("%s: Missing attribute \"name\" of file element", __func__);
        dwError = ERROR_TDNF_METALINK_PARSER_MISSING_FILE_ATTR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    name = (const char*)xmlPropValue;
    if (strcmp(name, filename))
    {
        pr_err("%s: Invalid filename from metalink file:%s", __func__, name);
        dwError = ERROR_TDNF_METALINK_PARSER_INVALID_FILE_NAME;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(name, &(ml_ctx->filename));
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(xmlPropValue)
    {
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFParseHashTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node
    )
{
    uint32_t dwError = 0;
    xmlChar* xmlPropValue = NULL;
    xmlChar* xmlContValue = NULL;
    const char *type = NULL;
    const char *value = NULL;
    TDNF_ML_HASH_INFO *ml_hash_info = NULL;

    if(!ml_ctx || !node)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Get Hash Properties.
    xmlPropValue = xmlGetProp(node, ATTR_TYPE);
    if (!xmlPropValue)
    {
        dwError = ERROR_TDNF_METALINK_PARSER_MISSING_HASH_ATTR;
        pr_err("XML Parser Error:HASH element doesn't have attribute \"type\"");
        BAIL_ON_TDNF_ERROR(dwError);
    }

    type = (const char*)xmlPropValue;
    dwError = TDNFAllocateMemory(1, sizeof(TDNF_ML_HASH_INFO),
                                 (void**)&ml_hash_info);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(type, &(ml_hash_info->type));
    BAIL_ON_TDNF_ERROR(dwError);

    //Get Hash Content.
    xmlContValue = xmlNodeGetContent(node);
    if(!xmlContValue)
    {
        dwError = ERROR_TDNF_METALINK_PARSER_MISSING_HASH_CONTENT;
        pr_err("XML Parser Error:HASH value is not present in HASH element", value);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    value = (const char*)xmlContValue;
    dwError = TDNFAllocateString(value, &(ml_hash_info->value));
    BAIL_ON_TDNF_ERROR(dwError);

    //Append hash info in ml_ctx hash list.
    dwError = TDNFAppendList(&(ml_ctx->hashes), ml_hash_info);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(xmlPropValue)
    {
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;
    }

    if(xmlContValue)
    {
        xmlFree(xmlContValue);
        xmlContValue = NULL;
    }
    return dwError;

error:
    if(ml_hash_info)
    {
        TDNFMetalinkHashFree(ml_hash_info);
        ml_hash_info = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFParseUrlTag(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node
    )
{
    uint32_t dwError = 0;
    xmlChar* xmlPropValue = NULL;
    xmlChar* xmlContValue = NULL;
    const char *value = NULL;
    int prefValue = 0;
    TDNF_ML_URL_INFO *ml_url_info = NULL;

    if(!ml_ctx || !node)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_ML_URL_INFO),
                                 (void**)&ml_url_info);
    BAIL_ON_TDNF_ERROR(dwError);

    if ((xmlPropValue = xmlGetProp(node, ATTR_PROTOCOL)))
    {
        value = (const char*)xmlPropValue;
        dwError = TDNFAllocateString(value, &(ml_url_info->protocol));
        BAIL_ON_TDNF_ERROR(dwError);
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;
        value = NULL;
    }
    if ((xmlPropValue = xmlGetProp(node, ATTR_TYPE)))
    {
        value = (const char*)xmlPropValue;
        dwError = TDNFAllocateString(value, &(ml_url_info->type));
        BAIL_ON_TDNF_ERROR(dwError);
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;
        value = NULL;
    }
    if ((xmlPropValue = xmlGetProp(node, ATTR_LOCATION)))
    {
        value = (const char*)xmlPropValue;
        dwError = TDNFAllocateString(value, &(ml_url_info->location));
        BAIL_ON_TDNF_ERROR(dwError);
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;
        value = NULL;
    }
    if ((xmlPropValue = xmlGetProp(node, ATTR_PREFERENCE)))
    {
        value = (const char*)xmlPropValue;
        if(sscanf(value, "%d", &prefValue) != 1)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            pr_err("XML Parser Warning: Preference is invalid value: %s\n", value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;

        if (prefValue < 0 || prefValue > 100)
        {
            dwError = ERROR_TDNF_METALINK_PARSER_MISSING_URL_ATTR;
            pr_err("XML Parser Warning: Bad value (\"%s\") of \"preference\""
                   "attribute in url element (should be in range 0-100)", value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else
        {
            ml_url_info->preference = prefValue;
        }
        value = NULL;
    }

    //Get URL Content.
    xmlContValue = xmlNodeGetContent(node);
    if(!xmlContValue)
    {
        dwError = ERROR_TDNF_METALINK_PARSER_MISSING_URL_CONTENT;
        pr_err("URL is no present in URL element", value);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    value = (const char*)xmlContValue;
    dwError = TDNFAllocateString(value, &(ml_url_info->url));
    BAIL_ON_TDNF_ERROR(dwError);

    //Append url info in ml_ctx url list.
    dwError = TDNFAppendList(&(ml_ctx->urls), ml_url_info);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(xmlPropValue)
    {
        xmlFree(xmlPropValue);
        xmlPropValue = NULL;
    }

    if(xmlContValue)
    {
        xmlFree(xmlContValue);
        xmlContValue = NULL;
    }
    return dwError;

error:
    if(ml_url_info)
    {
        TDNFMetalinkUrlFree(ml_url_info);
        ml_url_info = NULL;
    }
    goto cleanup;
}

uint32_t
TDNFXmlParseData(
    TDNF_ML_CTX *ml_ctx,
    xmlNode *node,
    const char *filename
    )
{
    uint32_t dwError = 0;
    xmlChar* xmlContValue = NULL;
    char *size = NULL;

    if(!ml_ctx || !node || IsNullOrEmptyString(filename))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Looping through all the nodes from root and parse all children nodes.
    while(node)
    {
        if(node->type == XML_ELEMENT_NODE)
        {
            if(!strcmp((const char*)node->name, TAG_NAME_FILE))
            {
                dwError = TDNFParseFileTag(ml_ctx, node, filename);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else if(!strcmp((const char*)node->name, TAG_NAME_SIZE))
            {
                //Get File Size.
                xmlContValue = xmlNodeGetContent(node);
                if(!xmlContValue)
                {
                    dwError = ERROR_TDNF_METALINK_PARSER_MISSING_FILE_SIZE;
                    pr_err("XML Parser Error:File size is missing: %s", size);
                    BAIL_ON_TDNF_ERROR(dwError);
                }
                size = (char*)xmlContValue;
                if(sscanf(size, "%ld", &(ml_ctx->size)) != 1)
                {
                    dwError = ERROR_TDNF_INVALID_PARAMETER;
                    pr_err("XML Parser Warning: size is invalid value: %s\n", size);
                    BAIL_ON_TDNF_ERROR(dwError);
                }
            }
            else if(!strcmp((const char*)node->name, TAG_NAME_HASH))
            {
                dwError = TDNFParseHashTag(ml_ctx, node);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else if(!strcmp((const char*)node->name, TAG_NAME_URL))
            {
                dwError = TDNFParseUrlTag(ml_ctx, node);
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        TDNFXmlParseData(ml_ctx, node->children, filename);
        node = node->next;
    }

cleanup:
    if(xmlContValue)
    {
        xmlFree(xmlContValue);
        xmlContValue = NULL;
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFMetalinkParseFile(
    TDNF_ML_CTX *ml_ctx,
    int fd,
    const char *filename
    )
{
    uint32_t dwError = 0;
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    if(!ml_ctx || (fd <= 0) || IsNullOrEmptyString(filename))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Read The File and get the doc object.
    doc = xmlReadFd(fd, NULL, NULL, 0);

    if (doc == NULL)
    {
        pr_err("%s: Error while reading xml from fd:%d \n", __func__, fd);
        dwError = ERROR_TDNF_METALINK_PARSER_INVALID_DOC_OBJECT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //Get the root element from parsed xml tree.
    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL)
    {
        pr_err("%s: Error to fetch root element of xml tree\n", __func__);
        dwError = ERROR_TDNF_METALINK_PARSER_INVALID_ROOT_ELEMENT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    // Parsing
    dwError = TDNFXmlParseData(ml_ctx, root_element, filename);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if(doc != NULL)
    {
        xmlFreeDoc(doc);
        doc = NULL;
    }
    xmlCleanupParser();

    return dwError;
error:
    goto cleanup;
}
