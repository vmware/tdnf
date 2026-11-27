/*
 * Copyright (C) 2024 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <fnmatch.h>
#include <limits.h>
#include <sys/wait.h>

#define RPMSPEC_PATH "/usr/bin/rpmspec"

/*
 * Parse a .spec file and return a NULL-terminated array of BuildRequires.
 * Uses "rpmspec --buildrequires <spec>" to get the list.
 * Caller must free ppszDeps with TDNFFreeStringArray().
 */
static uint32_t
TDNFCliGetSpecBuildDeps(
    const char *pszSpecFile,
    char ***pppszDeps
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    char **ppszDeps = NULL;
    char *pszLine = NULL;
    char *pszSpecPath = NULL;
    size_t nLineSize = 0;
    ssize_t nLength = 0;
    int pipeFds[2] = {-1, -1};
    pid_t pid = -1;
    pid_t waitResult = -1;
    int nStatus = 0;
    int nCount = 0;
    int nCap = 16;

    if (!pszSpecFile || !pppszDeps)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    /*
     * The CLI library does not link against librpmbuild, so the rpmspec
     * binary is used to parse spec files instead of the C API.
     */
    *pppszDeps = NULL;

    if (access(RPMSPEC_PATH, X_OK) != 0)
    {
        pr_err("builddep: rpmspec not found; is rpm-build installed?\n");
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pszSpecPath = realpath(pszSpecFile, NULL);
    if (!pszSpecPath)
    {
        pr_err("builddep: spec file not found: %s\n", pszSpecFile);
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (pipe(pipeFds) != 0)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pid = fork();
    if (pid == -1)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }
    if (pid == 0)
    {
        close(pipeFds[0]);
        if (dup2(pipeFds[1], STDOUT_FILENO) == -1)
        {
            _exit(127);
        }
        if (pipeFds[1] != STDOUT_FILENO)
        {
            close(pipeFds[1]);
        }
        execl(RPMSPEC_PATH, "rpmspec", "-q", "--buildrequires",
              "--", pszSpecPath, (char *)NULL);
        _exit(127);
    }

    close(pipeFds[1]);
    pipeFds[1] = -1;
    fp = fdopen(pipeFds[0], "r");
    if (!fp)
    {
        pr_err("builddep: failed to run rpmspec: %s\n", strerror(errno));
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }
    pipeFds[0] = -1;

    dwError = TDNFAllocateMemory(nCap + 1, sizeof(char *), (void **)&ppszDeps);
    BAIL_ON_CLI_ERROR(dwError);

    while ((nLength = getline(&pszLine, &nLineSize, fp)) >= 0)
    {
        char *pszDep = NULL;

        /* strip trailing newline */
        if (nLength && pszLine[nLength - 1] == '\n')
        {
            pszLine[--nLength] = '\0';
        }

        /* skip empty lines */
        if (nLength == 0)
        {
            continue;
        }

        if (nCount >= nCap)
        {
            char **ppszNewDeps = NULL;

            if (nCap > (INT_MAX - 1) / 2)
            {
                dwError = ERROR_TDNF_OUT_OF_MEMORY;
                BAIL_ON_CLI_ERROR(dwError);
            }
            nCap *= 2;
            ppszNewDeps = realloc(ppszDeps, (nCap + 1) * sizeof(char *));
            if (!ppszNewDeps)
            {
                dwError = ERROR_TDNF_OUT_OF_MEMORY;
                BAIL_ON_CLI_ERROR(dwError);
            }
            ppszDeps = ppszNewDeps;
        }

        dwError = TDNFAllocateString(pszLine, &pszDep);
        BAIL_ON_CLI_ERROR(dwError);

        ppszDeps[nCount++] = pszDep;
        ppszDeps[nCount] = NULL;
    }
    if (!feof(fp))
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + (errno ? errno : EIO);
        BAIL_ON_CLI_ERROR(dwError);
    }

    fclose(fp);
    fp = NULL;
    do
    {
        waitResult = waitpid(pid, &nStatus, 0);
    } while (waitResult == -1 && errno == EINTR);
    pid = -1;
    if (waitResult == -1)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }
    if (!WIFEXITED(nStatus) || WEXITSTATUS(nStatus) != 0)
    {
        pr_err("builddep: rpmspec failed to parse %s\n", pszSpecFile);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    *pppszDeps = ppszDeps;
    ppszDeps = NULL;

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    if (pipeFds[0] != -1)
    {
        close(pipeFds[0]);
    }
    if (pipeFds[1] != -1)
    {
        close(pipeFds[1]);
    }
    if (pid > 0)
    {
        while (waitpid(pid, &nStatus, 0) == -1 && errno == EINTR)
            ;
    }
    TDNF_SAFE_FREE_MEMORY(pszLine);
    TDNF_SAFE_FREE_MEMORY(pszSpecPath);
    return dwError;

error:
    TDNFFreeStringArray(ppszDeps);
    goto cleanup;
}

