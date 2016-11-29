/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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
    rpmKeyring pKeyring,
    const char* pszUrlKeyFile,
    const char* pszPkgFile
    )
{
    uint32_t dwError = 0;
    char* pszKeyData = NULL;

    if(!pKeyring || IsNullOrEmptyString(pszUrlKeyFile) || !pszPkgFile)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = AddKeyToKeyRing(pszUrlKeyFile, pKeyring); 
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = VerifyRpmSig(pKeyring, pszPkgFile);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    return dwError;

error:
    goto cleanup;
}

uint32_t TDNFParseScheme(
    const char* pszKeyUrl,
    char** ppszScheme)
{
    uint32_t dwError = 0;
    char* pszScheme = NULL;
    const char* pszTmpStr = NULL;
    int nLen = 0;
    int i = 0;
    if(IsNullOrEmptyString(pszKeyUrl) || !ppszScheme)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszTmpStr = strchr(pszKeyUrl, ':');
    if(pszTmpStr == NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    nLen = pszTmpStr - pszKeyUrl;
    dwError = TDNFAllocateMemory(
                  nLen + 1,
                  sizeof(char),
                  (void**)&pszScheme);
    BAIL_ON_TDNF_ERROR(dwError);
    for(i = 0; i < nLen; i ++)
    {
        pszScheme[i] = pszKeyUrl[i];
    }
    *ppszScheme = pszScheme;
cleanup:
    return dwError;

error:
    if(ppszScheme)
    {
        *ppszScheme = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszScheme);
    goto cleanup;
}

uint32_t FileNameFromUri(
    const char* pszKeyUrl,
    char** ppszFile)
{
    uint32_t dwError = 0;
    const char* pszPath = NULL;
    const char* pszFilePrefix = "file://";
    char *pszFile = NULL;

    if(strncmp(pszKeyUrl, pszFilePrefix, strlen(pszFilePrefix)) != 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszPath = pszKeyUrl + strlen(pszFilePrefix);

    if(!pszPath || *pszPath == '\0')
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (strchr (pszPath, '#') != NULL)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(*pszPath != '/')
    {
        //skip hostname in the uri.
        pszPath = strchr (pszPath, '/');
        if(pszPath == NULL)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    dwError = TDNFAllocateString(pszPath, &pszFile);
    BAIL_ON_TDNF_ERROR(dwError);
    *ppszFile = pszFile;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszFile);
    if(ppszFile)
    {
        *ppszFile = NULL;
    }
    goto cleanup;
}

uint32_t
ReadGPGKey(
   const char* pszKeyUrl,
   char** ppszKeyData
   )
{
    uint32_t dwError = 0;
    char* pszKeyData = NULL;
    int nPathIsDir = 0;
    char* pszScheme = NULL;
    char* pszFile = NULL;
    long nBytes = 0;
    size_t nRealByte = 0;
    FILE *pFile = NULL;
    if(IsNullOrEmptyString(pszKeyUrl) || !ppszKeyData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError =  TDNFParseScheme(pszKeyUrl, &pszScheme);
    BAIL_ON_TDNF_ERROR(dwError);
    if(strcasecmp("file", pszScheme))
    {
        dwError = ERROR_TDNF_KEYURL_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = FileNameFromUri(pszKeyUrl, &pszFile);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFIsDir(pszFile, &nPathIsDir);
    if(dwError)
    {
        fprintf(
            stderr,
            "Error: Accessing gpgkey at %s\n",
            pszFile);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if(nPathIsDir)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pFile = fopen(pszFile, "r");
    if(pFile == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    if(pFile != NULL)
    {
        fseek(pFile, 0L, SEEK_END);
        nBytes = ftell(pFile);
        fseek(pFile, 0L, SEEK_SET);
        dwError = TDNFAllocateMemory(
                      nBytes,
                      sizeof(char),
                      (void**)&pszKeyData);
        BAIL_ON_TDNF_ERROR(dwError);

        if(nBytes <= 0)
        {
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        nRealByte = fread(pszKeyData, sizeof(char), nBytes, pFile);
        if(nRealByte != nBytes)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }
    *ppszKeyData = pszKeyData;

cleanup:
    if(pFile)
    {
        fclose(pFile);
    }
    TDNF_SAFE_FREE_MEMORY(pszScheme);
    TDNF_SAFE_FREE_MEMORY(pszFile);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    goto cleanup;
}

uint32_t
AddKeyToKeyRing(
    const char* pszFile,
    rpmKeyring pKeyring
    )
{
    uint32_t dwError = 0;
    pgpArmor nArmor = PGPARMOR_NONE;
    pgpDig pDig = NULL;
    rpmPubkey pPubkey = NULL;
    uint8_t* pPkt = NULL;
    size_t nPktLen = 0;
    char* pszKeyData = NULL;

    if(IsNullOrEmptyString(pszFile) || !pKeyring)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = ReadGPGKey(pszFile, &pszKeyData);
    BAIL_ON_TDNF_ERROR(dwError);

    nArmor = pgpParsePkts(pszKeyData, &pPkt, &nPktLen);
    if(nArmor != PGPARMOR_PUBKEY) 
    {
        dwError = ERROR_TDNF_INVALID_PUBKEY_FILE;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pPubkey = rpmPubkeyNew (pPkt, nPktLen);
    if(!pPubkey)
    {
        dwError = ERROR_TDNF_CREATE_PUBKEY_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pDig = rpmPubkeyDig(pPubkey);
    if(!pDig)
    {
        dwError = ERROR_TDNF_CREATE_PUBKEY_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = rpmKeyringLookup(pKeyring, pDig);
    if(dwError == RPMRC_OK)
    {
        dwError = 0;//key exists
    }
    else
    {
        dwError = rpmKeyringAddKey(pKeyring, pPubkey);
        if(dwError == 1)
        {
            dwError = 0;//Already added. ignore
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszKeyData);
    if(pPubkey)
    {
        rpmPubkeyFree(pPubkey);
    }
    goto cleanup;
}

uint32_t
VerifyRpmSig(
    rpmKeyring pKeyring,
    const char* pszPkgFile
    )
{
    uint32_t dwError = 0;
    FD_t pFD_t = NULL;
    rpmts pTS = NULL;
    rpmtd pTD = NULL;
    Header pPkgHeader = NULL;
    pgpDig pDigest = NULL;


    if(!pKeyring || IsNullOrEmptyString(pszPkgFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pFD_t = Fopen(pszPkgFile, "r.fdio");
    if(!pFD_t)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    pTS = rpmtsCreate();
    if(!pTS)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
        BAIL_ON_TDNF_RPM_ERROR(dwError);
    }
    rpmtsSetVSFlags (pTS, _RPMVSF_NOSIGNATURES);

    pTD = rpmtdNew();
    if(!pTD)
    {
        dwError = ERROR_TDNF_RPMTD_CREATE_FAILED;
        BAIL_ON_TDNF_RPM_ERROR(dwError);
    }

    dwError = rpmReadPackageFile(pTS, pFD_t, pszPkgFile, &pPkgHeader);
    BAIL_ON_TDNF_RPM_ERROR(dwError);

    if(!headerConvert(pPkgHeader, HEADERCONV_RETROFIT_V3))
    {
        dwError = ERROR_TDNF_RPM_HEADER_CONVERT_FAILED; 
        BAIL_ON_TDNF_RPM_ERROR(dwError);
    }

    if(!headerGet(pPkgHeader, RPMTAG_RSAHEADER, pTD, HEADERGET_MINMEM))
    {
        dwError = ERROR_TDNF_RPM_GET_RSAHEADER_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pDigest = pgpNewDig();
    if(pgpPrtPkts(pTD->data, pTD->count, pDigest, 0))
    {
        dwError = ERROR_TDNF_RPM_GPG_PARSE_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(rpmKeyringLookup(pKeyring, pDigest) != RPMRC_OK)
    {
        dwError = ERROR_TDNF_RPM_GPG_NO_MATCH;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pFD_t)
    {
        Fclose(pFD_t);
    }
    if(pDigest)
    {
        pgpFreeDig(pDigest);
    }
    if(pPkgHeader)
    {
        headerFree(pPkgHeader);
    }
    if(pTD)
    {
        rpmtdFree(pTD);
    }
    if(pTS)
    {
        rpmtsFree(pTS);
    }
    return dwError;

error:
    goto cleanup;
}
