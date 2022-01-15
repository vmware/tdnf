/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static
uint32_t
_TDNFConfigReadPluginSettings(
    PCONF_SECTION pSection,
    PTDNF pTdnf
    );

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
    char *pszConfFileCopy = NULL;
    char *pszMinVersionsDir = NULL;
    char *pszConfFileCopy2 = NULL;
    char *pszPkgLocksDir = NULL;

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

    dwError = TDNFReadKeyValueStringArray(
                  pSection,
                  TDNF_CONF_KEY_EXCLUDE,
                  &pConf->ppszExcludes);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueStringArray(
                  pSection,
                  TDNF_CONF_KEY_MINVERSIONS,
                  &pConf->ppszMinVersions);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFConfigReadProxySettings(
                  pSection,
                  pConf);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = _TDNFConfigReadPluginSettings(
                  pSection,
                  pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    /* We need a copy of pszConfFile because dirname() modifies its argument */
    dwError = TDNFAllocateString(pszConfFile, &pszConfFileCopy);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszMinVersionsDir, dirname(pszConfFileCopy), "minversions.d", NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfFilesFromDir(pszMinVersionsDir, &pConf->ppszMinVersions);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszConfFile, &pszConfFileCopy2);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszPkgLocksDir, dirname(pszConfFileCopy2), "locks.d", NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfFilesFromDir(pszPkgLocksDir, &pConf->ppszPkgLocks);
    BAIL_ON_TDNF_ERROR(dwError);

    pTdnf->pConf = pConf;

cleanup:
    if(pData)
    {
        TDNFFreeConfigData(pData);
    }
    TDNF_SAFE_FREE_MEMORY(pszConfFileCopy);
    TDNF_SAFE_FREE_MEMORY(pszConfFileCopy2);
    TDNF_SAFE_FREE_MEMORY(pszMinVersionsDir);
    TDNF_SAFE_FREE_MEMORY(pszPkgLocksDir);
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
        TDNF_SAFE_FREE_MEMORY(pConf->pszBaseArch);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszExcludes);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszMinVersions);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszPkgLocks);
        TDNFFreeMemory(pConf);
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

    dwError = TDNFConfigExpandVars(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    /* fill variable values such as release and basearch
       if required */
    if(strstr(*ppszString, TDNF_VAR_RELEASEVER) ||
       strstr(*ppszString, TDNF_VAR_BASEARCH))
    {
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
    }
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszReplacedTemp);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszDst);
    goto cleanup;
}

