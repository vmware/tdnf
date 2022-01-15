/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

uint32_t
TDNFUpdateInfoSummary(
    PTDNF pTdnf,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO_SUMMARY* ppSummary
    )
{
    uint32_t dwError = 0;
    uint32_t nCount = 0;
    int iAdv = 0;
    uint32_t dwPkgIndex = 0;
    uint32_t dwSize = 0;
    PSolvPackageList pInstalledPkgList = NULL;
    PSolvPackageList pUpdateAdvPkgList = NULL;
    Id dwAdvId = 0;
    Id dwPkgId = 0;
    uint32_t nType = 0;
    PTDNF_UPDATEINFO_SUMMARY pSummary = NULL;
    const char *pszType = 0;
    char *pszSeverity = NULL;
    uint32_t dwSecurity = 0;
    const char* pszTemp = NULL;

    if(!pTdnf || !pTdnf->pSack || !pTdnf->pSack->pPool ||
       !ppSummary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRefresh(pTdnf);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!ppszPackageNameSpecs)
    {
        dwError = SolvFindAllInstalled(pTdnf->pSack, &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = SolvFindInstalledPkgByMultipleNames(
                      pTdnf->pSack,
                      ppszPackageNameSpecs,
                      &pInstalledPkgList);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = SolvGetPackageListSize(pInstalledPkgList, &dwSize);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateMemory(
                  UPDATE_ENHANCEMENT + 1,
                  sizeof(TDNF_UPDATEINFO_SUMMARY),
                  (void**)&pSummary);
    BAIL_ON_TDNF_ERROR(dwError);

    pSummary[UPDATE_UNKNOWN].nType = UPDATE_UNKNOWN;
    pSummary[UPDATE_SECURITY].nType = UPDATE_SECURITY;
    pSummary[UPDATE_BUGFIX].nType = UPDATE_BUGFIX;
    pSummary[UPDATE_ENHANCEMENT].nType = UPDATE_ENHANCEMENT;

    dwError = TDNFGetSecuritySeverityOption(
                  pTdnf,
                  &dwSecurity,
                  &pszSeverity);
    BAIL_ON_TDNF_ERROR(dwError);

    for(dwPkgIndex = 0; dwPkgIndex < dwSize; dwPkgIndex++)
    {
        dwError = SolvGetPackageId(pInstalledPkgList, dwPkgIndex, &dwPkgId);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetUpdateAdvisories(
                      pTdnf->pSack,
                      dwPkgId,
                      &pUpdateAdvPkgList);
        //Ignore no data and continue.
        if(dwError == ERROR_TDNF_NO_DATA)
        {
            dwError = 0;
            continue;
        }
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = SolvGetPackageListSize(pUpdateAdvPkgList, &nCount);
        BAIL_ON_TDNF_ERROR(dwError);

        for(iAdv = 0; (uint32_t)iAdv < nCount; iAdv++)
        {
            dwError = SolvGetPackageId(pUpdateAdvPkgList, iAdv, &dwAdvId);
            BAIL_ON_TDNF_ERROR(dwError);

            pszType = pool_lookup_str(
                          pTdnf->pSack->pPool,
                          dwAdvId,
                          SOLVABLE_PATCHCATEGORY);
            nType = UPDATE_UNKNOWN;
            if (pszType == NULL)
                nType = UPDATE_UNKNOWN;
            else if (!strcmp (pszType, "bugfix"))
                nType = UPDATE_BUGFIX;
            else if (!strcmp (pszType, "enhancement"))
                nType = UPDATE_ENHANCEMENT;
            else if (!strcmp (pszType, "security"))
                nType = UPDATE_SECURITY;
            if (dwSecurity)
            {
                if (nType != UPDATE_SECURITY)
                    continue;
            }
            else if (pszSeverity)
            {
                pszTemp = pool_lookup_str(
                              pTdnf->pSack->pPool,
                              dwAdvId,
                              UPDATE_SEVERITY);
                if (!pszTemp || atof(pszSeverity) > atof(pszTemp))
                    continue;
            }
            pSummary[nType].nCount++;
        }
        SolvFreePackageList(pUpdateAdvPkgList);
        pUpdateAdvPkgList = NULL;
    }
    *ppSummary = pSummary;

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszSeverity);
    if(pInstalledPkgList)
    {
        SolvFreePackageList(pInstalledPkgList);
    }
    if(pUpdateAdvPkgList)
    {
        SolvFreePackageList(pUpdateAdvPkgList);
    }
    return dwError;

error:
    if(ppSummary)
    {
        *ppSummary = NULL;
    }
    TDNFFreeUpdateInfoSummary(pSummary);
    goto cleanup;
}

uint32_t
TDNFGetUpdateInfoPackages(
    PSolvSack pSack,
    Id dwPkgId,
    PTDNF_UPDATEINFO_PKG* ppUpdateInfoPkg
    )
{
    uint32_t dwError = 0;
    Dataiterator di = {0};
    PTDNF_UPDATEINFO_PKG pPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    const char* pszTemp = NULL;


    if(!pSack || !pSack->pPool || !ppUpdateInfoPkg)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dataiterator_init(&di, pSack->pPool, 0, dwPkgId, UPDATE_COLLECTION, 0, 0);
    while (dataiterator_step(&di))
    {
        dataiterator_setpos(&di);

        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_UPDATEINFO_PKG),
                      (void**)&pPkg);
        BAIL_ON_TDNF_ERROR(dwError);

        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_NAME);

        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszName);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_EVR);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszEVR);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_ARCH);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszArch);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      SOLVID_POS,
                      UPDATE_COLLECTION_FILENAME);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pPkg->pszFileName);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        pPkg->pNext = pPkgs;
        pPkgs = pPkg;
        pPkg = NULL;


    }

    *ppUpdateInfoPkg = pPkgs;

