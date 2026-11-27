/*
 * Copyright (C) 2024 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <dirent.h>
#include <sys/utsname.h>

#define PROC_DIR         "/proc"
#define MAPS_FILE        "maps"
#define COMM_FILE        "comm"
#define DELETED_SUFFIX   " (deleted)"

/* Maximum length of a single /proc/<pid>/maps line */
#define MAPS_LINE_MAX    4096

/*
 * Read the process name from /proc/<pid>/comm.
 * Returns 0 and fills pszName (truncated to nLen) on success.
 */
static uint32_t
ReadProcComm(
    const char *pszPid,
    char *pszName,
    size_t nLen
    )
{
    uint32_t dwError = 0;
    char szPath[PATH_MAX] = {0};
    FILE *fp = NULL;
    char *pszNl = NULL;

    if (snprintf(szPath, sizeof(szPath),
                 "%s/%s/%s", PROC_DIR, pszPid, COMM_FILE) < 0)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    fp = fopen(szPath, "r");
    if (!fp)
    {
        /* process may have exited */
        goto cleanup;
    }

    if (fgets(pszName, (int)nLen, fp))
    {
        pszNl = strchr(pszName, '\n');
        if (pszNl)
        {
            *pszNl = '\0';
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

/*
 * Scan /proc/<pid>/maps for any line referencing a "(deleted)" file.
 * Returns 1 in *pnNeeds if found, 0 otherwise.
 */
static uint32_t
ProcHasDeletedMaps(
    const char *pszPid,
    int *pnNeeds
    )
{
    uint32_t dwError = 0;
    char szPath[PATH_MAX] = {0};
    FILE *fp = NULL;
    char szLine[MAPS_LINE_MAX] = {0};

    *pnNeeds = 0;

    if (snprintf(szPath, sizeof(szPath),
                 "%s/%s/%s", PROC_DIR, pszPid, MAPS_FILE) < 0)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    fp = fopen(szPath, "r");
    if (!fp)
    {
        /* process may have exited or we lack permission */
        goto cleanup;
    }

    while (fgets(szLine, sizeof(szLine), fp))
    {
        /* look for lines ending in " (deleted)" */
        size_t nLen = strlen(szLine);
        size_t nSufLen = strlen(DELETED_SUFFIX);

        /* strip newline first */
        if (nLen > 0 && szLine[nLen - 1] == '\n')
        {
            szLine[--nLen] = '\0';
        }

        if (nLen >= nSufLen &&
            strcmp(szLine + nLen - nSufLen, DELETED_SUFFIX) == 0)
        {
            /*
             * Only flag .so libraries and the main executable, not
             * anonymous mappings like memfd or /dev/zero.
             */
            if (strstr(szLine, ".so") || strstr(szLine, "/bin/") ||
                strstr(szLine, "/sbin/") || strstr(szLine, "/lib/") ||
                strstr(szLine, "/usr/"))
            {
                *pnNeeds = 1;
                break;
            }
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

/*
 * List all PIDs that have deleted shared libraries mapped.
 * Prints "<pid> <name>" for each such process.
 * Returns 1 in *pnFound if any were found.
 */
static uint32_t
TDNFCliListNeedsRestarting(
    int *pnFound
    )
{
    uint32_t dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    int nNeeds = 0;
    char szName[256] = {0};

    *pnFound = 0;

    pDir = opendir(PROC_DIR);
    if (!pDir)
    {
        pr_err("needs-restarting: cannot open %s: %s\n",
               PROC_DIR, strerror(errno));
        dwError = ERROR_TDNF_CLI_NEEDS_RESTARTING_PROC_READ;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while ((pEntry = readdir(pDir)) != NULL)
    {
        const char *pszPid = pEntry->d_name;
        int i = 0;

        /* Only process numeric directory names (PIDs) */
        for (i = 0; pszPid[i]; i++)
        {
            if (pszPid[i] < '0' || pszPid[i] > '9')
            {
                break;
            }
        }
        if (pszPid[i] != '\0' || i == 0)
        {
            continue;
        }

        nNeeds = 0;
        dwError = ProcHasDeletedMaps(pszPid, &nNeeds);
        /* ignore errors for individual processes (may have exited) */
        dwError = 0;

        if (nNeeds)
        {
            szName[0] = '\0';
            ReadProcComm(pszPid, szName, sizeof(szName));
            pr_info("%s  %s\n", pszPid, szName);
            *pnFound = 1;
        }
    }

cleanup:
    if (pDir)
    {
        closedir(pDir);
    }
    return dwError;

error:
    goto cleanup;
}

/*
 * Check if the currently running kernel matches the installed kernel package.
 * Prints a message and sets *pnNeeded = 1 if a reboot is required.
 */
static uint32_t
TDNFCliCheckRebootRequired(
    int *pnNeeded
    )
{
    uint32_t dwError = 0;
    struct utsname uts = {0};
    FILE *fp = NULL;
    char szCmd[PATH_MAX + 64] = {0};
    char szPkgName[256] = {0};
    char szInstalledVer[256] = {0};
    char *pszNl = NULL;

    *pnNeeded = 0;

    if (access("/usr/bin/rpm", X_OK) != 0)
    {
        pr_err("needs-restarting: rpm is not available; cannot check reboot status\n");
        dwError = ERROR_TDNF_CLI_NEEDS_RESTARTING_NO_RPM;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (uname(&uts) != 0)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    /*
     * Discover the kernel package name by querying the running vmlinuz.
     * Photon OS uses "linux", other distros use "kernel".
     */
    if (snprintf(szCmd, sizeof(szCmd),
                 "rpm -qf /boot/vmlinuz-%s --qf '%%{name}' 2>/dev/null",
                 uts.release) < 0)
    {
        goto cleanup;
    }

    fp = popen(szCmd, "r");
    if (!fp)
    {
        goto cleanup;
    }
    if (fgets(szPkgName, sizeof(szPkgName), fp))
    {
        pszNl = strchr(szPkgName, '\n');
        if (pszNl) *pszNl = '\0';
    }
    pclose(fp);
    fp = NULL;

    if (szPkgName[0] == '\0')
    {
        goto cleanup;
    }

    /*
     * Query the latest installed kernel package version.
     * "rpm -q --last <name>" lists installs newest first; take the first line.
     */
    if (snprintf(szCmd, sizeof(szCmd),
                 "rpm -q --last '%s' 2>/dev/null | head -1 | awk '{print $1}'",
                 szPkgName) < 0)
    {
        goto cleanup;
    }

    fp = popen(szCmd, "r");
    if (!fp)
    {
        goto cleanup;
    }
    if (fgets(szInstalledVer, sizeof(szInstalledVer), fp))
    {
        pszNl = strchr(szInstalledVer, '\n');
        if (pszNl) *pszNl = '\0';
    }
    pclose(fp);
    fp = NULL;

    if (szInstalledVer[0] == '\0')
    {
        goto cleanup;
    }

    /*
     * szInstalledVer is like "linux-6.12.0-1.ph5.x86_64" or
     * "kernel-5.10.0-1.ph4.x86_64".  uts.release is "6.12.0-1.ph5".
     * If the running release string is not in the installed package NEVRA,
     * the kernel was updated and a reboot is needed.
     */
    if (!strstr(szInstalledVer, uts.release))
    {
        pr_info("Core libraries or services have been updated since boot:\n");
        pr_info("  Running kernel : %s\n", uts.release);
        pr_info("  Installed      : %s\n", szInstalledVer);
        pr_info("Reboot is required to fully utilize these updates.\n");
        *pnNeeded = 1;
    }

cleanup:
    if (fp)
    {
        pclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

/*
 * tdnf needs-restarting [-r]
 *
 * Without flags: list running processes that use deleted/updated shared libs.
 *   Exit 0 if none need restarting, 1 if any do.
 *
 * With -r: check whether the system requires a reboot (kernel update etc.)
 *   Exit 0 if no reboot needed, 1 if reboot is required.
 */
uint32_t
TDNFCliNeedsRestartingCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nCheckReboot = 0;
    int nFound = 0;

    UNUSED(pContext);

    if (!pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nCheckReboot = pCmdArgs->nRebootRequired;

    if (nCheckReboot)
    {
        dwError = TDNFCliCheckRebootRequired(&nFound);
        BAIL_ON_CLI_ERROR(dwError);

        if (!nFound)
        {
            pr_info("No reboot required.\n");
        }
    }
    else
    {
        dwError = TDNFCliListNeedsRestarting(&nFound);
        BAIL_ON_CLI_ERROR(dwError);

        if (!nFound)
        {
            pr_info("No processes need restarting.\n");
        }
    }

    /* Exit 1 means action is needed; it is not a command failure. */
    dwError = nFound ? 1 : 0;

cleanup:
    return dwError;

error:
    goto cleanup;
}
