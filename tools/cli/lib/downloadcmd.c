/*
 * Copyright (C) 2024 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <limits.h>
#include <sys/stat.h>

/*
 * tdnf download <pkg> [<pkg> ...]
 *
 * Download the standalone package RPM to a per-package subdirectory, without
 * installing it.  With --withdeps, also download the full dependency tree.
 *
 * For each requested package a subdirectory named after the package is
 * created inside the base directory (--downloaddir, or the current working
 * directory if not specified).  All downloaded RPMs are placed there.
 *
 * If the per-package subdirectory already exists the entire operation is
 * aborted before any download begins.
 *
 * On success the path of each package's subdirectory is printed.
 *
 * Options:
 *   --downloaddir <dir>   base directory (default: current working directory)
 *   --withdeps            also download the full dependency tree
 *
 * Implementation note:
 *   The per-package download uses TDNFCliAlterWithPkgList (shared with the
 *   builddep command) which temporarily replaces pCmdArgs->ppszCmds and
 *   pCmdArgs->pszDownloadDir, then calls TDNFCliAlterCommand(ALTER_INSTALL).
 *   parseargs always sets nAllDeps=1 for the download command so that the
 *   solver resolves packages independently of what is currently installed.
 *   Without this, packages that are already installed would not be included
 *   in the download set because the solver considers them already satisfied.
 */
uint32_t
TDNFCliDownloadCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    char **ppszPkgs = NULL;
    int nPkgCount = 0;
    int i = 0;
    char szCwd[PATH_MAX] = {0};
    char *pszAllocDir = NULL;
    const char *pszBaseDir = NULL;
    char szPkgDir[PATH_MAX] = {0};
    char *ppszSinglePkg[2] = {0};
    struct stat st = {0};

    if (!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (pCmdArgs->nCmdCount < 2)
    {
        pr_err("download: package name required\n");
        dwError = ERROR_TDNF_CLI_NOT_ENOUGH_ARGS;
        BAIL_ON_CLI_ERROR(dwError);
    }

    /* nDownloadOnly, nAllDeps, nNoDeps/nWithDeps are set implicitly in parseargs */

    /* Determine base directory */
    if (IsNullOrEmptyString(pCmdArgs->pszDownloadDir))
    {
        if (!getcwd(szCwd, sizeof(szCwd)))
        {
            pr_err("download: cannot get current directory: %s\n",
                   strerror(errno));
            dwError = ERROR_TDNF_SYSTEM_BASE + errno;
            BAIL_ON_CLI_ERROR(dwError);
        }
        dwError = TDNFAllocateString(szCwd, &pszAllocDir);
        BAIL_ON_CLI_ERROR(dwError);
        pszBaseDir = pszAllocDir;
    }
    else
    {
        pszBaseDir = pCmdArgs->pszDownloadDir;
    }

    /* Ensure base directory exists */
    if (!pCmdArgs->nDryRun && stat(pszBaseDir, &st) != 0)
    {
        if (mkdir(pszBaseDir, 0755) != 0 && errno != EEXIST)
        {
            pr_err("download: cannot create directory %s: %s\n",
                   pszBaseDir, strerror(errno));
            dwError = ERROR_TDNF_SYSTEM_BASE + errno;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

    ppszPkgs = &pCmdArgs->ppszCmds[1];
    nPkgCount = pCmdArgs->nCmdCount - 1;

    /*
     * Pre-flight check: verify that no per-package subdirectory already
     * exists before starting any download.
     */
    for (i = 0; i < nPkgCount; i++)
    {
        if (snprintf(szPkgDir, sizeof(szPkgDir),
                     "%s/%s", pszBaseDir, ppszPkgs[i]) < 0)
        {
            dwError = ERROR_TDNF_SYSTEM_BASE + errno;
            BAIL_ON_CLI_ERROR(dwError);
        }
        if (stat(szPkgDir, &st) == 0)
        {
            pr_err("download: directory '%s' already exists\n", szPkgDir);
            dwError = ERROR_TDNF_ALREADY_EXISTS;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

    /* Create per-package subdirectories and download into each */
    ppszSinglePkg[1] = NULL;
    for (i = 0; i < nPkgCount; i++)
    {
        if (snprintf(szPkgDir, sizeof(szPkgDir),
                     "%s/%s", pszBaseDir, ppszPkgs[i]) < 0)
        {
            dwError = ERROR_TDNF_SYSTEM_BASE + errno;
            BAIL_ON_CLI_ERROR(dwError);
        }

        if (!pCmdArgs->nDryRun && mkdir(szPkgDir, 0755) != 0)
        {
            pr_err("download: cannot create directory %s: %s\n",
                   szPkgDir, strerror(errno));
            dwError = ERROR_TDNF_SYSTEM_BASE + errno;
            BAIL_ON_CLI_ERROR(dwError);
        }

        ppszSinglePkg[0] = ppszPkgs[i];
        dwError = TDNFCliAlterWithPkgList(
                      pContext, pCmdArgs,
                      "download", ppszSinglePkg,
                      szPkgDir,   /* per-package download directory */
                      ALTER_INSTALL);
        BAIL_ON_CLI_ERROR(dwError);

        if (!pCmdArgs->nDryRun)
        {
            pr_info("Downloaded %s to %s\n", ppszPkgs[i], szPkgDir);
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszAllocDir);
    return dwError;

error:
    goto cleanup;
}
