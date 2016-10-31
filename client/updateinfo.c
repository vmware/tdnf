/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : updateinfo.c
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
TDNFUpdateInfoSummary(
    PTDNF pTdnf,
    TDNF_AVAIL nAvail,
    char** ppszPackageNameSpecs,
    PTDNF_UPDATEINFO_SUMMARY* ppSummary
    )
{
    return 1;
}

void
TDNFFreeUpdateInfoSummary(
    PTDNF_UPDATEINFO_SUMMARY pSummary
    )
{
    if(pSummary)
    {
        TDNFFreeMemory(pSummary);
    }
}

void
TDNFFreeUpdateInfo(
    PTDNF_UPDATEINFO pUpdateInfo
    )
{
    if(pUpdateInfo)
    {
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszID);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszDate);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo->pszDescription);

        TDNFFreeUpdateInfoReferences(pUpdateInfo->pReferences);
        TDNFFreeUpdateInfoPackages(pUpdateInfo->pPackages);
        TDNF_SAFE_FREE_MEMORY(pUpdateInfo);
    }
}

void
TDNFFreeUpdateInfoReferences(
    PTDNF_UPDATEINFO_REF pRef
    )
{
    if(pRef)
    {
        TDNF_SAFE_FREE_MEMORY(pRef->pszID);
        TDNF_SAFE_FREE_MEMORY(pRef->pszLink);
        TDNF_SAFE_FREE_MEMORY(pRef->pszTitle);
        TDNF_SAFE_FREE_MEMORY(pRef->pszType);
    }
}

uint32_t
TDNFGetUpdateInfoPackages(
    PSolvAdvisory hAdv,
    PTDNF_UPDATEINFO_PKG* ppPkgs
    )
{
    return 1;
}

void
TDNFFreeUpdateInfoPackages(
    PTDNF_UPDATEINFO_PKG pPkgs
    )
{
    PTDNF_UPDATEINFO_PKG pTemp = NULL;
    while(pPkgs)
    {
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszName);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszFileName);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszEVR);
        TDNF_SAFE_FREE_MEMORY(pPkgs->pszArch);

        pTemp = pPkgs;
        pPkgs = pPkgs->pNext;
        
        TDNF_SAFE_FREE_MEMORY(pTemp);
    }
}
