/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Module   : gpgcheck.c
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            client library
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
*/
#include "includes.h"

static
uint32_t
ReadAllBytes(
    const char* pszFileName,
    char** pszFileContents
    );

static
uint32_t
ReadGPGKey(
   const char* pszFile,
   char** ppszKeyData
   );

static
uint32_t
AddKeyToKeyRing(
    const char* pszFile,
    rpmKeyring pKeyring
    );

static
uint32_t
VerifyRpmSig(
    rpmKeyring pKeyring,
    const char* pszPkgFile
    );

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

uint32_t
ReadAllBytes(
    const char* pszFile,
    char** ppszData
    )
{
    uint32_t dwError = 0;
    size_t fsize = 0;
    char* pszData = NULL;
    FILE* fp = NULL;

    if(IsNullOrEmptyString(pszFile) || !ppszData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fp = fopen(pszFile, "r");
    if(!fp)
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    if(fseek(fp, 0, SEEK_END))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fsize = ftell(fp);
    if(fseek(fp, 0, SEEK_SET))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(fsize+1, (void**)&pszData);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!fread(pszData, fsize, 1, fp))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    *ppszData = pszData;
cleanup:
    if(fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    if(!ppszData)
    {
        *ppszData = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszData);
    goto cleanup;
}

uint32_t
ReadGPGKey(
   const char* pszKeyUrl,
   char** ppszKeyData
   )
{
    uint32_t dwError = 0;
    gchar* pszScheme = NULL;
    gchar* pszFile = NULL;
    char* pszKeyData = NULL;
    int nPathIsDir = 0;

    if(IsNullOrEmptyString(pszKeyUrl) || !ppszKeyData)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    
    pszScheme = g_uri_parse_scheme(pszKeyUrl);
    if(!pszScheme)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(strcasecmp("file", pszScheme))
    {
        dwError = ERROR_TDNF_KEYURL_UNSUPPORTED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszFile = g_filename_from_uri(pszKeyUrl, NULL, NULL);
    if(!pszFile)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFIsDir(pszFile, &nPathIsDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if(nPathIsDir)
    {
        dwError = ERROR_TDNF_KEYURL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = ReadAllBytes(pszFile, &pszKeyData);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszKeyData = pszKeyData;

cleanup:
    if(pszScheme)
    {
        g_free(pszScheme);
    }
    if(pszFile)
    {
        g_free(pszFile);
    }
    return dwError;

error:
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

    pTD = rpmtdNew();
    if(!pTD)
    {
        dwError = ERROR_TDNF_RPMTS_CREATE_FAILED;
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
        dwError = ERROR_TDNF_RPM_NOT_SIGNED; 
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pFD_t)
    {
        Fclose(pFD_t);
    }
    if(pTS)
    {
        rpmtsFree(pTS);
    }
    return dwError;

error:
    goto cleanup;
}
