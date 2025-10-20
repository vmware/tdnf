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


#define USERAGENT_HEADER_MAX_LENGTH 256
#define OS_REL_FILE "/etc/os-release"
#define KEY_MAX_LEN                     32

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

static uint32_t TDNFParseOSInfo(PTDNF_CONF pConf, const char *os_rel_fn)
{
    char *name = NULL;
    char *version = NULL;
    uint32_t dwError = 0;
    FILE *fp = NULL;
    size_t s;
    char *buf;
    const char keys[][KEY_MAX_LEN] = {
        "ID",
        "VERSION_ID"
    };

    fp = fopen(os_rel_fn, "r");
    if (!fp) {
        pr_info("Warning: '%s' file is not present in the system\n", os_rel_fn);
        return dwError;
    }

    s = USERAGENT_HEADER_MAX_LENGTH;
    dwError = TDNFAllocateMemory(1, s, (void **)&buf);
    BAIL_ON_TDNF_ERROR(dwError);

    while (getline(&buf, &s, fp) >= 0) {
        buf[strcspn(buf, "\n")] = '\0';
        for (int i = 0; i < (int)ARRAY_SIZE(keys); ++i) {
            char *beg;
            const char *key = keys[i];
            int key_l = strlen(key);

            if (strncmp(key, buf, key_l))
                continue;

            /* read the value within double quotes */
            beg = strchr(buf + key_l + 1, '"'); // skip till 'KEY='
            if (beg) {
                char *end;

                beg += 1;
                end = strrchr(beg, '"');
                *end = '\0';
                snprintf(buf, s, "%s=%s", key, beg);
            }

            if (strncmp(buf, "ID=", sizeof("ID=") - 1)) {
                dwError = TDNFAllocateString(buf, &name);
            } else if (strncmp(buf, "VERSION_ID=", sizeof("VERSION_ID=") - 1)) {
                dwError = TDNFAllocateString(buf, &version);
            }
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

    pConf->pszOSName = name;
    pConf->pszOSVersion = version;

cleanup:
    TDNF_SAFE_FREE_MEMORY(buf);
    fclose(fp);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(name);
    TDNF_SAFE_FREE_MEMORY(version);
    goto cleanup;
}

struct {
    const char *name;
    rpmtransFlags flag;
} rpmtransflags_map[] = {
    {"noscripts",       RPMTRANS_FLAG_NOSCRIPTS },
    {"justdb",          RPMTRANS_FLAG_JUSTDB },
    {"notriggers",      RPMTRANS_FLAG_NOTRIGGERS },
    {"nodocs",          RPMTRANS_FLAG_NODOCS },
    {"allfiles",        RPMTRANS_FLAG_ALLFILES },
    {"noplugins",       RPMTRANS_FLAG_NOPLUGINS },
    {"nocontexts",      RPMTRANS_FLAG_NOCONTEXTS },
    {"nocaps",          RPMTRANS_FLAG_NOCAPS},
    {"nodb",            RPMTRANS_FLAG_NODB},

//    {"nopreuntrans",    RPMTRANS_FLAG_NOPREUNTRANS },
//    {"nopostuntrans",   RPMTRANS_FLAG_NOPOSTUNTRANS },
    {"notriggerprein",  RPMTRANS_FLAG_NOTRIGGERPREIN },
    {"nopre",           RPMTRANS_FLAG_NOPRE },
    {"nopost",          RPMTRANS_FLAG_NOPOST },
    {"notriggerin",     RPMTRANS_FLAG_NOTRIGGERIN },
    {"notriggerun",     RPMTRANS_FLAG_NOTRIGGERUN },
    {"nopreun",         RPMTRANS_FLAG_NOPREUN },
    {"nopostun",        RPMTRANS_FLAG_NOPOSTUN },
    {"notriggerpostun", RPMTRANS_FLAG_NOTRIGGERPOSTUN },
    {"nopretrans",      RPMTRANS_FLAG_NOPRETRANS },
    {"noposttrans",     RPMTRANS_FLAG_NOPOSTTRANS},
//    {"nosysusers",      RPMTRANS_FLAG_NOSYSUSERS},
    {"nomd5",           RPMTRANS_FLAG_NOMD5},
    {"nofiledigest",    RPMTRANS_FLAG_NOFILEDIGEST},

    {"noartifacts",     RPMTRANS_FLAG_NOARTIFACTS},
    {"noconfigs",       RPMTRANS_FLAG_NOCONFIGS},
    {"deploops",        RPMTRANS_FLAG_DEPLOOPS},
    {NULL,              RPMTRANS_FLAG_NONE}
};

static
uint32_t
TDNFConfigFromCnfTree(PTDNF_CONF pConf, struct cnfnode *cn_top)
{
    uint32_t dwError = 0;
    struct cnfnode *cn;
    const char *pszProxyUser = NULL;
    const char *pszProxyPass = NULL;

    for(cn = cn_top->first_child; cn; cn = cn->next)
    {
        if ((cn->name[0] == '.') || (cn->value == NULL))
            continue;

        if (strcmp(cn->name, TDNF_CONF_KEY_INSTALLONLY_LIMIT) == 0)
        {
            pConf->nInstallOnlyLimit = strtoi(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE) == 0)
        {
            pConf->nCleanRequirementsOnRemove = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_GPGCHECK) == 0)
        {
            pConf->nGPGCheck = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_CMDLINEGPGCHECK) == 0)
        {
            pConf->nCliGPGCheck = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_SSL_VERIFY) == 0)
        {
            pConf->nSSLVerify = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_KEEP_CACHE) == 0)
        {
            pConf->nKeepCache = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_REPOSDIR) == 0 ||
                 strcmp(cn->name, TDNF_CONF_KEY_REPODIR) == 0)
        {
            SET_STRING(pConf->pszRepoDir, cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_CACHEDIR) == 0)
        {
            SET_STRING(pConf->pszCacheDir, cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PERSISTDIR) == 0)
        {
            SET_STRING(pConf->pszPersistDir, cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_DISTROVERPKGS) == 0)
        {
            dwError = TDNFSplitStringToArray(cn->value,
                                             (char *)" ", &pConf->ppszDistroVerPkgs);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_EXCLUDE) == 0)
        {
            dwError = TDNFAddStringArray(&pConf->ppszExcludes, cn->value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_MINVERSIONS) == 0)
        {
            dwError = TDNFAddStringArray(&pConf->ppszMinVersions, cn->value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_OPENMAX) == 0)
        {
            pConf->nOpenMax = strtoi(cn->value);
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
            SET_STRING(pConf->pszProxy, cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PROXY_USER) == 0)
        {
            pszProxyUser = cn->value;
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PROXY_PASS) == 0)
        {
            pszProxyPass = cn->value;
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_INSTALLONLYPKGS) == 0)
        {
            dwError = TDNFAddStringArray(&pConf->ppszInstallOnlyPkgs, cn->value);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_VARS_DIRS) == 0)
        {
            dwError = TDNFSplitStringToArray(cn->value,
                                             (char *)" ", &pConf->ppszVarsDirs);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PLUGINS) == 0)
        {
            pConf->nPluginsEnabled = isTrue(cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PLUGIN_CONF_PATH) == 0)
        {
            SET_STRING(pConf->pszPluginConfPath, cn->value);
        }
        else if (strcmp(cn->name, TDNF_CONF_KEY_PLUGIN_PATH) == 0)
        {
            SET_STRING(pConf->pszPluginPath, cn->value);
        }
        else if  (strcmp(cn->name, TDNF_CONF_KEY_TSFLAGS) == 0)
        {
            if (cn->value == NULL || strcmp(cn->value, "") == 0) {
                pConf->rpmTransFlags = RPMTRANS_FLAG_NONE;
            } else {
                char *value = NULL;
                char *saveptr = NULL, *str, *token;
                int i;

                SET_STRING(value, cn->value);

                for (str = value; ; str = NULL){
                    token = strtok_r(str, " ", &saveptr);
                    if (token == NULL)
                        break;

                    for (i = 0; rpmtransflags_map[i].name != NULL; i++) {
                        if (strcmp(token, rpmtransflags_map[i].name) == 0){
                            pConf->rpmTransFlags |= rpmtransflags_map[i].flag;
                            break;
                        }
                    }
                    if (rpmtransflags_map[i].name == NULL) {
                        pr_err("unknown tsflag '%s'\n", token);
                        free(value);
                        dwError = ERROR_TDNF_INVALID_PARAMETER;
                        BAIL_ON_TDNF_ERROR(dwError);
                    }
                    /* in rpmts.h, deprecated flags will be set to 0. Warn user about it. */
                    if (rpmtransflags_map[i].flag == 0) {
                        pr_info("flag tsflag '%s' is not supported and has no effect\n", token);
                    }
                }
                free(value);
            }
        }
    }

    if (pszProxyUser && pszProxyPass)
    {
        TDNF_SAFE_FREE_MEMORY(pConf->pszProxyUserPass);
        dwError = TDNFAllocateStringPrintf(
                      &pConf->pszProxyUserPass,
                      "%s:%s",
                      pszProxyUser,
                      pszProxyPass);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
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
    char *pszCacheDir = NULL;
    char *pszRepoDir = NULL;
    const char *pszTdnfVersion = NULL;
    struct cnfnode *cn_conf = NULL;
    struct cnfmodule *mod_ini;

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
    pConf->nCliGPGCheck = -1; /* mark as unset */
    pConf->nCleanRequirementsOnRemove = 0;
    pConf->nKeepCache = 0;
    pConf->nOpenMax = TDNF_CONF_DEFAULT_OPENMAX;
    pConf->nInstallOnlyLimit = TDNF_CONF_DEFAULT_INSTALLONLY_LIMIT;
    pConf->nSSLVerify = TDNF_CONF_DEFAULT_SSLVERIFY;

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

    dwError = TDNFParseOSInfo(pConf, OS_REL_FILE);
    BAIL_ON_TDNF_ERROR(dwError);

    /* cn_conf == NULL => we will not reach here */
    /* coverity[var_deref_op] */
    dwError = TDNFConfigFromCnfTree(pConf, cn_conf->first_child);
    BAIL_ON_TDNF_ERROR(dwError);

    /* override from cmd line */
    if (pTdnf->pArgs->nNoGPGCheck) {
        pConf->nGPGCheck = 0;
    }

    /* these two have no config setting (future?) */
    if (pTdnf->pArgs->nSkipDigest) {
        pConf->nSkipDigest = 1;
    }
    if (pTdnf->pArgs->nSkipSignature) {
        pConf->nSkipSignature = 1;
    }

    if (pConf->pszRepoDir == NULL)
        SET_STRING(pConf->pszRepoDir, TDNF_DEFAULT_REPO_LOCATION);
    if (pConf->pszCacheDir == NULL)
        SET_STRING(pConf->pszCacheDir, TDNF_DEFAULT_CACHE_LOCATION);

    if (!IsNullOrEmptyString(pTdnf->pArgs->pszInstallRoot) &&
        strcmp(pTdnf->pArgs->pszInstallRoot, "/"))
    {
        int nIsDir = 0;
        dwError = TDNFJoinPath(&pszCacheDir,
                               pTdnf->pArgs->pszInstallRoot,
                               pConf->pszCacheDir,
                               NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        TDNF_SAFE_FREE_MEMORY(pConf->pszCacheDir);
        pConf->pszCacheDir = pszCacheDir;
        pszCacheDir = NULL;

        dwError = TDNFJoinPath(&pszRepoDir,
                               pTdnf->pArgs->pszInstallRoot,
                               pConf->pszRepoDir,
                               NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFIsDir(pszRepoDir, &nIsDir);
        if (dwError == ERROR_TDNF_FILE_NOT_FOUND)
        {
            nIsDir = 0;
            dwError = 0;
        }
        BAIL_ON_TDNF_ERROR(dwError);
        if (nIsDir)
        {
            TDNF_SAFE_FREE_MEMORY(pConf->pszRepoDir);
            pConf->pszRepoDir = pszRepoDir;
            pszRepoDir = NULL;
        }
    }

    if (pTdnf->pArgs->cn_setopts) {
        dwError = TDNFConfigFromCnfTree(pConf, pTdnf->pArgs->cn_setopts);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszTdnfVersion = TDNFGetVersion();

    /* override from cmd line */
    if (pTdnf->pArgs->nNoCmdLineGPGCheck) {
        pConf->nCliGPGCheck = 0;
        /* if unset, same as general nGPGCheck */
    } else if (pConf->nCliGPGCheck == -1) {
        pConf->nCliGPGCheck = pConf->nGPGCheck;
    }

    if (pConf->pszOSName == NULL)
        TDNFAllocateString("UNKNOWN", &pConf->pszOSName);

    if (pConf->pszOSVersion == NULL)
        TDNFAllocateString("UNKNOWN", &pConf->pszOSVersion);

    dwError = TDNFAllocateStringPrintf(&pConf->pszUserAgentHeader, "tdnf/%s %s/%s", pszTdnfVersion, pConf->pszOSName, pConf->pszOSVersion);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pConf->ppszDistroVerPkgs == NULL) {
        dwError = TDNFSplitStringToArray(TDNF_DEFAULT_DISTROVERPKGS,
                                         (char *)" ", &pConf->ppszDistroVerPkgs);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pConf->pszPersistDir == NULL)
        SET_STRING(pConf->pszPersistDir, TDNF_DEFAULT_DB_LOCATION);

    if (pConf->ppszVarsDirs == NULL) {
        dwError = TDNFSplitStringToArray(TDNF_DEFAULT_VARS_DIRS,
                                         (char *)" ", &pConf->ppszVarsDirs);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pConf->pszPluginPath == NULL)
        SET_STRING(pConf->pszPluginPath, TDNF_DEFAULT_PLUGIN_PATH);
    if (pConf->pszPluginConfPath == NULL)
        SET_STRING(pConf->pszPluginConfPath, TDNF_DEFAULT_PLUGIN_CONF_PATH);

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
    TDNF_SAFE_FREE_MEMORY(pszCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRepoDir);
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

    if(!pConf->pszVarReleaseVer || IsNullOrEmptyString(pTdnf->pArgs->pszReleaseVer))
    {
        int i;

        for (i = 0; pConf->ppszDistroVerPkgs[i]; i++) {
            dwError = TDNFGetReleaseVersion(
                          pTdnf->pArgs->pszInstallRoot,
                          pConf->ppszDistroVerPkgs[i],
                          &pConf->pszVarReleaseVer);

            if (dwError == 0)
                break;
            else if (dwError != ERROR_TDNF_NO_DISTROVERPKG)
                BAIL_ON_TDNF_ERROR(dwError);
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* pszArch (from --forcearch) overrides variable */
    if (!IsNullOrEmptyString(pTdnf->pArgs->pszArch)) {
        TDNF_SAFE_FREE_MEMORY(pConf->pszVarBaseArch);
        dwError = TDNFAllocateString(pTdnf->pArgs->pszArch,
                      &pConf->pszVarBaseArch);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* use system arch if unset */
    if(IsNullOrEmptyString(pConf->pszVarBaseArch))
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
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszDistroVerPkgs);
        TDNF_SAFE_FREE_MEMORY(pConf->pszVarReleaseVer);
        TDNF_SAFE_FREE_MEMORY(pConf->pszVarBaseArch);
        TDNF_SAFE_FREE_MEMORY(pConf->pszBaseArch);
        TDNF_SAFE_FREE_MEMORY(pConf->pszUserAgentHeader);
        TDNF_SAFE_FREE_MEMORY(pConf->pszOSName);
        TDNF_SAFE_FREE_MEMORY(pConf->pszOSVersion);
        TDNF_SAFE_FREE_MEMORY(pConf->pszPluginPath);
        TDNF_SAFE_FREE_MEMORY(pConf->pszPluginConfPath);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszExcludes);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszMinVersions);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszPkgLocks);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszProtectedPkgs);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszInstallOnlyPkgs);
        TDNF_SAFE_FREE_STRINGARRAY(pConf->ppszVarsDirs);
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
    struct cnfnode * cn_vars = NULL, *cn;

    if(!pTdnf || !ppszString || IsNullOrEmptyString(*ppszString))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    cn_vars = parse_varsdirs(pTdnf->pConf->ppszVarsDirs);
    if (cn_vars == NULL) {
        pr_err("parsing vars failed: %s (%d)\n", strerror(errno), errno);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    cn = create_cnfnode(TDNF_VAR_RELEASEVER);
    cnfnode_setval(cn, pTdnf->pConf->pszVarReleaseVer);
    append_node(cn_vars, cn);

    cn = create_cnfnode(TDNF_VAR_BASEARCH);
    cnfnode_setval(cn, pTdnf->pConf->pszVarBaseArch);
    append_node(cn_vars, cn);

    pszDst = replace_vars(cn_vars, *ppszString);
    if (pszDst == NULL) {
        pr_err("replacing vars in %s failed\n", *ppszString);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    TDNFFreeMemory(*ppszString);
    *ppszString = pszDst;

cleanup:
    destroy_cnftree(cn_vars);
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
