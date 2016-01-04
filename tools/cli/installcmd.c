/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : installcmd.c
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

uint32_t
TDNFCliInstallCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    
    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, ALTER_INSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliEraseCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    
    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, ALTER_ERASE);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliUpgradeCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_UPGRADE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_UPGRADEALL;
    }

    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliDistroSyncCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, ALTER_DISTRO_SYNC);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliDowngradeCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_DOWNGRADE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_DOWNGRADEALL;
    }
    
    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliAutoEraseCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    
    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, ALTER_AUTOERASE);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliReinstallCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    
    dwError = TDNFCliAlterCommand(pTdnf, pCmdArgs, ALTER_REINSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliAlterCommand(
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;
    char chChoice = 'n';
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;

    if(!pTdnf)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParsePackageArgs(
                  pCmdArgs,
                  &ppszPackageArgs,
                  &nPackageCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFResolve(pTdnf, nAlterType, &pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

    if(pSolvedPkgInfo->ppszPkgsNotResolved)
    {
        dwError = PrintNotAvailable(pSolvedPkgInfo->ppszPkgsNotResolved);
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(!pSolvedPkgInfo->nNeedAction)
    {
        dwError = ERROR_TDNF_CLI_NOTHING_TO_DO;
        //If there are unresolved, error with no match
        if(pSolvedPkgInfo->ppszPkgsNotResolved)
        {
            dwError = ERROR_TDNF_NO_MATCH;
        }
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pSolvedPkgInfo->pPkgsExisting)
    {
        dwError = PrintExistingPackagesSkipped(pSolvedPkgInfo->pPkgsExisting);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsNotAvailable)
    {
        dwError = PrintNotAvailablePackages(pSolvedPkgInfo->pPkgsNotAvailable);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToInstall)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToInstall, ALTER_INSTALL);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToUpgrade)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToUpgrade, ALTER_UPGRADE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToDowngrade)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToDowngrade, ALTER_DOWNGRADE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToRemove, ALTER_ERASE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsUnNeeded)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsUnNeeded, ALTER_ERASE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToReinstall)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToReinstall, ALTER_REINSTALL);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsObsoleted)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsObsoleted, ALTER_OBSOLETED);
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pSolvedPkgInfo->nNeedAction)
    {
        if(!pCmdArgs->nAssumeYes && !pCmdArgs->nAssumeNo)
        {
            printf("Is this ok [y/N]:");
            if (scanf("%c", &chChoice) != 1)
            {
                printf("Invalid input\n");
            }
        }

        if(pCmdArgs->nAssumeYes || chChoice == 'y')
        {
            if(pSolvedPkgInfo->nNeedDownload)
            {
                fprintf(stdout, "\nDownloading:\n");
            }
            dwError = TDNFAlterCommand(pTdnf, nAlterType, pSolvedPkgInfo);
            BAIL_ON_CLI_ERROR(dwError);

            fprintf(stdout, "\nComplete!\n");
        }
        else
        {
            printf("Exiting on user Command\n");
        }
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    TDNFFreeSolvedPackageInfo(pSolvedPkgInfo);
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintNotAvailable(
    char** ppszPkgsNotAvailable
    )
{
    uint32_t dwError = 0;
    int i = 0;
    #define BOLD "\033[1m\033[30m"
    #define RESET   "\033[0m"

    if(!ppszPkgsNotAvailable)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while(ppszPkgsNotAvailable[i])
    {
        printf(
            "No package " BOLD "%s " RESET "available\n",
            ppszPkgsNotAvailable[i]);
        ++i;
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
PrintExistingPackagesSkipped(
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        printf(
            "Package %s-%s-%s.%s is already installed, skipping.\n",
            pPkgInfo->pszName,
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease,
            pPkgInfo->pszArch);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintNotAvailablePackages(
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        printf(
            "No package %s available.\n",
            pPkgInfo->pszName);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
PrintAction(
    PTDNF_PKG_INFO pPkgInfos,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    #define COL_COUNT 4
    //Name | Arch | Version-Release | Install Size
    int nColPercents[COL_COUNT] = {40, 15, 25, 10};
    int nColWidths[COL_COUNT] = {0};

    #define MAX_COL_LEN 256
    char szVersionAndRelease[MAX_COL_LEN] = {0};

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    switch(nAlterType)
    {
        case ALTER_INSTALL:
            printf("\nInstalling:");
            break;
        case ALTER_UPGRADE:
            printf("\nUpgrading:");
            break;
        case ALTER_ERASE:
            printf("\nRemoving:");
            break;
        case ALTER_DOWNGRADE:
            printf("\nDowngrading:");
            break;
        case ALTER_REINSTALL:
            printf("\nReinstalling:");
            break;
        case ALTER_OBSOLETED:
            printf("\nObsoleting:");
            break;
        default:
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_CLI_ERROR(dwError);
    }
    printf("\n");

    dwError = GetColumnWidths(COL_COUNT, nColPercents, nColWidths);
    BAIL_ON_CLI_ERROR(dwError);

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        memset(szVersionAndRelease, 0, MAX_COL_LEN);
        if(snprintf(
            szVersionAndRelease,
            MAX_COL_LEN,
            "%s-%s",
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease) < 0)
        {
            dwError = errno;
            BAIL_ON_CLI_ERROR(dwError);
        }

        printf(
            "%-*s%-*s%-*s%*s\n",
            nColWidths[0],
            pPkgInfo->pszName,
            nColWidths[1],
            pPkgInfo->pszArch,
            nColWidths[2],
            szVersionAndRelease,
            nColWidths[3],
            pPkgInfo->pszFormattedSize);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
