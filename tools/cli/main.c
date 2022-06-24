/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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

static TDNF_CLI_CMD_MAP arCmdMap[] =
{
    {"autoerase",          TDNFCliAutoEraseCommand, true},
    {"autoremove",         TDNFCliAutoEraseCommand, true},
    {"check",              TDNFCliCheckCommand, false},
    {"check-local",        TDNFCliCheckLocalCommand, false},
    {"check-update",       TDNFCliCheckUpdateCommand, false},
    {"clean",              TDNFCliCleanCommand, true},
    {"count",              TDNFCliCountCommand, false},
    {"distro-sync",        TDNFCliDistroSyncCommand, true},
    {"downgrade",          TDNFCliDowngradeCommand, true},
    {"erase",              TDNFCliEraseCommand, true},
    {"help",               TDNFCliHelpCommand, false},
    {"info",               TDNFCliInfoCommand, false},
    {"install",            TDNFCliInstallCommand, true},
    {"list",               TDNFCliListCommand, false},
    {"makecache",          TDNFCliMakeCacheCommand, true},
    {"provides",           TDNFCliProvidesCommand, false},
    {"whatprovides",       TDNFCliProvidesCommand, false},
    {"reinstall",          TDNFCliReinstallCommand, true},
    {"remove",             TDNFCliEraseCommand, true},
    {"repolist",           TDNFCliRepoListCommand, false},
    {"reposync",           TDNFCliRepoSyncCommand, false},
    {"repoquery",          TDNFCliRepoQueryCommand, false},
    {"search",             TDNFCliSearchCommand, false},
    {"update",             TDNFCliUpgradeCommand, true},
    {"update-to",          TDNFCliUpgradeCommand, true},
    {"upgrade",            TDNFCliUpgradeCommand, true},
    {"upgrade-to",         TDNFCliUpgradeCommand, true},
    {"updateinfo",         TDNFCliUpdateInfoCommand, false},
};

int main(int argc, char **argv)
{
    uint32_t dwError = 0;
    PTDNF pTdnf = NULL;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    TDNF_CLI_CMD_MAP *pCmd = NULL;

    dwError = TDNFCliParseArgs(argc, argv, &pCmdArgs);
    BAIL_ON_CLI_ERROR(dwError);

    if(pCmdArgs->nShowVersion)
    {
        TDNFCliShowVersion(pCmdArgs);
    }
    else if(pCmdArgs->nShowHelp)
    {
        TDNFCliShowHelp();
    }
    else if(pCmdArgs->nCmdCount > 0)
    {
        const char *pszCmd = NULL;
        TDNF_CLI_CONTEXT _context;

        memset(&_context, 0, sizeof(TDNF_CLI_CONTEXT));

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

        pszCmd = pCmdArgs->ppszCmds[0];

        for (int i = 0; i < (int)ARRAY_SIZE(arCmdMap); i++)
        {
            if (strcmp(pszCmd, arCmdMap[i].pszCmdName) == 0)
            {
                pCmd = &arCmdMap[i];
            }
        }

        if (pCmd)
        {
            if (pCmd->ReqRoot && geteuid())
            {
                dwError = ERROR_TDNF_PERM;
                BAIL_ON_CLI_ERROR(dwError);
            }

            if (!strcmp(pszCmd, "makecache"))
            {
                pCmdArgs->nRefresh = 1;
            }

            dwError = TDNFInit();
            BAIL_ON_CLI_ERROR(dwError);

            dwError = TDNFOpenHandle(pCmdArgs, &pTdnf);
            BAIL_ON_CLI_ERROR(dwError);

            _context.hTdnf = pTdnf;

            if (pCmdArgs->nVerbose)
            {
                dwError = TDNFCliVerboseShowEnv(pCmdArgs);
                BAIL_ON_CLI_ERROR(dwError);
            }

            dwError = pCmd->pFnCmd(&_context, pCmdArgs);
            BAIL_ON_CLI_ERROR(dwError);
        }
        else
        {
            if (!pCmdArgs->nJsonOutput)
            {
                TDNFCliShowNoSuchCommand(pszCmd);
            }
            dwError = ERROR_TDNF_CLI_NO_SUCH_CMD;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }
    else
    {
        if (!pCmdArgs->nJsonOutput)
        {
            TDNFCliShowUsage();
        }
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
    TDNFCliPrintError(dwError, pCmdArgs ? pCmdArgs->nJsonOutput : 0);
    if (dwError == ERROR_TDNF_CLI_NOTHING_TO_DO ||
        dwError == ERROR_TDNF_NO_DATA)
    {
        // Nothing to do should not return an error code
        dwError = 0;
    }

    goto cleanup;
}

uint32_t
TDNFCliPrintError(
    uint32_t dwErrorCode,
    int doJson
    )
{
    uint32_t dwError = 0;
    char* pszError = NULL;
    struct json_dump *jd = NULL;

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

    if (doJson)
    {
        if (dwErrorCode)
        {
            jd = jd_create(0);
            CHECK_JD_NULL(jd);

            CHECK_JD_RC(jd_map_start(jd));
            CHECK_JD_RC(jd_map_add_int(jd, "Error", dwErrorCode));
            CHECK_JD_RC(jd_map_add_string(jd, "ErrorMessage", pszError));
            
            pr_json(jd->buf);
        }
    }
    else
    {
        if (dwErrorCode)
        {
            pr_err("Error(%u) : %s\n", dwErrorCode, pszError);
        }
        else
        {
            pr_err("%s\n", pszError);
        }
    }

cleanup:
    JD_SAFE_DESTROY(jd);
    TDNF_CLI_SAFE_FREE_MEMORY(pszError);
    return dwError;

error:
    pr_err("Retrieving error string for %u failed with %u\n",
            dwErrorCode, dwError);

    goto cleanup;
}

void
TDNFCliShowVersion(
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    if (pCmdArgs->nJsonOutput)
    {
        struct json_dump *jd = jd_create(0);
        jd_map_start(jd);
        jd_map_add_string(jd, "Name", TDNFGetPackageName());
        jd_map_add_string(jd, "Version", TDNFGetVersion());
        pr_json(jd->buf);
        jd_destroy(jd);
    }
    else
    {
        pr_info("%s: %s\n", TDNFGetPackageName(), TDNFGetVersion());
    }
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
    uint32_t nCleanType
    )
{
    return TDNFClean(pContext->hTdnf, nCleanType);
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