cleanup:
    dataiterator_free(&di);
    return dwError;

error:
    if(ppUpdateInfoPkg)
    {
        *ppUpdateInfoPkg = NULL;
    }
    if(pPkg)
    {
        TDNFFreeUpdateInfoPackages(pPkg);
    }
    if(pPkgs)
    {
        TDNFFreeUpdateInfoPackages(pPkgs);
    }

    goto cleanup;
}

uint32_t
TDNFPopulateUpdateInfoOfOneAdvisory(
    PSolvSack pSack,
    Id dwAdvId,
    uint32_t dwSecurity,
    const char*  pszSeverity,
    uint32_t dwRebootRequired,
    PTDNF_UPDATEINFO* ppInfo)
{
    uint32_t dwError = 0;

    time_t dwUpdated = 0;
    const char *pszType = 0;
    PTDNF_UPDATEINFO pInfo = NULL;
    const char* pszTemp = NULL;
    uint32_t dwKeepEntry = 1;
    const int DATELEN = 200;
    char szDate[DATELEN];
    int dwReboot = 0;

    struct tm* pLocalTime = NULL;

    if(!pSack || !pSack->pPool || !ppInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }


    pszType = pool_lookup_str(pSack->pPool,
                              dwAdvId,
                              SOLVABLE_PATCHCATEGORY);
    pszTemp = pool_lookup_str(
                  pSack->pPool,
                  dwAdvId,
                  UPDATE_SEVERITY);
    dwReboot = pool_lookup_void(
                       pSack->pPool,
                       dwAdvId,
                       UPDATE_REBOOT);
    if (dwSecurity)
    {
        if (strcmp (pszType, "security"))
        {
            dwKeepEntry = 0;
        }
    }
    else if (pszSeverity)
    {
         if(!pszTemp || atof(pszSeverity) > atof(pszTemp))
         {
             dwKeepEntry = 0;
         }
    }
    if (dwRebootRequired)
    {
         if(dwReboot == 0)
         {
            dwKeepEntry = 0;
         }
    }

    if (dwKeepEntry)
    {
        dwError = TDNFAllocateMemory(
                      1,
                      sizeof(TDNF_UPDATEINFO),
                      (void**)&pInfo);
        BAIL_ON_TDNF_ERROR(dwError);

        pInfo->nType = UPDATE_UNKNOWN;
        if (pszType == NULL)
            pInfo->nType = UPDATE_UNKNOWN;
        else if (!strcmp (pszType, "bugfix"))
            pInfo->nType = UPDATE_BUGFIX;
        else if (!strcmp (pszType, "enhancement"))
            pInfo->nType = UPDATE_ENHANCEMENT;
        else if (!strcmp (pszType, "security"))
            pInfo->nType = UPDATE_SECURITY;

        pInfo->nRebootRequired = dwReboot;

        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      dwAdvId,
                      SOLVABLE_NAME);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pInfo->pszID);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        pszTemp = pool_lookup_str(
                      pSack->pPool,
                      dwAdvId,
                      SOLVABLE_DESCRIPTION);
        if(pszTemp)
        {
            dwError = TDNFAllocateString(pszTemp, &pInfo->pszDescription);
            BAIL_ON_TDNF_ERROR(dwError);
        }

        dwUpdated = pool_lookup_num(
                        pSack->pPool,
                        dwAdvId,
                        SOLVABLE_BUILDTIME,
                        0);
        if(dwUpdated > 0)
        {
            pLocalTime = localtime(&dwUpdated);
            if(!pLocalTime)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            memset(szDate, 0, DATELEN);
            dwError = strftime(szDate, DATELEN, "%c", pLocalTime);
            if(dwError == 0)
            {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
            dwError = TDNFAllocateString(szDate, &pInfo->pszDate);
            BAIL_ON_TDNF_ERROR(dwError);
        }


        dwError = TDNFGetUpdateInfoPackages(pSack, dwAdvId, &pInfo->pPackages);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppInfo = pInfo;

cleanup:
    return dwError;

error:
    if(ppInfo)
    {
        *ppInfo = NULL;
    }
    if(pInfo)
    {
        TDNFFreeUpdateInfo(pInfo);
    }
    goto cleanup;
}