/*
 * Read the following settings from tdnf.conf
 * plugins - 0/1. 0 = no plugins. default is 0
 * pluginpath - path to look for plugin libraries. default /usr/lib/tdnf-plugins
 * pluginconfpath - path to look for plugin config files. default /etc/tdnf/pluginconf.d
*/
static
uint32_t
_TDNFConfigReadPluginSettings(
    PCONF_SECTION pSection,
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    char *pszValue = NULL;
    int nPlugins = 0;

    if(!pSection || !pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* if there is a command line override to deactivate plugins, exit early */
    dwError = TDNFHasOpt(pTdnf->pArgs, TDNF_CONF_KEY_NO_PLUGINS, &nPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    if (nPlugins)
    {
        goto cleanup;
    }

    /* plugins option to enable or deactivate plugins. default 0 */
    dwError = TDNFReadKeyValueInt(
                  pSection,
                  TDNF_CONF_KEY_PLUGINS,
                  TDNF_DEFAULT_PLUGINS_ENABLED,
                  &nPlugins);
    BAIL_ON_TDNF_ERROR(dwError);

    /*
     * config file having a plugins=0 setting is the same as
     * --noplugins from cmd line
    */
    if (nPlugins == 0)
    {
        dwError = TDNFSetOpt(
                      pTdnf->pArgs,
                      TDNF_CONF_KEY_NO_PLUGINS, "1");
        BAIL_ON_TDNF_ERROR(dwError);

        /* no further reads required */
        goto cleanup;
    }

    /* plugin conf path - default to /etc/tdnf/pluginconf.d */
    dwError = TDNFReadKeyValue(
                  pSection,
                  TDNF_CONF_KEY_PLUGIN_CONF_PATH,
                  TDNF_DEFAULT_PLUGIN_CONF_PATH,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSetOpt(pTdnf->pArgs, TDNF_CONF_KEY_PLUGIN_CONF_PATH, pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    TDNFFreeMemory(pszValue);
    pszValue = NULL;

    /* plugin path - default to /usr/lib/tdnf-plugins */
    dwError = TDNFReadKeyValue(
                  pSection,
                  TDNF_CONF_KEY_PLUGIN_PATH,
                  TDNF_DEFAULT_PLUGIN_PATH,
                  &pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSetOpt(pTdnf->pArgs, TDNF_CONF_KEY_PLUGIN_PATH, pszValue);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszValue);
    return dwError;

error:
    goto cleanup;
}

/*
 * Read all minimal versions files from pszDir, and store results into
 * string array pointed to by pppszLines. pppszLines may already
 * have values set from the config file, which are preserved.
 */
uint32_t
TDNFReadConfFilesFromDir(
    char *pszDir,
    char ***pppszLines
    )
{
    uint32_t dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    char *pszFile = NULL;
    char **ppszNewLines = NULL;
    char ***pppszArrayList = NULL;
    int nFileCount = 0;
    int i, j, k;
    int nLineCount = 0;
    int nTmp = 0;

    if(IsNullOrEmptyString(pszDir) || !pppszLines)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* first, open directory and count all files match *.conf,
     * so we know how much memory we need */
    pDir = opendir(pszDir);
    if (pDir == NULL)
    {
        goto cleanup;
    }

    while((pEnt = readdir(pDir)) != NULL)
    {
        if (fnmatch("*.conf", pEnt->d_name, 0) != 0)
        {
            continue;
        }
        nFileCount++;
    }
    closedir(pDir);
    pDir = NULL;

    /* allocate memory for our string array */
    dwError = TDNFAllocateMemory(nFileCount + 1, sizeof(char **), (void **)&pppszArrayList);
    BAIL_ON_TDNF_ERROR(dwError);

    /* read directory again and the files, store content of each file
     * temporarily in pppszArrayList[i] */
    i = 0;
    pDir = opendir(pszDir);
    while((pEnt = readdir(pDir)) != NULL && i < nFileCount)
    {
        if (fnmatch("*.conf", pEnt->d_name, 0) != 0)
        {
            continue;
        }
        dwError = TDNFAllocateStringPrintf(&pszFile, pszDir, pEnt->d_name);
        dwError = TDNFJoinPath(&pszFile, pszDir, pEnt->d_name, NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFReadFileToStringArray(pszFile, &pppszArrayList[i]);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pszFile);

        i++;
    }
    closedir(pDir);
    pDir = NULL;

    /* append values that are already set */
    pppszArrayList[i] = *pppszLines;

    /* each file can have multiple lines, count them */
    for (i = 0; pppszArrayList[i]; i++)
    {
        dwError = TDNFStringArrayCount(pppszArrayList[i], &nTmp);
        BAIL_ON_TDNF_ERROR(dwError);

        nLineCount += nTmp;
    }

    /* move the lines from 2 dimensional pppszArrayList to
     * flat pointer list ppszLines */
    dwError = TDNFAllocateMemory(nLineCount+1, sizeof(char *), (void **)&ppszNewLines);
    BAIL_ON_TDNF_ERROR(dwError);

    for (i = 0, k = 0; pppszArrayList[i]; i++)
    {
        for (j = 0; pppszArrayList[i][j]; j++)
        {
            ppszNewLines[k++] = pppszArrayList[i][j];
        }
        TDNF_SAFE_FREE_MEMORY(pppszArrayList[i]);
    }

    *pppszLines = ppszNewLines;

cleanup:
    if (pDir)
    {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pppszArrayList);
    TDNF_SAFE_FREE_MEMORY(pszFile);
    return dwError;
error:
    goto cleanup;
}

/* read all lines in file pszFile and store in string array
 * pointed to by pppszArray, one entry for each line */
uint32_t
TDNFReadFileToStringArray(
    const char *pszFile,
    char ***pppszArray
    )
{
    uint32_t dwError = 0;
    int nLength = 0;
    char *pszText = NULL;
    char **ppszArray = NULL;

    if (!pszFile || !pppszArray)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFFileReadAllText(pszFile, &pszText, &nLength);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFSplitStringToArray(pszText, "\n", &ppszArray);
    BAIL_ON_TDNF_ERROR(dwError);

    *pppszArray = ppszArray;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszText);
    return dwError;
error:
    TDNF_SAFE_FREE_STRINGARRAY(ppszArray);
    goto cleanup;
}
