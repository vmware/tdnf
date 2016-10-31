/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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
    PTDNF_SOLVED_PKG_INFO pSolvedInfo,
    const char* pszPkgName
    )
{
    uint32_t dwError = 0 ;
    int nIndex = 0;

    if(!pSolvedInfo ||
       !pSolvedInfo->ppszPkgsNotResolved ||
       IsNullOrEmptyString(pszPkgName))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    while(pSolvedInfo->ppszPkgsNotResolved[nIndex++]);

    dwError = TDNFAllocateString(
                  pszPkgName,
                  &pSolvedInfo->ppszPkgsNotResolved[--nIndex]);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