/*
 * Install packages given as a NULL-terminated array of names via the
 * standard alter/resolve flow.
 */
static uint32_t
TDNFCliInstallPackageList(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    char **ppszPkgs
    )
{
    uint32_t dwError = 0;

    if (!pContext || !pCmdArgs || !ppszPkgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (!ppszPkgs[0])
    {
        pr_info("No build dependencies to install.\n");
        goto cleanup;
    }

    dwError = TDNFCliAlterWithPkgList(
                  pContext, pCmdArgs,
                  "builddep", ppszPkgs,
                  NULL,        /* no download-dir override */
                  ALTER_INSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * tdnf builddep <spec-or-srpm> [<spec-or-srpm> ...]
 *
 * Installs the build dependencies for the given spec files or source RPMs.
 * Spec files are parsed with "rpmspec --buildrequires".
 * Source RPMs and source package names are resolved through the solver
 * using the existing --builddeps / TDNFResolveBuildDependencies path.
 */
uint32_t
TDNFCliBuilddepCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    char **ppszDeps = NULL;
    char **ppszAllDeps = NULL;
    char **ppszSources = NULL;
    int i = 0;
    int nSources = 0;
    int nOldBuildDeps = 0;
    int nRestoreBuildDeps = 0;

    if (!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (pCmdArgs->nCmdCount < 2)
    {
        pr_err("builddep: spec file, source RPM, or source package name required\n");
        dwError = ERROR_TDNF_CLI_BUILDDEP_EXPECT_SPEC;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nOldBuildDeps = pCmdArgs->nBuildDeps;
    nRestoreBuildDeps = 1;
    dwError = TDNFAllocateMemory(1, sizeof(char *), (void **)&ppszAllDeps);
    BAIL_ON_CLI_ERROR(dwError);
    dwError = TDNFAllocateMemory(pCmdArgs->nCmdCount, sizeof(char *),
                               (void **)&ppszSources);
    BAIL_ON_CLI_ERROR(dwError);

    /* Parse every spec before resolving the combined dependency list. */
    for (i = 1; i < pCmdArgs->nCmdCount; i++)
    {
        if (fnmatch("*.spec", pCmdArgs->ppszCmds[i], 0) != 0)
        {
            ppszSources[nSources++] = pCmdArgs->ppszCmds[i];
            continue;
        }

        dwError = TDNFCliGetSpecBuildDeps(pCmdArgs->ppszCmds[i], &ppszDeps);
        BAIL_ON_CLI_ERROR(dwError);

        if (!ppszDeps[0])
        {
            pr_info("No build dependencies found in %s\n",
                    pCmdArgs->ppszCmds[i]);
        }

        dwError = TDNFMergeStringArrays(&ppszAllDeps, ppszDeps);
        BAIL_ON_CLI_ERROR(dwError);
        ppszDeps = NULL;
    }

    if (ppszAllDeps[0])
    {
        pCmdArgs->nBuildDeps = 0;
        dwError = TDNFCliInstallPackageList(pContext, pCmdArgs, ppszAllDeps);
        if (dwError == ERROR_TDNF_CLI_NOTHING_TO_DO)
        {
            dwError = 0;
        }
        BAIL_ON_CLI_ERROR(dwError);
    }

    /*
     * Handle source RPMs and source package names through the resolver.
     * The existing --builddeps flag triggers TDNFResolveBuildDependencies.
     */
    if (nSources)
    {
        pCmdArgs->nBuildDeps = 1;
        dwError = TDNFCliAlterWithPkgList(pContext, pCmdArgs, "builddep",
                                        ppszSources, NULL, ALTER_INSTALL);
        if (dwError == ERROR_TDNF_CLI_NOTHING_TO_DO)
        {
            dwError = 0;
        }
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    TDNFFreeStringArray(ppszDeps);
    TDNFFreeStringArray(ppszAllDeps);
    TDNF_SAFE_FREE_MEMORY(ppszSources);
    if (nRestoreBuildDeps)
    {
        pCmdArgs->nBuildDeps = nOldBuildDeps;
    }
    return dwError;

error:
    goto cleanup;
}
