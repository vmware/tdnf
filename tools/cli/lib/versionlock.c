/*
 * Copyright (C) 2024 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <sys/stat.h>

#define VERSIONLOCK_FILENAME  "versionlock.conf"

/* ------------------------------------------------------------------ */
/* Path helpers                                                         */
/* ------------------------------------------------------------------ */

/* Build paths beside the configuration selected by TDNFOpenHandle. */
uint32_t
DeriveLockPaths(
    PTDNF_CMD_ARGS pCmdArgs,
    char **ppszLocksDir,
    char **ppszLockFile
    )
{
    uint32_t dwError = 0;
    char *pszConfFile = NULL;
    char *pszConfDir = NULL;
    char *pszLocksDir = NULL;
    char *pszLockFile = NULL;

    if (!pCmdArgs || !ppszLocksDir || !ppszLockFile)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }
    *ppszLocksDir = NULL;
    *ppszLockFile = NULL;

    dwError = TDNFGetConfigFile(pCmdArgs, &pszConfFile);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFDirName(pszConfFile, &pszConfDir);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFJoinPath(&pszLocksDir, pszConfDir, "locks.d", NULL);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFJoinPath(&pszLockFile, pszLocksDir,
                           VERSIONLOCK_FILENAME, NULL);
    BAIL_ON_CLI_ERROR(dwError);

    *ppszLocksDir = pszLocksDir;
    *ppszLockFile = pszLockFile;
    pszLocksDir = NULL;
    pszLockFile = NULL;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszConfFile);
    TDNF_SAFE_FREE_MEMORY(pszConfDir);
    TDNF_SAFE_FREE_MEMORY(pszLocksDir);
    TDNF_SAFE_FREE_MEMORY(pszLockFile);
    return dwError;

error:
    goto cleanup;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static uint32_t
EnsureLocksDir(
    const char *pszLocksDir
    )
{
    struct stat st = {0};

    if (stat(pszLocksDir, &st) == 0)
    {
        return 0;
    }

    if (mkdir(pszLocksDir, 0755) != 0 && errno != EEXIST)
    {
        pr_err("lock: cannot create %s: %s\n",
               pszLocksDir, strerror(errno));
        return ERROR_TDNF_SYSTEM_BASE + errno;
    }

    return 0;
}

/*
 * Read all entries from the versionlock file into a NULL-terminated array.
 * Returns an empty (but non-NULL) array if the file does not exist yet.
 * Caller must free with TDNFFreeStringArray().
 */
