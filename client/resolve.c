/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : resolve.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 */

#include "includes.h"

uint32_t
TDNFAddNotResolved(
    char** ppszPkgsNotResolved,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0 ;
    int nIndex = 0;

    if(!ppszPkgsNotResolved ||
       IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while(ppszPkgsNotResolved[nIndex++]);

    dwError = TDNFAllocateString(
                  pszPkgName,
                  &ppszPkgsNotResolved[--nIndex]);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPrepareAllPackages(
    PTDNF pTdnf,
    TDNF_ALTERTYPE* pAlterType,
    char** ppszPkgsNotResolved,
    Queue* queueGoal
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    int nPkgIndex = 0;
    int nIsFile = 0;
    int nDummy = 0;
    char* pszPkgName = NULL;
    char* pszName = NULL;
    Queue queueLocal = {0};
    char*  pszSeverity = NULL;
    uint32_t dwSecurity = 0;
    char** ppszPkgArray = NULL;
    uint32_t dwCount = 0;
    uint32_t dwRebootRequired = 0;
    TDNF_ALTERTYPE nAlterType = 0;

    if(!pTdnf || !pTdnf->pSack ||
       !pTdnf->pArgs || !ppszPkgsNotResolved || !queueGoal || !pAlterType)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    queue_init(&queueLocal);
    pCmdArgs = pTdnf->pArgs;
    nAlterType = *pAlterType;

    if(nAlterType == ALTER_DOWNGRADEALL)
    {
        dwError = TDNFFilterPackages(
                       pTdnf,
                       nAlterType,
                       ppszPkgsNotResolved,
                       queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFGetSecuritySeverityOption(
                  pTdnf,
                  &dwSecurity,
                  &pszSeverity);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFGetRebootRequiredOption(
                  pTdnf,
                  &dwRebootRequired);
    BAIL_ON_TDNF_ERROR(dwError);

    if ((nAlterType == ALTER_UPGRADEALL ||
         nAlterType == ALTER_UPGRADE) &&
        (dwSecurity || pszSeverity || dwRebootRequired))
    {
        //pAlterType is changed to ALTER_UPGRADE and later used in TDNFGoal() to add exclude the
        // list of packages that are added in --exclude option.
        *pAlterType = ALTER_UPGRADE;
        dwError = TDNFGetUpdatePkgs(pTdnf, &ppszPkgArray, &dwCount);
        BAIL_ON_TDNF_ERROR(dwError);
        for(nPkgIndex = 0; (uint32_t)nPkgIndex < dwCount; ++nPkgIndex)
        {
            dwError = TDNFPrepareAndAddPkg(
                          pTdnf,
                          ppszPkgArray[nPkgIndex],
                          *pAlterType,
                          ppszPkgsNotResolved,
                          queueGoal);
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }
    else
    {
       for(int nCmdIndex = 1; nCmdIndex < pCmdArgs->nCmdCount; ++nCmdIndex)
       {
           pszPkgName = pCmdArgs->ppszCmds[nCmdIndex];

           if(TDNFIsGlob(pszPkgName))
           {
               queue_empty(&queueLocal);
               dwError = TDNFGetGlobPackages(
                             pTdnf->pSack,
                             pszPkgName,
                             &queueLocal);
               BAIL_ON_TDNF_ERROR(dwError);
               if(queueLocal.count == 0)
               {
                   dwError = TDNFAddNotResolved(ppszPkgsNotResolved, pszPkgName);
                   BAIL_ON_TDNF_ERROR(dwError);
               }
               else
               {
                   nPkgIndex = 0;
                   for(nPkgIndex = 0; nPkgIndex < queueLocal.count; nPkgIndex++)
                   {
                       dwError = SolvGetPkgNameFromId(
                                     pTdnf->pSack,
                                     queueLocal.elements[nPkgIndex],
                                     &pszName);
                       BAIL_ON_TDNF_ERROR(dwError);

                       dwError = TDNFPrepareAndAddPkg(
                                     pTdnf,
                                     pszName,
                                     nAlterType,
                                     ppszPkgsNotResolved,
                                     queueGoal);
                       BAIL_ON_TDNF_ERROR(dwError);
                       TDNF_SAFE_FREE_MEMORY(pszName);
                       pszName = NULL;
                   }
               }
           }
           else
           {
               dwError = TDNFIsFileOrSymlink(pszPkgName, &nIsFile);
               BAIL_ON_TDNF_ERROR(dwError);

               if (nIsFile && (fnmatch("*.rpm", pszPkgName, 0) == 0))
               {
                   continue;
               }

               dwError = TDNFUriIsRemote(pszPkgName, &nDummy);
               if (dwError == 0)
               {
                 /* URL => cmd line pkg, already handled */
                 dwError = 0;
                 continue;
               }
               else if (dwError == ERROR_TDNF_URL_INVALID)
               {
                   dwError = 0;
               }
               BAIL_ON_TDNF_ERROR(dwError);

               dwError = TDNFPrepareAndAddPkg(
                             pTdnf,
                             pszPkgName,
                             nAlterType,
                             ppszPkgsNotResolved,
                             queueGoal);
               BAIL_ON_TDNF_ERROR(dwError);
           }
       }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszSeverity);
    TDNF_SAFE_FREE_MEMORY(pszName);
    queue_free(&queueLocal);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFFilterPackages(
    PTDNF pTdnf,
    TDNF_ALTERTYPE nAlterType,
    char** ppszPkgsNotResolved,
    Queue* queueGoal)
{
    uint32_t dwError = 0;
    Id dwInstalledId = 0;
    uint32_t dwPkgIndex = 0;
    uint32_t dwSize = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    char* pszName = NULL;
    PSolvSack pSack = NULL;

    if(!pTdnf || !pTdnf->pSack || !queueGoal || !ppszPkgsNotResolved)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSack = pTdnf->pSack;

    dwError = SolvFindAllInstalled(pSack, &pInstalledPkgList);
    if(dwError == ERROR_TDNF_NO_MATCH)
    {
        dwError = 0;
    }

    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwSize);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwSize; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &dwInstalledId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPkgNameFromId(pSack,
                      dwInstalledId,
                      &pszName);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFPrepareAndAddPkg(
                      pTdnf,
                      pszName,
                      nAlterType,
                      ppszPkgsNotResolved,
                      queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
        TDNF_SAFE_FREE_MEMORY(pszName);
        pszName = NULL;
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszName);
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFPrepareAndAddPkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    TDNF_ALTERTYPE nAlterType,
    char** ppszPkgsNotResolved,
    Queue* queueGoal
    )
{
    uint32_t dwError = 0;
    if( !pTdnf ||
        IsNullOrEmptyString(pszPkgName) ||
        !ppszPkgsNotResolved ||
        !queueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPrepareSinglePkg(
                  pTdnf,
                  pszPkgName,
                  nAlterType,
                  ppszPkgsNotResolved,
                  queueGoal);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFPrepareSinglePkg(
    PTDNF pTdnf,
    const char* pszPkgName,
    TDNF_ALTERTYPE nAlterType,
    char** ppszPkgsNotResolved,
    Queue* queueGoal
    )
{
    uint32_t dwError = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    uint32_t dwCount = 0;
    PSolvSack pSack = NULL;

    if(!pTdnf ||
       !pTdnf->pSack ||
       !ppszPkgsNotResolved ||
       IsNullOrEmptyString(pszPkgName) ||
       !queueGoal)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSack = pTdnf->pSack;

    //Check if this is a known package. If not add to unresolved
    dwError = SolvCountPkgByName(pSack, pszPkgName, &dwCount);
    if (dwError == ERROR_TDNF_NO_MATCH)
    {
        pr_err("%s package not found or not installed\n", pszPkgName);
    }

    BAIL_ON_TDNF_ERROR(dwError);
    if (dwCount == 0)
    {
        dwError = ERROR_TDNF_NO_SEARCH_RESULTS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_REINSTALL)
    {
        dwError = TDNFMatchForReinstall(
                      pSack,
                      pszPkgName,
                      queueGoal);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(nAlterType == ALTER_ERASE ||
        nAlterType == ALTER_AUTOERASE)
    {
        dwError = SolvFindInstalledPkgByName(
                      pSack,
                      pszPkgName,
                      &pInstalledPkgList);
        if(dwError == ERROR_TDNF_NO_MATCH)
        {
            dwError = ERROR_TDNF_ERASE_NEEDS_INSTALL;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAddPackagesForErase(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_INSTALL)
    {
        dwError = TDNFAddPackagesForInstall(
                      pSack,
                      queueGoal,
                      pszPkgName);
        if (dwError == ERROR_TDNF_ALREADY_INSTALLED)
        {
            /* the package may have been already installed as a dependency,
               but now the user wants it on its own */
            dwError = TDNFMarkAutoInstalledSinglePkg(pTdnf, pszPkgName);
            BAIL_ON_TDNF_ERROR(dwError);
            /* if TDNFMarkAutoInstalledSinglePkg() was successful, restore
               the original error */
            dwError = ERROR_TDNF_ALREADY_INSTALLED;
        }
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_UPGRADE)
    {
        dwError = TDNFAddPackagesForUpgrade(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else if (nAlterType == ALTER_DOWNGRADE ||
             nAlterType == ALTER_DOWNGRADEALL)
    {
        dwError = TDNFAddPackagesForDowngrade(
                      pSack,
                      queueGoal,
                      pszPkgName);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    return dwError;

error:
    if(dwError == ERROR_TDNF_ALREADY_INSTALLED)
    {
        int nShowAlreadyInstalled = 1;
        //dont show already installed errors in the check path
        if(pTdnf && pTdnf->pArgs)
        {
            if(!strcmp(pTdnf->pArgs->ppszCmds[0], "check"))
            {
                nShowAlreadyInstalled = 0;
            }
        }
        dwError = 0;
        if(nShowAlreadyInstalled)
        {
            pr_err("Package %s is already installed.\n", pszPkgName);
        }
    }
    if(dwError == ERROR_TDNF_NO_UPGRADE_PATH)
    {
        dwError = 0;
        pr_err("There is no upgrade path for %s.\n", pszPkgName);
    }
    if(dwError == ERROR_TDNF_NO_DOWNGRADE_PATH)
    {
        dwError = 0;
        pr_err("There is no downgrade path for %s.\n", pszPkgName);
    }
    if(dwError == ERROR_TDNF_NO_SEARCH_RESULTS)
    {
        dwError = 0;
        if(TDNFAddNotResolved(ppszPkgsNotResolved, pszPkgName))
        {
            pr_err("Error while adding not resolved packages\n");
        }
    }
    if(dwError == ERROR_TDNF_ERASE_NEEDS_INSTALL)
    {
        dwError = 0;
//TODO: maybe restore solvedinfo based processing here.
    }
    goto cleanup;
}
