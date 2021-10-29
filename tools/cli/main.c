/*
 * Copyright (C) 2015-2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : main.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

TDNF_CLI_CONTEXT _context = {0};

/* glockfd used in exit handler nad inst*/
static int32_t glockfd = 0;

static void TdnfExitHandler(void);
static bool IsTdnfAlreadyRunning(void);

int main(int argc, char* argv[])
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    TDNF_CLI_CMD_MAP arCmdMap[] =
    {
        {"autoerase",          TDNFCliAutoEraseCommand},
        {"autoremove",         TDNFCliAutoEraseCommand},
        {"check",              TDNFCliCheckCommand},
        {"check-local",        TDNFCliCheckLocalCommand},
        {"check-update",       TDNFCliCheckUpdateCommand},
        {"clean",              TDNFCliCleanCommand},
        {"count",              TDNFCliCountCommand},
        {"distro-sync",        TDNFCliDistroSyncCommand},
        {"downgrade",          TDNFCliDowngradeCommand},
        {"erase",              TDNFCliEraseCommand},
        {"help",               TDNFCliHelpCommand},
        {"info",               TDNFCliInfoCommand},
        {"install",            TDNFCliInstallCommand},
        {"list",               TDNFCliListCommand},
        {"makecache",          TDNFCliMakeCacheCommand},
        {"provides",           TDNFCliProvidesCommand},
        {"whatprovides",       TDNFCliProvidesCommand},
        {"reinstall",          TDNFCliReinstallCommand},
        {"remove",             TDNFCliEraseCommand},
        {"repolist",           TDNFCliRepoListCommand},
        {"reposync",           TDNFCliRepoSyncCommand},
        {"repoquery",          TDNFCliRepoQueryCommand},
        {"search",             TDNFCliSearchCommand},
        {"update",             TDNFCliUpgradeCommand},
        {"update-to",          TDNFCliUpgradeCommand},
        {"upgrade",            TDNFCliUpgradeCommand},
        {"upgrade-to",         TDNFCliUpgradeCommand},
        {"updateinfo",         TDNFCliUpdateInfoCommand},
    };

    int nCommandCount = ARRAY_SIZE(arCmdMap);
    const char* pszCmd = NULL;
    PTDNF pTdnf = NULL;
    int nFound = 0;

    /*
     * granular permissions for non root users are pending.
     * blocking all operations for non root and show the
     * right error to avoid confusion
     */
    if (geteuid())
    {
        dwError = ERROR_TDNF_PERM;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (IsTdnfAlreadyRunning())
    {
        pr_err("An instance of tdnf is already running, wait for it to finish\n");
        dwError = ERROR_TDNF_ACCESS_DENIED;
        BAIL_ON_CLI_ERROR(dwError);
    }

    /* exit handler for normal exit */
    if (atexit(TdnfExitHandler))
    {
        dwError = errno;
        BAIL_ON_CLI_ERROR(dwError);
    }

    _context.pFnCheck = TDNFCliInvokeCheck;
    _context.pFnCheckLocal = TDNFCliInvokeCheckLocal;
    _context.pFnCheckUpdate = TDNFCliInvokeCheckUpdate;
    _context.pFnClean = TDNFCliInvokeClean;
    _context.pFnCount = TDNFCliInvokeCount;
    _context.pFnInfo = TDNFCliInvokeInfo;
    _context.pFnList = TDNFCliInvokeList;
    _context.pFnProvides = TDNFCliInvokeProvides;
    _context.pFnRepoList = TDNFCliInvokeRepoList;
    _context.pFnRepoSync = TDNFCliInvokeRepoSync;
    _context.pFnRepoQuery = TDNFCliInvokeRepoQuery;

    /*
     * Alter and resolve will address commands like
     * install, upgrade, erase, downgrade, distrosync
     */
    _context.pFnAlter = TDNFCliInvokeAlter;
    _context.pFnResolve = TDNFCliInvokeResolve;
    _context.pFnSearch = TDNFCliInvokeSearch;
    _context.pFnUpdateInfo = TDNFCliInvokeUpdateInfo;
    _context.pFnUpdateInfoSummary = TDNFCliInvokeUpdateInfoSummary;

    dwError = TDNFCliParseArgs(argc, argv, &pCmdArgs);
    BAIL_ON_CLI_ERROR(dwError);

    GlobalSetQuiet(pCmdArgs->nQuiet);

    //If --version, show version and exit
    if(pCmdArgs->nShowVersion)
    {
        TDNFCliShowVersion();
    }
    else if(pCmdArgs->nShowHelp)
    {
        TDNFCliShowHelp();
    }
    else if(pCmdArgs->nCmdCount > 0)
    {
        pszCmd = pCmdArgs->ppszCmds[0];
        while(nCommandCount > 0)
        {
            --nCommandCount;
            if(!strcmp(pszCmd, arCmdMap[nCommandCount].pszCmdName))
            {
                nFound = 1;

                if(!strcmp(pszCmd, "makecache"))
                {
                    pCmdArgs->nRefresh = 1;
                }

                dwError = TDNFInit();
                BAIL_ON_CLI_ERROR(dwError);

                dwError = TDNFOpenHandle(pCmdArgs, &pTdnf);
                BAIL_ON_CLI_ERROR(dwError);

                _context.hTdnf = pTdnf;

                if(pCmdArgs->nVerbose)
                {
                    dwError = TDNFCliVerboseShowEnv(pCmdArgs);
                    BAIL_ON_CLI_ERROR(dwError);
                }

                dwError = arCmdMap[nCommandCount].pFnCmd(&_context, pCmdArgs);
                BAIL_ON_CLI_ERROR(dwError);

                break;
            }
        };
        if(!nFound)
        {
            TDNFCliShowNoSuchCommand(pszCmd);
            dwError = ERROR_TDNF_CLI_NO_SUCH_CMD;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
    else
    {
        TDNFCliShowUsage();
    }

cleanup:
    if(pTdnf)
    {
        TDNFCloseHandle(pTdnf);
    }
    if(pCmdArgs)
    {
        TDNFFreeCmdArgs(pCmdArgs);
    }
    TDNFUninit();
    return dwError;

error:
    TDNFCliPrintError(dwError);
    if (dwError == ERROR_TDNF_CLI_NOTHING_TO_DO ||
        dwError == ERROR_TDNF_NO_DATA)
    {
        // Nothing to do should not return an error code
        dwError = 0;
    }

    goto cleanup;
}

static void TdnfExitHandler(void)
{
    if (glockfd < 0)
    {
        return;
    }

    if (flock(glockfd, LOCK_UN))
    {
        pr_err("ERROR: Failed to unlock lock file\n");
    }

    close(glockfd);

    if (remove(TDNF_INSTANCE_LOCK_FILE))
    {
        pr_err("ERROR: Unable to remove lockfile(%s) "
                "Try removing it manually and run tdnf again\n",
                TDNF_INSTANCE_LOCK_FILE);
    }
}

static bool IsTdnfAlreadyRunning(void)
{
    bool ret = false;

    glockfd = open(TDNF_INSTANCE_LOCK_FILE, O_CREAT | O_RDONLY, S_IRWXU);
    if (glockfd < 0)
    {
        pr_err("ERROR: failed to create instance lock file\n");
        goto end;
    }

    if (flock(glockfd, LOCK_EX | LOCK_NB))
    {
        if (errno == EAGAIN)
        {
            ret = true;
            pr_err("ERROR: failed to acquire lock on: %s\n", TDNF_INSTANCE_LOCK_FILE);
        }
    }

end:
    /* glockfd is closed in exit handler */
    return ret;
}

uint32_t
TDNFCliPrintError(
    uint32_t dwErrorCode
    )
{
    uint32_t dwError = 0;
    char* pszError = NULL;

    if (!dwErrorCode)
    {
        return dwError;
    }

    if (dwErrorCode < ERROR_TDNF_BASE)
    {
        dwError = TDNFCliGetErrorString(dwErrorCode, &pszError);
    }
    else
    {
        dwError = TDNFGetErrorString(dwErrorCode, &pszError);
    }
    BAIL_ON_CLI_ERROR(dwError || !pszError);

    if (dwErrorCode == ERROR_TDNF_CLI_NOTHING_TO_DO ||
        dwErrorCode == ERROR_TDNF_NO_DATA)
    {
        dwErrorCode = 0;
    }

    if (dwErrorCode)
    {
        pr_err("Error(%u) : %s\n", dwErrorCode, pszError);
    }
    else
    {
        pr_err("%s\n", pszError);
    }

cleanup:
    TDNF_CLI_SAFE_FREE_MEMORY(pszError);
    return dwError;

error:
    pr_err("Retrieving error string for %u failed with %u\n",
            dwErrorCode, dwError);

    goto cleanup;
}

void
TDNFCliShowVersion(
    void
    )
{
    pr_info("%s: %s\n", TDNFGetPackageName(), TDNFGetVersion());
}

uint32_t
TDNFCliVerboseShowEnv(
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pOpt = NULL;

    if(!pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pOpt = pCmdArgs->pSetOpt;
    if(pOpt)
    {
        pr_info("Setting options:\n");
        while(pOpt)
        {
            pr_info("\t%s = %s\n", pOpt->pszOptName, pOpt->pszOptValue);
            pOpt = pOpt->pNext;
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliInvokeCheck(
    PTDNF_CLI_CONTEXT pContext
    )
{
    return TDNFCheckPackages(pContext->hTdnf);
}

uint32_t
TDNFCliInvokeCheckLocal(
    PTDNF_CLI_CONTEXT pContext,
    const char *pszFolder
    )
{
    return TDNFCheckLocalPackages(pContext->hTdnf, pszFolder);
}

uint32_t
TDNFCliInvokeCheckUpdate(
    PTDNF_CLI_CONTEXT pContext,
    char** ppszPackageArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    )
{
    return TDNFCheckUpdates(pContext->hTdnf,
                            ppszPackageArgs,
                            ppPkgInfo,
                            pdwCount);
}

uint32_t
TDNFCliInvokeClean(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_CLEANTYPE nCleanType,
    PTDNF_CLEAN_INFO *ppTDNFCleanInfo
    )
{
    return TDNFClean(pContext->hTdnf, nCleanType, ppTDNFCleanInfo);
}

uint32_t
TDNFCliInvokeCount(
    PTDNF_CLI_CONTEXT pContext,
    uint32_t *pnCount
    )
{
    return TDNFCountCommand(pContext->hTdnf, pnCount);
}

uint32_t
TDNFCliInvokeAlter(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    return TDNFAlterCommand(pContext->hTdnf, nAlterType, pSolvedPkgInfo);
}

uint32_t
TDNFCliInvokeInfo(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_LIST_ARGS pInfoArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    )
{
    return TDNFInfo(pContext->hTdnf,
                   pInfoArgs->nScope,
                   pInfoArgs->ppszPackageNameSpecs,
                   ppPkgInfo,
                   pdwCount);
}

uint32_t
TDNFCliInvokeList(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_LIST_ARGS pListArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    )
{
    return TDNFList(pContext->hTdnf,
                    pListArgs->nScope,
                    pListArgs->ppszPackageNameSpecs,
                    ppPkgInfo,
                    pdwCount);
}

uint32_t
TDNFCliInvokeProvides(
    PTDNF_CLI_CONTEXT pContext,
    const char *pszProvides,
    PTDNF_PKG_INFO *ppPkgInfos
    )
{
    return TDNFProvides(pContext->hTdnf, pszProvides, ppPkgInfos);
}

uint32_t
TDNFCliInvokeRepoList(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_REPOLISTFILTER nFilter,
    PTDNF_REPO_DATA *ppRepos
    )
{
    return TDNFRepoList(pContext->hTdnf, nFilter, ppRepos);
}

uint32_t
TDNFCliInvokeRepoSync(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_REPOSYNC_ARGS pRepoSyncArgs
    )
{
    return TDNFRepoSync(pContext->hTdnf, pRepoSyncArgs);
}

uint32_t
TDNFCliInvokeRepoQuery(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_REPOQUERY_ARGS pRepoQueryArgs,
    PTDNF_PKG_INFO *ppPkgInfos,
    uint32_t *pdwCount
    )
{
    return TDNFRepoQuery(pContext->hTdnf, pRepoQueryArgs, ppPkgInfos, pdwCount);
}

uint32_t
TDNFCliInvokeResolve(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_ALTERTYPE nAlterType,
    PTDNF_SOLVED_PKG_INFO *ppSolvedPkgInfo
    )
{
    return TDNFResolve(pContext->hTdnf, nAlterType, ppSolvedPkgInfo);
}

uint32_t
TDNFCliInvokeSearch(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_PKG_INFO *ppPkgInfo,
    uint32_t *pdwCount
    )
{
    return TDNFSearchCommand(pContext->hTdnf, pCmdArgs, ppPkgInfo, pdwCount);
}

uint32_t
TDNFCliInvokeUpdateInfo(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_UPDATEINFO_ARGS pInfoArgs,
    PTDNF_UPDATEINFO *ppUpdateInfo
    )
{
    return TDNFUpdateInfo(
               pContext->hTdnf,
               pInfoArgs->ppszPackageNameSpecs,
               ppUpdateInfo);
}

uint32_t
TDNFCliInvokeUpdateInfoSummary(
    PTDNF_CLI_CONTEXT pContext,
    TDNF_AVAIL nAvail,
    PTDNF_UPDATEINFO_ARGS pInfoArgs,
    PTDNF_UPDATEINFO_SUMMARY *ppSummary
    )
{
    UNUSED(nAvail);

    return TDNFUpdateInfoSummary(
               pContext->hTdnf,
               pInfoArgs->ppszPackageNameSpecs,
               ppSummary);
}

