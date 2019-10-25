/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : setopt.c
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

void
TDNFFreeCmdOpt(
    PTDNF_CMD_OPT pCmdOpt
    );

uint32_t
AddSetOpt(
    PTDNF_CMD_ARGS pCmdArgs,
    const char* pszOptArg
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pCmdOpt = NULL;

    if(!pCmdArgs || IsNullOrEmptyString(pszOptArg))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    dwError = GetOptionAndValue(pszOptArg, &pCmdOpt);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!strcmp(pCmdOpt->pszOptName, "tdnf.conf"))
    {
        TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszConfFile);
        dwError = TDNFAllocateString(
                      pCmdOpt->pszOptValue,
                      &pCmdArgs->pszConfFile);
        BAIL_ON_TDNF_ERROR(dwError);
    }

cleanup:
    if(pCmdOpt)
    {
        TDNFFreeCmdOpt(pCmdOpt);
    }
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pCmdArgs->pszConfFile);
    goto cleanup;
}

uint32_t
AddSetOptWithValues(
    PTDNF_CMD_ARGS pCmdArgs,
    int nType,
    const char* pszOptArg,
    const char* pszOptValue
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pCmdOpt = NULL;
    PTDNF_CMD_OPT pSetOptTemp = NULL;

    if(!pCmdArgs ||
       IsNullOrEmptyString(pszOptArg) ||
       IsNullOrEmptyString(pszOptValue) || nType == CMDOPT_CURL_INIT_CB)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_CMD_OPT), (void**)&pCmdOpt);
    BAIL_ON_TDNF_ERROR(dwError);

    pCmdOpt->nType = nType;

    dwError = TDNFAllocateString(pszOptArg, &pCmdOpt->pszOptName);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateString(pszOptValue, &pCmdOpt->pszOptValue);
    BAIL_ON_TDNF_ERROR(dwError);

    pSetOptTemp = pCmdArgs->pSetOpt;
    if(!pSetOptTemp)
    {
        pCmdArgs->pSetOpt = pCmdOpt;
    }
    else
    {
        while(pSetOptTemp->pNext)
        {
            pSetOptTemp = pSetOptTemp->pNext;
        }
        pSetOptTemp->pNext = pCmdOpt;
    }

cleanup:
    return dwError;

error:
    if(pCmdOpt)
    {
        TDNFFreeCmdOpt(pCmdOpt);
    }
    goto cleanup;
}

uint32_t
GetOptionAndValue(
    const char* pszOptArg,
    PTDNF_CMD_OPT* ppCmdOpt
    )
{
    uint32_t dwError = 0;
    const char* EQUAL_SIGN = "=";
    const char* pszIndex = NULL;
    PTDNF_CMD_OPT pCmdOpt = NULL;
    int nEqualsPos = -1;

    if(IsNullOrEmptyString(pszOptArg) || !ppCmdOpt)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pszIndex = strstr(pszOptArg, EQUAL_SIGN);
    if(!pszIndex)
    {
        dwError = ERROR_TDNF_SETOPT_NO_EQUALS;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_CMD_OPT), (void**)&pCmdOpt);
    BAIL_ON_TDNF_ERROR(dwError);

    pCmdOpt->nType = CMDOPT_KEYVALUE;
    dwError = TDNFAllocateString(pszOptArg, &pCmdOpt->pszOptName);
    BAIL_ON_TDNF_ERROR(dwError);

    nEqualsPos = pszIndex - pszOptArg;
    pCmdOpt->pszOptName[nEqualsPos] = '\0';

    pCmdOpt->nType = CMDOPT_KEYVALUE;
    dwError = TDNFAllocateString(pszOptArg+nEqualsPos+1,
                                 &pCmdOpt->pszOptValue);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppCmdOpt = pCmdOpt;
cleanup:
    return dwError;

error:
    if(ppCmdOpt)
    {
        *ppCmdOpt = NULL;
    }
    if(pCmdOpt)
    {
        TDNFFreeCmdOpt(pCmdOpt);
    }
    goto cleanup;
}
