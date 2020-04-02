/*
 * Copyright (C) 2020 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <dlfcn.h>

struct plugin_config
{
    PTDNF_CMD_ARGS pArgs;
    char *pszPath;
    char *pszConfPath;
};

static
uint32_t
_TDNFLoadPlugins(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_PLUGIN *ppPlugins
    );

static
uint32_t
_TDNFInitPlugins(
    PTDNF pTdnf,
    PTDNF_PLUGIN pPlugins
    );

/*
 * Plugins are c libraries which are dynamically loaded.
 * if noplugins is set, this function returns immediately
 * without further processing.
 * Files with extension ".conf" are enumerated from the config
 * directory.
 * If a plugin is disabled by command line override,
 * the config is skipped.
 * If a plugin's config file has the enabled flag set to 0,
 * the config is skipped.
 * For all the configs that are parsed without skipping,
 * the corresponding shared library is dynamically loaded
 * as lib<pluginname>.so from the plugin dir where <pluginname>
 * is the file name (without extension) of the corresponding
 * config file.
*/
uint32_t
TDNFLoadPlugins(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_PLUGIN pPlugins = NULL;

    if (!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = _TDNFLoadPlugins(pTdnf->pArgs, &pPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = _TDNFInitPlugins(pTdnf, pPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    pTdnf->pPlugins = pPlugins;

cleanup:
    return dwError;

error:
    if (dwError == ERROR_TDNF_PLUGINS_DISABLED ||
        dwError == ERROR_TDNF_NO_PLUGIN_CONF_DIR)
    {
        dwError = 0;
    }
    goto cleanup;
}

static
uint32_t
_TDNFInitPlugins(
    PTDNF pTdnf,
    PTDNF_PLUGIN pPlugins
    )
{
    uint32_t dwError = 0;
    PTDNF_PLUGIN pPlugin = NULL;
    TDNF_EVENT_CONTEXT stContext = {0};

    if (!pTdnf || !pPlugins)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* plugin event init */
    stContext.nEvent = MAKE_PLUGIN_EVENT(TDNF_PLUGIN_EVENT_TYPE_INIT,
                           TDNF_PLUGIN_EVENT_STATE_CREATE,
                           TDNF_PLUGIN_EVENT_PHASE_START);
    dwError = TDNFAddEventDataPtr(
                  &stContext,
                  TDNF_EVENT_ITEM_TDNF_HANDLE,
                  pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pPlugin = pPlugins; pPlugin; pPlugin = pPlugin->pNext)
    {
        if (!pPlugin->nEnabled)
        {
            continue;
        }

        dwError = pPlugin->stInterface.pFnInitialize(NULL, &pPlugin->pHandle);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = pPlugin->stInterface.pFnEvent(pPlugin->pHandle, &stContext);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = pPlugin->stInterface.pFnEventsNeeded(pPlugin->pHandle,
                                                &pPlugin->RegisterdEvts);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    TDNFFreeEventData(stContext.pData);
    return dwError;

error:
    TDNFShowPluginError(pTdnf, pPlugin, dwError);
    goto cleanup;
}

static
uint32_t
_TDNFGetPluginSettings(
    struct plugin_config *pConf
    )
{
    uint32_t dwError = 0;
    int nHasOpt = 0;
    char *pszConfPath = NULL;
    char *pszPath = NULL;

    if(!pConf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFHasOpt(pConf->pArgs, TDNF_CONF_KEY_NO_PLUGINS, &nHasOpt);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nHasOpt)
    {
        dwError = ERROR_TDNF_PLUGINS_DISABLED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetOptWithDefault(
                  pConf->pArgs, TDNF_CONF_KEY_PLUGIN_CONF_PATH,
                  TDNF_DEFAULT_PLUGIN_CONF_PATH, &pszConfPath);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetOptWithDefault(
                  pConf->pArgs, TDNF_CONF_KEY_PLUGIN_PATH,
                  TDNF_DEFAULT_PLUGIN_PATH, &pszPath);
    BAIL_ON_TDNF_ERROR(dwError);

    TDNF_SAFE_FREE_MEMORY(pConf->pszConfPath);
    TDNF_SAFE_FREE_MEMORY(pConf->pszPath);

    pConf->pszConfPath = pszConfPath;
    pConf->pszPath = pszPath;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszConfPath);
    TDNF_SAFE_FREE_MEMORY(pszPath);
    goto cleanup;
}

static
void
_TDNFClosePlugin(
    PTDNF_PLUGIN pPlugin
    )
{
    if (pPlugin->stInterface.pFnCloseHandle)
    {
        pPlugin->stInterface.pFnCloseHandle(pPlugin->pHandle);
    }
    if (pPlugin->pModule)
    {
        dlclose(pPlugin->pModule);
    }
}

static
void
_TDNFFreePlugin(
    PTDNF_PLUGIN pPlugin
    )
{
    if (pPlugin)
    {
        if (pPlugin->pHandle)
        {
            _TDNFClosePlugin(pPlugin);
        }
        TDNF_SAFE_FREE_MEMORY(pPlugin->pszName);
        TDNFFreeMemory(pPlugin);
    }
}

/* read config file */
static
uint32_t
_TDNFLoadPluginConfig(
    const char *pszConfigFile,
    PTDNF_PLUGIN *ppPlugin
    )
{
    uint32_t dwError = 0;
    PTDNF_PLUGIN pPlugin = NULL;
    PCONF_DATA pData = NULL;
    PCONF_SECTION pSection = NULL;

    if(IsNullOrEmptyString(pszConfigFile) || !ppPlugin)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFReadConfigFile(pszConfigFile, 0, &pData);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(1, sizeof(*pPlugin), (void **)&pPlugin);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pSection = pData->pSections; pSection; pSection = pSection->pNext)
    {
        /* look for main section only */
        if (strcmp(pSection->pszName, TDNF_PLUGIN_CONF_MAIN_SECTION) != 0)
        {
            continue;
        }
        dwError = TDNFReadKeyValueBoolean(
                      pSection,
                      TDNF_PLUGIN_CONF_KEY_ENABLED,
                      0,
                      &pPlugin->nEnabled);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppPlugin = pPlugin;

cleanup:
    if(pData)
    {
        TDNFFreeConfigData(pData);
    }
    return dwError;

error:
    _TDNFFreePlugin(pPlugin);
    goto cleanup;
}

/*
 * enumerate the config directory and load all configs
*/
static
uint32_t
_TDNFLoadPluginConfigs(
    struct plugin_config *pConf,
    PTDNF_PLUGIN *ppPlugins
    )
{
    uint32_t dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    int nLen = 0;
    int nExtLen = TDNF_PLUGIN_CONF_EXT_LEN;
    PTDNF_PLUGIN pPlugin = NULL;
    PTDNF_PLUGIN pPlugins = NULL;
    PTDNF_PLUGIN pLast = NULL;
    char *pszPluginConfig = NULL;

    if(!pConf || !ppPlugins)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pDir = opendir(pConf->pszConfPath);
    if(pDir == NULL)
    {
        dwError = ERROR_TDNF_NO_PLUGIN_CONF_DIR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while((pEnt = readdir(pDir)) != NULL)
    {
        nLen = strlen(pEnt->d_name);
        if (nLen <= nExtLen ||
            strcmp(pEnt->d_name + nLen - nExtLen, TDNF_PLUGIN_CONF_EXT))
        {
            continue;
        }

        dwError = TDNFAllocateStringPrintf(
                      &pszPluginConfig,
                      "%s/%s",
                      pConf->pszConfPath,
                      pEnt->d_name);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = _TDNFLoadPluginConfig(pszPluginConfig, &pPlugin);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateStringN(pEnt->d_name, nLen - nExtLen, &pPlugin->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszPluginConfig);
        pszPluginConfig = NULL;

        if(!pPlugins)
        {
            pPlugins = pLast = pPlugin;
        }
        else
        {
            pLast->pNext = pPlugin;
            pLast = pPlugin;
        }
    }

    *ppPlugins = pPlugins;

cleanup:
    if(pDir)
    {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszPluginConfig);
    return dwError;

error:
    TDNFFreePlugins(pPlugins);
    goto cleanup;
}

static
uint32_t
_TDNFAlterPluginState(
    PTDNF_PLUGIN pPlugins,
    int nEnable,
    const char* pszName
    )
{
    uint32_t dwError = 0;
    int nMatch = 0;
    int nIsGlob = 0;
    PTDNF_PLUGIN pPlugin = NULL;

    if(!pPlugins || IsNullOrEmptyString(pszName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nIsGlob = TDNFIsGlob(pszName);

    for(pPlugin = pPlugins; pPlugin; pPlugin = pPlugin->pNext)
    {
        nMatch = 0;
        if(nIsGlob)
        {
            if(!fnmatch(pszName, pPlugin->pszName, 0))
            {
                nMatch = 1;
            }
        }
        else if(!strcmp(pPlugin->pszName, pszName))
        {
            nMatch = 1;
        }
        if(nMatch)
        {
            pPlugin->nEnabled = nEnable;
            if(!nIsGlob)
            {
                break;
            }
        }
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * apply command line overrides such as
 * --enableplugin=<filter/glob>
 * --disableplugin=<filter/glob>
 * and combinations thereof. For eg.
 * --disableplugin=* --enableplugin=plugin1
*/
static
uint32_t
_TDNFApplyPluginOverrides(
    struct plugin_config *pConf,
    PTDNF_PLUGIN pPlugins
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;

    if (!pConf || !pPlugins)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* apply command line overrides to enable/disable specific plugins */
    pSetOpt = pConf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_ENABLEPLUGIN ||
           pSetOpt->nType == CMDOPT_DISABLEPLUGIN)
        {
            dwError = _TDNFAlterPluginState(
                          pPlugins,
                          pSetOpt->nType == CMDOPT_ENABLEPLUGIN,
                          pSetOpt->pszOptValue);
            BAIL_ON_TDNF_ERROR(dwError);
         }
         pSetOpt = pSetOpt->pNext;
    }
error:
    return dwError;
}

static
uint32_t
_TDNFLoadPluginLib(
    const char *pszLib,
    PTDNF_PLUGIN pPlugin
    )
{
    uint32_t dwError = 0;
    PFN_TDNF_PLUGIN_LOAD_INTERFACE pFnLoadInterface = NULL;

    if (IsNullOrEmptyString(pszLib) || !pPlugin)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* exists */
    if (pPlugin->pModule)
    {
        goto cleanup;
    }

    /* clear error */
    dlerror();

    pPlugin->pModule = dlopen(pszLib, RTLD_NOW);
    if(!pPlugin->pModule)
    {
        fprintf(stderr, "Error loading plugin: %s\n", pszLib);
        dwError = ERROR_TDNF_PLUGIN_LOAD_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pFnLoadInterface = dlsym(pPlugin->pModule,
                             TDNF_FN_NAME_PLUGIN_LOAD_INTERFACE);
    if (!pFnLoadInterface)
    {
        dwError = ERROR_TDNF_PLUGIN_LOAD_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = pFnLoadInterface(&pPlugin->stInterface);
    BAIL_ON_TDNF_ERROR(dwError);

    /*
     * final validation for all the function pointers.
     * if any fail to populate, plugin will be disabled.
    */
    if (!pPlugin->stInterface.pFnInitialize ||
        !pPlugin->stInterface.pFnEventsNeeded ||
        !pPlugin->stInterface.pFnGetErrorString ||
        !pPlugin->stInterface.pFnEvent ||
        !pPlugin->stInterface.pFnCloseHandle)
    {
        dwError = ERROR_TDNF_PLUGIN_LOAD_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    printf("Loaded plugin: %s\n", pPlugin->pszName);

cleanup:
    return dwError;

error:
    fprintf(stderr, "Error: %u dlerror: %s\n", dwError, dlerror());
    if (pPlugin)
    {
        if (pPlugin->pModule)
        {
            dlclose(pPlugin->pModule);
            pPlugin->pModule = NULL;
        }
        pPlugin->nEnabled = 0;
    }
    dwError = 0; /* okay to proceed without any or all plugins */
    goto cleanup;
}

static
uint32_t
_TDNFLoadPluginLibs(
    const char *pszLibPath,
    PTDNF_PLUGIN pPlugins
    )
{
    uint32_t dwError = 0;
    char *pszPlugin = NULL;
    PTDNF_PLUGIN pPlugin = NULL;

    if (IsNullOrEmptyString(pszLibPath) || !pPlugins)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pPlugin = pPlugins; pPlugin; pPlugin = pPlugin->pNext)
    {
        if (pPlugin->nEnabled == 0)
        {
            continue;
        }
        dwError = TDNFAllocateStringPrintf(
                      &pszPlugin,
                      "%s/%s/lib%s.so",
                      pszLibPath,
                      pPlugin->pszName,
                      pPlugin->pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = _TDNFLoadPluginLib(pszPlugin, pPlugin);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszPlugin);
        pszPlugin = NULL;
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszPlugin);
    return dwError;

error:
    goto cleanup;
}

/*
 * load all enabled plugins
*/
static
uint32_t
_TDNFLoadPlugins(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_PLUGIN *ppPlugins
    )
{
    uint32_t dwError = 0;
    PTDNF_PLUGIN pPlugins = NULL;
    struct plugin_config stConf = {0};

    if(!pArgs || !ppPlugins)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    stConf.pArgs = pArgs;
    dwError = _TDNFGetPluginSettings(&stConf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = _TDNFLoadPluginConfigs(&stConf, &pPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = _TDNFApplyPluginOverrides(&stConf, pPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = _TDNFLoadPluginLibs(stConf.pszPath, pPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppPlugins = pPlugins;
cleanup:
    TDNF_SAFE_FREE_MEMORY(stConf.pszPath);
    TDNF_SAFE_FREE_MEMORY(stConf.pszConfPath);
    return dwError;

error:
    TDNFFreePlugins(pPlugins);
    goto cleanup;
}

void
TDNFFreePlugins(
    PTDNF_PLUGIN pPlugins
    )
{
    PTDNF_PLUGIN pPlugin = NULL;
    while(pPlugins)
    {
        pPlugin = pPlugins->pNext;
        _TDNFFreePlugin(pPlugins);
        pPlugins = pPlugin;
    }
}

uint32_t
TDNFPluginRaiseEvent(
    PTDNF pTdnf,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    PTDNF_PLUGIN pPlugin = NULL;

    if (!pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pPlugin = pTdnf->pPlugins; pPlugin; pPlugin = pPlugin->pNext)
    {
        if (!pPlugin->nEnabled ||
            !(pPlugin->RegisterdEvts & PLUGIN_EVENT_TYPE(pContext->nEvent)))
        {
            continue;
        }

        dwError = pPlugin->stInterface.pFnEvent(pPlugin->pHandle, pContext);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    TDNFShowPluginError(pTdnf, pPlugin, dwError);
    goto cleanup;
}

void
TDNFShowPluginError(
    PTDNF pTdnf,
    PTDNF_PLUGIN pPlugin,
    uint32_t nErrorCode
    )
{
    char *pszError = NULL;

    if (!pTdnf || !pPlugin || !nErrorCode)
    {
        goto cleanup;
    }

    if (!TDNFGetPluginErrorString(pTdnf, pPlugin, nErrorCode, &pszError))
    {
        fprintf(stderr, "Plugin error: %s\n", pszError);
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszError);
    return;
}

uint32_t
TDNFGetPluginErrorString(
    PTDNF pTdnf,
    PTDNF_PLUGIN pPlugin,
    uint32_t nErrorCode,
    char **ppszError
    )
{
    char *pszError = 0;
    uint32_t dwError = 0;

    if (!pTdnf || !pPlugin || !nErrorCode || !ppszError)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = pPlugin->stInterface.pFnGetErrorString(
                    pPlugin->pHandle,
                    nErrorCode,
                    &pszError);
    if (dwError == ERROR_TDNF_NO_PLUGIN_ERROR)
    {
        dwError = 0;
    }

    BAIL_ON_TDNF_ERROR(dwError);
    if (IsNullOrEmptyString(pszError))
    {
        dwError = ERROR_TDNF_NO_PLUGIN_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszError = pszError;

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszError);
    goto cleanup;
}