uint32_t
TDNFGetSecuritySeverityOption(
    PTDNF pTdnf,
    uint32_t *pdwSecurity,
    char **ppszSeverity
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    uint32_t dwSecurity = 0;
    char* pszSeverity = NULL;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
           !strcasecmp(pSetOpt->pszOptName, "sec-severity"))
        {
            dwError = TDNFAllocateString(pSetOpt->pszOptValue, &pszSeverity);
            BAIL_ON_TDNF_ERROR(dwError);
        }
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
           !strcasecmp(pSetOpt->pszOptName, "security"))
        {
            dwSecurity = 1;
        }

         pSetOpt = pSetOpt->pNext;
    }

    *pdwSecurity = dwSecurity;
    *ppszSeverity = pszSeverity;
cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszSeverity);
    if(ppszSeverity)
    {
        *ppszSeverity = NULL;
    }
    if(pdwSecurity)
    {
        *pdwSecurity = 0;
    }
    goto cleanup;
}

uint32_t
TDNFNumUpdatePkgs(
    PTDNF_UPDATEINFO pInfo,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    if(!pInfo || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    while(pInfo)
    {
        pPkg = pInfo->pPackages;
        while(pPkg)
        {
            dwCount++;
            pPkg = pPkg->pNext;
        }
        pInfo = pInfo->pNext;
    }
    *pdwCount = dwCount;
cleanup:
    return dwError;

error:
    if(pdwCount)
    {
        *pdwCount = 0;
    }
    goto cleanup;
}

uint32_t
TDNFGetUpdatePkgs(
    PTDNF pTdnf,
    char*** pppszPkgs,
    uint32_t *pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    char**   ppszPkgs = NULL;
    PTDNF_UPDATEINFO_PKG pPkg = NULL;
    int nIndex = 0;
    char* pszPkgName = NULL;

    PTDNF_UPDATEINFO pUpdateInfo = NULL;
    PTDNF_UPDATEINFO pInfo = NULL;

    if(!pTdnf || !pdwCount || !pppszPkgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFUpdateInfo(pTdnf, &pszPkgName, &pUpdateInfo);
    BAIL_ON_TDNF_ERROR(dwError);

    pInfo = pUpdateInfo;
    dwError = TDNFNumUpdatePkgs(pInfo, &dwCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if(dwCount == 0)
    {
        goto cleanup;
    }

    dwError = TDNFAllocateMemory(
                  dwCount + 1,
                  sizeof(char*),
                  (void**)&ppszPkgs);
    BAIL_ON_TDNF_ERROR(dwError);

    for(pInfo = pUpdateInfo; pInfo; pInfo = pInfo->pNext)
    {
        pPkg = pInfo->pPackages;
        while(pPkg)
        {
            dwError = TDNFAllocateString(
                          pPkg->pszName,
                          &ppszPkgs[nIndex++]);
            BAIL_ON_TDNF_ERROR(dwError);
            pPkg = pPkg->pNext;
        }

    }
    *pppszPkgs = ppszPkgs;
    *pdwCount  = dwCount;
cleanup:
    if(pUpdateInfo)
    {
        TDNFFreeUpdateInfo(pUpdateInfo);
    }
    return dwError;

error:
    if(pppszPkgs)
    {
        *pppszPkgs = NULL;
    }
    if(ppszPkgs)
    {
        TDNFFreeStringArray(ppszPkgs);
    }
    goto cleanup;
}

uint32_t
TDNFGetRebootRequiredOption(
    PTDNF pTdnf,
    uint32_t *pdwRebootRequired
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pSetOpt = NULL;
    uint32_t dwRebootRequired = 0;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pSetOpt = pTdnf->pArgs->pSetOpt;

    while(pSetOpt)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE &&
          !strcasecmp(pSetOpt->pszOptName, "reboot-required"))
        {
            dwRebootRequired = 1;
            break;
        }
        pSetOpt = pSetOpt->pNext;
    }
    *pdwRebootRequired = dwRebootRequired;
cleanup:
    return dwError;

error:
    if(pdwRebootRequired)
    {
       *pdwRebootRequired = 0;
    }
    goto cleanup;
}