static uint32_t
ReadLockFile(
    const char *pszLockFile,
    char ***pppszEntries,
    int *pnCount
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    char **ppszEntries = NULL;
    char szLine[512] = {0};
    int nCount = 0;
    int nCap = 8;

    *pppszEntries = NULL;
    *pnCount = 0;

    dwError = TDNFAllocateMemory(nCap + 1, sizeof(char *),
                                 (void **)&ppszEntries);
    BAIL_ON_CLI_ERROR(dwError);

    fp = fopen(pszLockFile, "r");
    if (!fp)
    {
        if (errno == ENOENT)
        {
            /* File does not exist yet; return empty list */
            goto cleanup;
        }
        pr_err("lock: cannot open %s: %s\n",
               pszLockFile, strerror(errno));
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while (fgets(szLine, sizeof(szLine), fp))
    {
        char *pszNl = NULL;
        char *pszEntry = NULL;

        pszNl = strchr(szLine, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
        }

        /* skip blank lines and comments */
        if (szLine[0] == '\0' || szLine[0] == '#')
        {
            continue;
        }

        if (nCount >= nCap)
        {
            nCap *= 2;
            dwError = TDNFReAllocateMemory((nCap + 1) * sizeof(char *),
                                           (void **)&ppszEntries);
            BAIL_ON_CLI_ERROR(dwError);
        }

        dwError = TDNFAllocateString(szLine, &pszEntry);
        BAIL_ON_CLI_ERROR(dwError);

        ppszEntries[nCount++] = pszEntry;
        ppszEntries[nCount] = NULL;
    }

    *pppszEntries = ppszEntries;
    ppszEntries = NULL;
    *pnCount = nCount;

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    TDNFFreeStringArray(ppszEntries);
    goto cleanup;
}

/*
 * Write the entry array back to the lockfile (overwrites it).
 */
static uint32_t
WriteLockFile(
    const char *pszLocksDir,
    const char *pszLockFile,
    char **ppszEntries,
    int nCount
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    int i = 0;

    dwError = EnsureLocksDir(pszLocksDir);
    BAIL_ON_CLI_ERROR(dwError);

    fp = fopen(pszLockFile, "w");
    if (!fp)
    {
        pr_err("lock: cannot write %s: %s\n",
               pszLockFile, strerror(errno));
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    for (i = 0; i < nCount; i++)
    {
        if (fprintf(fp, "%s\n", ppszEntries[i]) < 0)
        {
            dwError = ERROR_TDNF_SYSTEM_BASE + errno;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

/* ------------------------------------------------------------------ */
/* Subcommands                                                          */
/* ------------------------------------------------------------------ */

/*
 * lock list
 * Print all locked package names, one per line.
 */
uint32_t
TDNFCliLockList(
    const char *pszLockFile
    )
{
    uint32_t dwError = 0;
    char **ppszEntries = NULL;
    int nCount = 0;
    int i = 0;

    dwError = ReadLockFile(pszLockFile, &ppszEntries, &nCount);
    BAIL_ON_CLI_ERROR(dwError);

    if (nCount == 0)
    {
        pr_info("lock list is empty.\n");
    }
    else
    {
        for (i = 0; i < nCount; i++)
        {
            pr_info("%s\n", ppszEntries[i]);
        }
    }

cleanup:
    TDNFFreeStringArray(ppszEntries);
    return dwError;

error:
    goto cleanup;
}

/*
 * lock add <pkg> [<pkg> ...]
 * Add package names to the lockfile (no duplicates).
 */
uint32_t
TDNFCliLockAdd(
    const char *pszLocksDir,
    const char *pszLockFile,
    char **ppszPkgs,
    int nPkgCount
    )
{
    uint32_t dwError = 0;
    char **ppszEntries = NULL;
    int nCount = 0;
    int i = 0;
    int j = 0;
    int nDup = 0;

    if (!ppszPkgs || nPkgCount == 0)
    {
        dwError = ERROR_TDNF_CLI_VERSIONLOCK_EXPECT_ARG;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = ReadLockFile(pszLockFile, &ppszEntries, &nCount);
    BAIL_ON_CLI_ERROR(dwError);

    for (i = 0; i < nPkgCount; i++)
    {
        nDup = 0;
        for (j = 0; j < nCount; j++)
        {
            if (strcmp(ppszPkgs[i], ppszEntries[j]) == 0)
            {
                nDup = 1;
                break;
            }
        }

        if (nDup)
        {
            pr_info("lock: %s is already locked\n", ppszPkgs[i]);
            continue;
        }

        dwError = TDNFReAllocateMemory((nCount + 2) * sizeof(char *),
                                       (void **)&ppszEntries);
        BAIL_ON_CLI_ERROR(dwError);

        dwError = TDNFAllocateString(ppszPkgs[i], &ppszEntries[nCount]);
        BAIL_ON_CLI_ERROR(dwError);

        nCount++;
        ppszEntries[nCount] = NULL;
        pr_info("lock: added lock for %s\n", ppszPkgs[i]);
    }

    dwError = WriteLockFile(pszLocksDir, pszLockFile, ppszEntries, nCount);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    TDNFFreeStringArray(ppszEntries);
    return dwError;

error:
    goto cleanup;
}

/*
 * lock delete <pkg> [<pkg> ...]
 * Remove package names from the lockfile.
 */
uint32_t
TDNFCliLockDelete(
    const char *pszLocksDir,
    const char *pszLockFile,
    char **ppszPkgs,
    int nPkgCount
    )
{
    uint32_t dwError = 0;
    char **ppszEntries = NULL;
    char **ppszNew = NULL;
    int nCount = 0;
    int nNew = 0;
    int i = 0;
    int j = 0;
    int nMatch = 0;

    if (!ppszPkgs || nPkgCount == 0)
    {
        dwError = ERROR_TDNF_CLI_VERSIONLOCK_EXPECT_ARG;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = ReadLockFile(pszLockFile, &ppszEntries, &nCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFAllocateMemory(nCount + 1, sizeof(char *),
                                 (void **)&ppszNew);
    BAIL_ON_CLI_ERROR(dwError);

    for (i = 0; i < nCount; i++)
    {
        nMatch = 0;
        for (j = 0; j < nPkgCount; j++)
        {
            if (strcmp(ppszEntries[i], ppszPkgs[j]) == 0)
            {
                nMatch = 1;
                pr_info("lock: removed lock for %s\n", ppszEntries[i]);
                break;
            }
        }

        if (!nMatch)
        {
            dwError = TDNFAllocateString(ppszEntries[i], &ppszNew[nNew++]);
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
    ppszNew[nNew] = NULL;

    dwError = WriteLockFile(pszLocksDir, pszLockFile, ppszNew, nNew);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    TDNFFreeStringArray(ppszEntries);
    TDNFFreeStringArray(ppszNew);
    return dwError;

error:
    goto cleanup;
}

/*
 * lock clear
 * Remove all entries from the lockfile.
 */
uint32_t
TDNFCliLockClear(
    const char *pszLocksDir,
    const char *pszLockFile
    )
{
    uint32_t dwError = 0;

    dwError = WriteLockFile(pszLocksDir, pszLockFile, NULL, 0);
    BAIL_ON_CLI_ERROR(dwError);

    pr_info("lock: all locks cleared\n");

cleanup:
    return dwError;

error:
    goto cleanup;
}

/* ------------------------------------------------------------------ */
/* Entry point                                                          */
/* ------------------------------------------------------------------ */

/*
 * tdnf lock <subcommand> [args]
 *
 * Subcommands:
 *   list              list all locked packages
 *   add <pkg> ...     lock packages by name
 *   delete <pkg> ...  remove locks for packages
 *   clear             remove all locks
 *
 * Unlike dnf versionlock, tdnf locks pin by name and block all solver actions
 * including removal. Locks are stored in locks.d/versionlock.conf under the
 * same directory as the active tdnf.conf.
 */
uint32_t
TDNFCliLockCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    const char *pszSubCmd = NULL;
    char *pszLocksDir = NULL;
    char *pszLockFile = NULL;

    UNUSED(pContext);

    if (!pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = DeriveLockPaths(pCmdArgs, &pszLocksDir, &pszLockFile);
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nCmdCount < 2)
    {
        /* Default with no subcommand: show the list */
        dwError = TDNFCliLockList(pszLockFile);
        BAIL_ON_CLI_ERROR(dwError);
        goto cleanup;
    }

    pszSubCmd = pCmdArgs->ppszCmds[1];

    if (strcmp(pszSubCmd, "list") == 0)
    {
        dwError = TDNFCliLockList(pszLockFile);
    }
    else if (strcmp(pszSubCmd, "add") == 0)
    {
        if (pCmdArgs->nCmdCount < 3)
        {
            pr_err("lock add: package name required\n");
            dwError = ERROR_TDNF_CLI_VERSIONLOCK_EXPECT_ARG;
            BAIL_ON_CLI_ERROR(dwError);
        }
        dwError = TDNFCliLockAdd(
                      pszLocksDir,
                      pszLockFile,
                      &pCmdArgs->ppszCmds[2],
                      pCmdArgs->nCmdCount - 2);
    }
    else if (strcmp(pszSubCmd, "delete") == 0 ||
             strcmp(pszSubCmd, "del") == 0)
    {
        if (pCmdArgs->nCmdCount < 3)
        {
            pr_err("lock delete: package name required\n");
            dwError = ERROR_TDNF_CLI_VERSIONLOCK_EXPECT_ARG;
            BAIL_ON_CLI_ERROR(dwError);
        }
        dwError = TDNFCliLockDelete(
                      pszLocksDir,
                      pszLockFile,
                      &pCmdArgs->ppszCmds[2],
                      pCmdArgs->nCmdCount - 2);
    }
    else if (strcmp(pszSubCmd, "clear") == 0)
    {
        dwError = TDNFCliLockClear(pszLocksDir, pszLockFile);
    }
    else
    {
        pr_err("lock: unknown subcommand '%s'\n", pszSubCmd);
        pr_err("Usage: tdnf lock {list|add|delete|clear} [<package>...]\n");
        dwError = ERROR_TDNF_CLI_VERSIONLOCK_UNKNOWN_CMD;
    }
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszLocksDir);
    TDNF_SAFE_FREE_MEMORY(pszLockFile);
    return dwError;

error:
    goto cleanup;
}
