/*
 * Copyright (C) 2015-2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : gpgcheck.c
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

uint32_t
TDNFGPGCheck(
    rpmts pTS,
    const char* pszKeyFile,
    const char* pszPkgFile
    )
{
    uint32_t dwError = 0;
    rpmKeyring pKeyring = NULL;
    FD_t fp = NULL;

    if(!pTS || IsNullOrEmptyString(pszKeyFile) || !pszPkgFile)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fp = Fopen(pszPkgFile, "r.ufdio");
    if (fp == NULL) {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    pKeyring = rpmtsGetKeyring(pTS, 0);
    if (pKeyring == NULL) {
        pr_err("failed to get RPM keyring");
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = AddKeyFileToKeyring(pszKeyFile, pKeyring);
    BAIL_ON_TDNF_ERROR(dwError);

    if (rpmVerifySignatures(
        /* unused but must be != NULL, see lib/rpmchecksig.c in rpm */ (QVA_t)1,
        pTS, fp, pszPkgFile) != 0)
    {
        dwError = ERROR_TDNF_RPM_GPG_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(fp)
    {
        Fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
ReadGPGKeyFile(
    const char* pszFile,
    char** ppszKeyData,
    int* pnSize
   )
{
    uint32_t dwError = 0;
    char* pszKeyData = NULL;
    int nPathIsDir = 0;
    char* pszScheme = NULL;

    if(IsNullOrEmptyString(pszFile) || !ppszKeyData || !pnSize)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFIsDir(pszFile, &nPathIsDir);
    if(dwError)
    {
        pr_err("Error: Accessing gpgkey at %s\n",
            pszFile);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if(nPathIsDir)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFFileReadAllText(pszFile, &pszKeyData, pnSize);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszKeyData = pszKeyData;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszScheme);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    goto cleanup;
}

uint32_t
AddKeyFileToKeyring(
    const char* pszFile,
    rpmKeyring pKeyring
    )
{
    uint32_t dwError = 0;

    int subkeysCount, i;
    rpmPubkey *subkeys = NULL;
    rpmPubkey key = NULL;

    if(IsNullOrEmptyString(pszFile) || !pKeyring)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    key = rpmPubkeyRead(pszFile);
    if (key == NULL) {
        pr_err("reading %s failed: %s (%d)", pszFile, strerror(errno), errno);
        dwError = ERROR_TDNF_INVALID_PUBKEY_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if (rpmKeyringAddKey(pKeyring, key) == 0) {
        pr_info("added key %s to keyring");
    }
    subkeys = rpmGetSubkeys(key, &subkeysCount);
    rpmPubkeyFree(key);
    for (i = 0; i < subkeysCount; i++) {
        rpmPubkey subkey = subkeys[i];

        if (rpmKeyringAddKey(pKeyring, subkey) == 0) {
            pr_info("added subkey %d of main key %s to keyring\n", i, pszFile);
        }
        rpmPubkeyFree(subkey);
    }

cleanup:
    if (subkeys)
        free(subkeys);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFImportGPGKeyFile(
    rpmts pTS,
    const char* pszFile
    )
{
    uint32_t dwError = 0;
    uint8_t* pPkt = NULL;
    size_t nPktLen = 0;
    char* pszKeyData = NULL;
    int nKeyDataSize;
    int nKeys = 0;
    int nOffset = 0;

    if(pTS == NULL || IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = ReadGPGKeyFile(pszFile, &pszKeyData, &nKeyDataSize);
    BAIL_ON_TDNF_ERROR(dwError);

    while (nOffset < nKeyDataSize)
    {
        pgpArmor nArmor = pgpParsePkts(pszKeyData + nOffset, &pPkt, &nPktLen);
        if(nArmor == PGPARMOR_PUBKEY)
        {
            dwError = rpmtsImportPubkey(pTS, pPkt, nPktLen);
            BAIL_ON_TDNF_ERROR(dwError);
            nKeys++;
        }
        nOffset += nPktLen;
    }

    if (nKeys == 0) {
        dwError = ERROR_TDNF_INVALID_PUBKEY_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFGPGCheckPackage(
    PTDNFRPMTS pTS,
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char* pszFilePath,
    Header *pRpmHeader
    )
{
    uint32_t dwError = 0;
    Header rpmHeader = NULL;
    int nGPGSigCheck = 0;
    FD_t fp = NULL;
    char** ppszUrlGPGKeys = NULL;
    char* pszLocalGPGKey = NULL;
    int nAnswer = 0;
    int nRemote = 0;
    int i;
    int nMatched = 0;

    dwError = TDNFGetGPGSignatureCheck(pTdnf, pRepo, &nGPGSigCheck, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    fp = Fopen (pszFilePath, "r.ufdio");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = rpmReadPackageFile(
                  pTS->pTS,
                  fp,
                  pszFilePath,
                  &rpmHeader);

    Fclose(fp);
    fp = NULL;

    if (dwError != RPMRC_NOTTRUSTED && dwError != RPMRC_NOKEY)
    {
        BAIL_ON_TDNF_RPM_ERROR(dwError);
    }
    else if(nGPGSigCheck)
    {
        dwError = TDNFGetGPGSignatureCheck(pTdnf, pRepo, &nGPGSigCheck, &ppszUrlGPGKeys);
        BAIL_ON_TDNF_ERROR(dwError);

        for (i = 0; ppszUrlGPGKeys[i]; i++) {
            pr_info("importing key from %s\n", ppszUrlGPGKeys[i]);
            dwError = TDNFYesOrNo(pTdnf->pArgs, "Is this ok [y/N]: ", &nAnswer);
            BAIL_ON_TDNF_ERROR(dwError);

            if(!nAnswer)
            {
                dwError = ERROR_TDNF_OPERATION_ABORTED;
                BAIL_ON_TDNF_ERROR(dwError);
            }

            dwError = TDNFUriIsRemote(ppszUrlGPGKeys[i], &nRemote);
            if (dwError == ERROR_TDNF_URL_INVALID)
            {
                dwError = ERROR_TDNF_KEYURL_INVALID;
            }
            BAIL_ON_TDNF_ERROR(dwError);

            if (nRemote)
            {
                dwError = TDNFFetchRemoteGPGKey(pTdnf, pRepo, ppszUrlGPGKeys[i], &pszLocalGPGKey);
                BAIL_ON_TDNF_ERROR(dwError);
            }
            else
            {
                dwError = TDNFPathFromUri(ppszUrlGPGKeys[i], &pszLocalGPGKey);
                if (dwError == ERROR_TDNF_URL_INVALID)
                {
                    dwError = ERROR_TDNF_KEYURL_INVALID;
                }
                BAIL_ON_TDNF_ERROR(dwError);
            }
            dwError = TDNFImportGPGKeyFile(pTS->pTS, pszLocalGPGKey);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFGPGCheck(pTS->pTS, pszLocalGPGKey, pszFilePath);
            if (dwError == 0)
            {
                nMatched++;
            }
            else if (dwError == ERROR_TDNF_RPM_GPG_NO_MATCH)
            {
                dwError = 0;
            }
            BAIL_ON_TDNF_ERROR(dwError);

            if (nRemote)
            {
                if (unlink(pszLocalGPGKey))
                {
                    dwError = errno;
                    BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
                }
            }
        }

        if (nMatched == 0)
        {
            dwError = ERROR_TDNF_RPM_GPG_NO_MATCH;
            BAIL_ON_TDNF_ERROR(dwError);
        }
        fp = Fopen (pszFilePath, "r.ufdio");
        if(!fp)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }

        dwError = rpmReadPackageFile(
                      pTS->pTS,
                      fp,
                      pszFilePath,
                      &rpmHeader);

        BAIL_ON_TDNF_RPM_ERROR(dwError);

        Fclose(fp);
        fp = NULL;
    }
    else
    {
        dwError = 0;
    }

    /* optional output parameter */
    if (pRpmHeader != NULL)
    {
        *pRpmHeader = rpmHeader;
    }
    else if(rpmHeader)
    {
        headerFree(rpmHeader);
    }

cleanup:
    TDNF_SAFE_FREE_STRINGARRAY(ppszUrlGPGKeys);
    TDNF_SAFE_FREE_MEMORY(pszLocalGPGKey);
    if(fp)
    {
        Fclose(fp);
    }
    return dwError;

error:
    if(rpmHeader)
    {
        headerFree(rpmHeader);
    }
    goto cleanup;
}

uint32_t
TDNFFetchRemoteGPGKey(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char* pszUrlGPGKey,
    char** ppszKeyLocation
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;
    char* pszNormalPath = NULL;
    char* pszFilePathCopy = NULL;
    char* pszTopKeyCacheDir = NULL;
    char* pszRealTopKeyCacheDir = NULL;
    char* pszDownloadCacheDir = NULL;
    char* pszKeyLocation = NULL;

    if(!pTdnf || !pRepo || IsNullOrEmptyString(pszUrlGPGKey))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPathFromUri(pszUrlGPGKey, &pszKeyLocation);
    if (dwError == ERROR_TDNF_URL_INVALID)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetCachePath(pTdnf, pRepo,
                               "keys", NULL,
                               &pszTopKeyCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(pszTopKeyCacheDir,
                                &pszRealTopKeyCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(
                  &pszFilePath,
                  pszRealTopKeyCacheDir,
                  pszKeyLocation,
                  NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(
                  pszFilePath,
                  &pszNormalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    if (strncmp(pszRealTopKeyCacheDir, pszNormalPath, strlen(pszRealTopKeyCacheDir)))
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
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

    dwError = TDNFDownloadFile(pTdnf, pRepo, pszUrlGPGKey, pszFilePath,
                               basename(pszFilePath));
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszKeyLocation = pszNormalPath;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszRealTopKeyCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszTopKeyCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszFilePathCopy);
    TDNF_SAFE_FREE_MEMORY(pszKeyLocation);
    return dwError;

error:
    pr_err("Error processing key: %s\n", pszUrlGPGKey);
    TDNF_SAFE_FREE_MEMORY(pszNormalPath);
    *ppszKeyLocation = NULL;
    goto cleanup;
}

