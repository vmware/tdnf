/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : config.c
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

int
TDNFConfGetRpmVerbosity(
    PTDNF pTdnf
    )
{
    rpmlogLvl nLogLevel = RPMLOG_INFO;
    if(pTdnf)
    {
        nLogLevel = pTdnf->pArgs->nRpmVerbosity;
    }
    return nLogLevel;
}

uint32_t
TDNFReadConfig(
    PTDNF pTdnf,
    const char* pszConfFile,
    const char* pszGroup
    )
{
    uint32_t dwError = 0;
    PTDNF_CONF pConf = NULL;
    PCONF_DATA pData = NULL;
    PCONF_SECTION pSection = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszConfFile) ||
       IsNullOrEmptyString(pszGroup))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFReadConfigFile(pszConfFile, 0, &pData);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_CONF),
                  (void**)&pConf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFConfigGetSection(pData, pszGroup, &pSection);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueInt(
                  pSection,
                  TDNF_CONF_KEY_INSTALLONLY_LIMIT,
                  1,
                  &pConf->nInstallOnlyLimit);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueBoolean(
                  pSection,
                  TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE,
                  0,
                  &pConf->nCleanRequirementsOnRemove);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueBoolean(
                  pSection,
                  TDNF_CONF_KEY_GPGCHECK,
                  0,
                  &pConf->nGPGCheck);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueBoolean(
                  pSection,
                  TDNF_CONF_KEY_KEEP_CACHE,
                  0,
                  &pConf->nKeepCache);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValue(
                  pSection,
                  TDNF_CONF_KEY_REPODIR,
                  TDNF_DEFAULT_REPO_LOCATION,
                  &pConf->pszRepoDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValue(
                  pSection,
                  TDNF_CONF_KEY_CACHEDIR,
                  TDNF_DEFAULT_CACHE_LOCATION,
                  &pConf->pszCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValue(
                  pSection,
                  TDNF_CONF_KEY_DISTROVERPKG,
                  TDNF_DEFAULT_DISTROVERPKG,
                  &pConf->pszDistroVerPkg);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFConfigReadProxySettings(
                  pSection,
                  pConf);
    BAIL_ON_TDNF_ERROR(dwError);


    pTdnf->pConf = pConf;

cleanup:
    if(pData)
    {
        TDNFFreeConfigData(pData);
    }
    return dwError;

error:
    if(pTdnf)
    {
        pTdnf->pConf = NULL;
    }
    if(pConf)
    {
        TDNFFreeConfig(pConf);
    }
    goto cleanup;
}

uint32_t
TDNFConfigExpandVars(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_CONF pConf = NULL;

    if(!pTdnf || !pTdnf->pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pConf = pTdnf->pConf;

    //Allow --releasever overrides
    if(!pConf->pszVarReleaseVer &&
       !IsNullOrEmptyString(pTdnf->pArgs->pszReleaseVer))
    {
        dwError = TDNFAllocateString(pTdnf->pArgs->pszReleaseVer,
                      &pConf->pszVarReleaseVer);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pConf->pszVarReleaseVer &&
       !IsNullOrEmptyString(pConf->pszDistroVerPkg))
    {
        dwError = TDNFRawGetPackageVersion(
                      pTdnf->pArgs->pszInstallRoot,
                      pConf->pszDistroVerPkg,
                      &pConf->pszVarReleaseVer);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!pConf->pszVarBaseArch)
    {
        dwError = TDNFGetKernelArch(&pConf->pszVarBaseArch);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFConfigReadProxySettings(
    PCONF_SECTION pSection,
    PTDNF_CONF pConf)
{
    uint32_t dwError = 0;
    char* pszProxyUser = NULL;
    char* pszProxyPass = NULL;

    if(!pSection || !pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //optional proxy server
    dwError = TDNFReadKeyValue(
                  pSection,
                  TDNF_CONF_KEY_PROXY,
                  NULL,
                  &pConf->pszProxy);
    if(dwError == ERROR_TDNF_NO_DATA)
    {
        dwError = 0;
    }
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pConf->pszProxy))
    {
        //optional proxy user
        dwError = TDNFReadKeyValue(
                      pSection,
                      TDNF_CONF_KEY_PROXY_USER,
                      NULL,
                      &pszProxyUser);
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        //optional proxy pass
        dwError = TDNFReadKeyValue(
                      pSection,
                      TDNF_CONF_KEY_PROXY_PASS,
                      NULL,
                      &pszProxyPass);
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        if(!IsNullOrEmptyString(pszProxyUser) &&
           !IsNullOrEmptyString(pszProxyPass))
        {
            dwError = TDNFAllocateStringPrintf(
                          &pConf->pszProxyUserPass,
                          "%s:%s",
                          pszProxyUser,
                          pszProxyPass);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszProxyUser);
    TDNF_SAFE_FREE_MEMORY(pszProxyPass);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFReadKeyValueBoolean(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    )
{
    uint32_t dwError = 0;
    char* pszValue = NULL;
    int nValue = 0;

    if(!pSection || !pszKeyName || !pnValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFReadKeyValue(
                  pSection,
                  pszKeyName,
                  NULL,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pszValue)
    {
        if(!strcmp(pszValue, "1") || !strcasecmp(pszValue, "true"))
        {
            nValue = 1;
        }
    }
    else
    {
        nValue = nDefault;
    }

    *pnValue = nValue;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszValue);
    return dwError;

error:
    if(pnValue)
    {
        *pnValue = 0;
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValueInt(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    int nDefault,
    int* pnValue
    )
{
    uint32_t dwError = 0;
    char* pszValue = NULL;
    int nValue = 0;

    if(!pSection || !pszKeyName || !pnValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = TDNFReadKeyValue(
                  pSection,
                  pszKeyName,
                  NULL,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    if(pszValue)
    {

        nValue = atoi(pszValue);
    }
    else
    {
        nValue = nDefault;
    }

    *pnValue = nValue;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszValue);
    return dwError;

error:
    if(pnValue)
    {
        *pnValue = 0;
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValue(
    PCONF_SECTION pSection,
    const char* pszKeyName,
    const char* pszDefault,
    char** ppszValue
    )
{
    uint32_t dwError = 0;
    char* pszVal = NULL;
    char* pszValue = NULL;
    PKEYVALUE pKeyValues = NULL;
    
    if(!pSection || !pszKeyName || !ppszValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pKeyValues = pSection->pKeyValues;
    for(; pKeyValues; pKeyValues = pKeyValues->pNext)
    {
        if(strcmp(pszKeyName, pKeyValues->pszKey) == 0)
        {
            dwError = TDNFAllocateString(pKeyValues->pszValue, &pszVal);
            BAIL_ON_TDNF_ERROR(dwError);
            break;
        }
    }

    if(pszVal)
    {
        dwError = TDNFAllocateString(
                      pszVal,
                      &pszValue);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if(pszDefault)
    {
        dwError = TDNFAllocateString(
                      pszDefault,
                      &pszValue);
        BAIL_ON_TDNF_ERROR(dwError);

    }

    *ppszValue = pszValue;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszVal);
    return dwError;

error:
    if(ppszValue)
    {
        *ppszValue = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszValue);
    goto cleanup;
}

void
TDNFFreeConfig(
    PTDNF_CONF pConf
    )
{
    if(pConf)
    {
       TDNF_SAFE_FREE_MEMORY(pConf->pszProxy);
       TDNF_SAFE_FREE_MEMORY(pConf->pszProxyUserPass);
       TDNF_SAFE_FREE_MEMORY(pConf->pszRepoDir);
       TDNF_SAFE_FREE_MEMORY(pConf->pszCacheDir);
       TDNF_SAFE_FREE_MEMORY(pConf->pszDistroVerPkg);
       TDNF_SAFE_FREE_MEMORY(pConf->pszVarReleaseVer);
       TDNF_SAFE_FREE_MEMORY(pConf->pszVarBaseArch);
       TDNF_SAFE_FREE_MEMORY(pConf);
    }
}

uint32_t
TDNFConfigReplaceVars(
    PTDNF pTdnf,
    char** ppszString
    )
{
    uint32_t dwError = 0;
    char* pszDst = NULL;
    char* pszReplacedTemp = NULL;
    PTDNF_CONF pConf = NULL;

    if(!pTdnf || !ppszString || IsNullOrEmptyString(*ppszString))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    //fill variable values such as release and basearch
    //if required
    if(strstr(*ppszString, TDNF_VAR_RELEASEVER) ||
       strstr(*ppszString, TDNF_VAR_BASEARCH))
    {
        dwError = TDNFConfigExpandVars(pTdnf);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        goto cleanup;
    }

    pConf = pTdnf->pConf;
    dwError = TDNFReplaceString(
                  *ppszString,
                  TDNF_VAR_RELEASEVER,
                  pConf->pszVarReleaseVer,
                  &pszReplacedTemp);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReplaceString(
                  pszReplacedTemp,
                  TDNF_VAR_BASEARCH,
                  pConf->pszVarBaseArch,
                  &pszDst);
    BAIL_ON_TDNF_ERROR(dwError);

    TDNFFreeMemory(*ppszString);
    *ppszString = pszDst;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszReplacedTemp);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszDst);
    goto cleanup;
}
