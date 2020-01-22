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

/*
 * plugins are c libraries which are dynamically loaded
 * if noplugins is set, this function returns immediately
 * without further processing
 * files with extension .conf are enumerated from the config
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
_TDNFGetPluginSettings(
    struct plugin_config *pConf
    )
{
    uint32_t dwError = 0;
    int nHasOpt = 0;

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
                  TDNF_DEFAULT_PLUGIN_CONF_PATH, &pConf->pszConfPath);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetOptWithDefault(
                  pConf->pArgs, TDNF_CONF_KEY_PLUGIN_PATH,
                  TDNF_DEFAULT_PLUGIN_PATH, &pConf->pszPath);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pConf->pszConfPath);
    TDNF_SAFE_FREE_MEMORY(pConf->pszPath);
    pConf->pszConfPath = NULL;
    pConf->pszPath = NULL;
    goto cleanup;
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
            dlclose(pPlugin->pHandle);
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

    if (IsNullOrEmptyString(pszLib) || !pPlugin)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* exists */
    if (pPlugin->pHandle)
    {
        goto cleanup;
    }

    /* clear error */
    dlerror();

    pPlugin->pHandle = dlopen(pszLib, RTLD_NOW);
    if(!pPlugin->pHandle)
    {
        fprintf(stderr, "Error loading plugin: %s\n", pszLib);
        dwError = ERROR_TDNF_PLUGIN_LOAD_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    fprintf(stderr, "Error: %d dlerror: %s\n", dwError, dlerror());
    if (pPlugin && pPlugin->pHandle)
    {
        dlclose(pPlugin->pHandle);
        pPlugin->pHandle = NULL;
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
