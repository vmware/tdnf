/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

#include "../llconf/nodes.h"
#include "../llconf/modules.h"
#include "../llconf/entry.h"
#include "../llconf/ini.h"

int
TDNFConfGetRpmVerbosity(
    PTDNF pTdnf
    )
{
    rpmlogLvl nLogLevel = RPMLOG_INFO;
    if(pTdnf && pTdnf->pArgs->nRpmVerbosity >= 0)
    {
        nLogLevel = pTdnf->pArgs->nRpmVerbosity;
    }
    return nLogLevel;
}

static
int isTrue(const char *str)
{
    return strcasecmp(str, "true") == 0 || atoi(str) != 0;
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
    char *pszConfDir = NULL;
    char *pszMinVersionsDir = NULL;
    char *pszPkgLocksDir = NULL;
    char *pszProtectedDir = NULL;

    const char *pszProxyUser = NULL;
    const char *pszProxyPass = NULL;

    struct cnfnode *cn_conf = NULL, *cn_top, *cn;
    struct cnfmodule *mod_ini;

    int nPluginSet = 0;

    if(!pTdnf ||
       IsNullOrEmptyString(pszConfFile) ||
       IsNullOrEmptyString(pszGroup))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_CONF),
                  (void**)&pConf);
    BAIL_ON_TDNF_ERROR(dwError);

    /* defaults */
    pConf->nGPGCheck = 0;
    pConf->nInstallOnlyLimit = 1;
    pConf->nCleanRequirementsOnRemove = 0;
    pConf->nKeepCache = 0;
    pConf->nOpenMax = TDNF_DEFAULT_OPENMAX;

    register_ini(NULL);
    mod_ini = find_cnfmodule("ini");
    if (mod_ini == NULL) {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    cn_conf = cnfmodule_parse_file(mod_ini, pszConfFile);
    if (cn_conf == NULL)
    {
        if (errno != 0)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_TDNF_CONF_FILE_LOAD;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    /* cn_conf == NULL => we will not reach here */
    /* coverity[var_deref_op] */
    cn_top = cn_conf->first_child;

    for(cn = cn_top->first_child; cn; cn = cn->next)
    {
        if ((cn->name[0] == '.') || (cn->value == NULL))
            continue;

        if (strcmp(cn->name, TDNF_CONF_KEY_INSTALLONLY_LIMIT) == 0)
        {
            pConf->nInstallOnlyLimit = atoi(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE) == 0)
        {
            pConf->nCleanRequirementsOnRemove = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_GPGCHECK) == 0)
        {
            pConf->nGPGCheck = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_KEEP_CACHE) == 0)
        {
            pConf->nKeepCache = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_REPOSDIR) == 0 ||
                 strcmp(cn->name, TDNF_CONF_KEY_REPODIR) == 0)
        {
            pConf->pszRepoDir = strdup(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_CACHEDIR) == 0)
        {
            pConf->pszCacheDir = strdup(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PERSISTDIR) == 0)
        {
            pConf->pszPersistDir = strdup(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_DISTROVERPKG) == 0)
        {
            pConf->pszDistroVerPkg = strdup(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_EXCLUDE) == 0)
        {
            dwError = TDNFSplitStringToArray(cn->value,
                                             " ", &pConf->ppszExcludes);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_MINVERSIONS) == 0)
        {
            dwError = TDNFSplitStringToArray(cn->value,
                                             " ", &pConf->ppszMinVersions);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_OPENMAX) == 0)
        {
            pConf->nOpenMax = atoi(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_CHECK_UPDATE_COMPAT) == 0)
        {
            pConf->nCheckUpdateCompat = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_DISTROSYNC_REINSTALL_CHANGED) == 0)
        {
            pConf->nDistroSyncReinstallChanged = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PROXY) == 0)
        {
            pConf->pszProxy = strdup(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PROXY_USER) == 0)
        {
            pszProxyUser = cn->value;
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PROXY_PASS) == 0)
        {
            pszProxyPass = cn->value;
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PLUGINS) == 0)
        {
            /* presence of option disables plugins, no matter the value */
            if(!isTrue(cn->value)) {
                dwError = TDNFSetOpt(
                              pTdnf->pArgs,
                              TDNF_CONF_KEY_NO_PLUGINS, "1");
                BAIL_ON_TDNF_ERROR(dwError);
            }
            nPluginSet = 1;
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PLUGIN_CONF_PATH) == 0)
        {
            dwError = TDNFSetOpt(pTdnf->pArgs, TDNF_CONF_KEY_PLUGIN_CONF_PATH, cn->value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PLUGIN_PATH) == 0)
        {
            dwError = TDNFSetOpt(pTdnf->pArgs, TDNF_CONF_KEY_PLUGIN_PATH, cn->value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    /* if plugins are not enabled explicitely,
       we have to disable them because it's the default */
    if (!nPluginSet) {
        /* no plugins by default */
        dwError = TDNFSetOpt(
                      pTdnf->pArgs,
                      TDNF_CONF_KEY_NO_PLUGINS, "1");
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pszProxyUser && pszProxyPass)
    {
        dwError = TDNFAllocateStringPrintf(
                      &pConf->pszProxyUserPass,
                      "%s:%s",
                      pszProxyUser,
                      pszProxyPass);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pConf->pszRepoDir == NULL)
        pConf->pszRepoDir = strdup(TDNF_DEFAULT_REPO_LOCATION);
    if (pConf->pszCacheDir == NULL)
        pConf->pszCacheDir = strdup(TDNF_DEFAULT_CACHE_LOCATION);
    if (pConf->pszDistroVerPkg == NULL)
        pConf->pszDistroVerPkg = strdup(TDNF_DEFAULT_DISTROVERPKG);
    if (pConf->pszPersistDir == NULL)
        pConf->pszPersistDir = strdup(TDNF_DEFAULT_DB_LOCATION);

    dwError = TDNFDirName(pszConfFile, &pszConfDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszMinVersionsDir, pszConfDir, "minversions.d", NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfFilesFromDir(pszMinVersionsDir, &pConf->ppszMinVersions);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszPkgLocksDir, pszConfDir, "locks.d", NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfFilesFromDir(pszPkgLocksDir, &pConf->ppszPkgLocks);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszProtectedDir, pszConfDir, "protected.d", NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadConfFilesFromDir(pszProtectedDir, &pConf->ppszProtectedPkgs);
    BAIL_ON_TDNF_ERROR(dwError);

    pTdnf->pConf = pConf;

cleanup:
    destroy_cnftree(cn_conf);
    TDNF_SAFE_FREE_MEMORY(pszConfDir);
    TDNF_SAFE_FREE_MEMORY(pszMinVersionsDir);
    TDNF_SAFE_FREE_MEMORY(pszPkgLocksDir);
    TDNF_SAFE_FREE_MEMORY(pszProtectedDir);
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
        TDNF_SAFE_FREE_MEMORY(pConf->pszPersistDir);
        TDNF_SAFE_FREE_MEMORY(pConf->pszDistroVerPkg);
        TDNF_SAFE_FREE_MEMORY(pConf->pszVarReleaseVer);
        TDNF_SAFE_FREE_MEMORY(pConf->pszVarBaseArch);
        TDNF_SAFE_FREE_MEMORY(pConf->pszBaseArch);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszExcludes);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszMinVersions);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszPkgLocks);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszProtectedPkgs);
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
