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
    char* pszFile,
    char* pszGroup,
    PTDNF_CONF* ppConf
    )
{
    uint32_t dwError = 0;

    GKeyFile* pKeyFile = NULL;
    char* pszValue = NULL;

    PTDNF_CONF pConf = NULL;

    pKeyFile = g_key_file_new(); 
    if(!pKeyFile)
    {
        dwError = ERROR_TDNF_OUT_OF_MEMORY;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!g_key_file_load_from_file(
                pKeyFile,
                pszFile,
                G_KEY_FILE_KEEP_COMMENTS,
                NULL))
    {
        dwError = ERROR_TDNF_CONF_FILE_LOAD;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    if(g_key_file_has_group(pKeyFile, pszGroup))
    {
        dwError = TDNFAllocateMemory(
                    sizeof(TDNF_CONF),
                    (void**)&pConf);
        BAIL_ON_TDNF_ERROR(dwError);

        if(g_key_file_has_key(pKeyFile, pszGroup, TDNF_CONF_KEY_GPGCHECK, NULL))
        {
            if(g_key_file_get_boolean(
                   pKeyFile,
                   pszGroup,
                   TDNF_CONF_KEY_GPGCHECK,
                   NULL))
            {
                pConf->nGPGCheck=1;
            }
        }
        if(g_key_file_has_key(
                              pKeyFile,
                              pszGroup,
                              TDNF_CONF_KEY_INSTALLONLY_LIMIT,
                              NULL))
        {
            pConf->nInstallOnlyLimit = g_key_file_get_integer(
                                            pKeyFile,
                                            pszGroup,
                                            TDNF_CONF_KEY_INSTALLONLY_LIMIT,
                                            NULL);
        }
        if(g_key_file_has_key(
                pKeyFile,
                pszGroup,
                TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE,
                NULL))
        {
            pConf->nCleanRequirementsOnRemove = 
                     g_key_file_get_boolean(
                          pKeyFile,
                          pszGroup,
                          TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE,
                          NULL);
        }
        dwError = TDNFReadKeyValue(
                      pKeyFile,
                      pszGroup,
                      TDNF_CONF_KEY_REPODIR,
                      TDNF_DEFAULT_REPO_LOCATION,
                      &pConf->pszRepoDir);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadKeyValue(
                      pKeyFile,
                      pszGroup,
                      TDNF_CONF_KEY_CACHEDIR,
                      TDNF_DEFAULT_CACHE_LOCATION,
                      &pConf->pszCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppConf = pConf;

cleanup:
    if(pKeyFile)
    {
        g_key_file_free(pKeyFile);
    }
    if(pszValue)
    {
        g_free(pszValue);
    }
    return dwError;

error:
    if(ppConf)
    {
        *ppConf = NULL;
    }
    if(pConf)
    {
        TDNFFreeConfig(pConf);
    }
    goto cleanup;
}

uint32_t
TDNFReadKeyValue(
    GKeyFile* pKeyFile,
    char* pszGroupName,
    char* pszKeyName,
    char* pszDefault,
    char** ppszValue
    )
{
    uint32_t dwError = 0;
    char* pszVal = NULL;
    char* pszValue = NULL;
    
    if(!pKeyFile || !pszGroupName || !pszKeyName || !ppszValue)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(!g_key_file_has_key(pKeyFile, pszGroupName, pszKeyName, NULL))
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pszVal = g_key_file_get_string(
                    pKeyFile,
                    pszGroupName,
                    pszKeyName,
                    NULL);
    if(!pszVal && pszDefault)
    {
        pszVal = g_strdup(pszDefault);
        if(!pszVal)
        {
            dwError = ERROR_TDNF_OUT_OF_MEMORY; 
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    if(!pszVal)
    {
        dwError = ERROR_TDNF_NO_DATA;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(
                  pszVal,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:
    g_free(pszVal);
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
       TDNF_SAFE_FREE_MEMORY(pConf->pszRepoDir);
       TDNF_SAFE_FREE_MEMORY(pConf->pszCacheDir);
       TDNF_SAFE_FREE_MEMORY(pConf);
    }
}
