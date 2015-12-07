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

uint32_t
AddSetOpt(
    PTDNF_CMD_ARGS pCmdArgs,
    const char* pszOptArg
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_OPT pCmdOpt = NULL;
    PTDNF_CMD_OPT pCmdOptEnd = NULL;

    if(!pCmdArgs || IsNullOrEmptyString(pszOptArg))
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }
    dwError = GetOptionAndValue(pszOptArg, &pCmdOpt);
    BAIL_ON_CLI_ERROR(dwError);

    pCmdOptEnd = pCmdArgs->pSetOpt;
    if(!pCmdOptEnd)
    {
        pCmdArgs->pSetOpt = pCmdOpt;
    }
    else
    {
        while(pCmdOptEnd->pNext)
        {
            pCmdOptEnd = pCmdOptEnd->pNext;
        }
        pCmdOptEnd->pNext = pCmdOpt;
    }

cleanup:
    return dwError;

error:
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
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pszIndex = strstr(pszOptArg, EQUAL_SIGN);
    if(!pszIndex)
    {
        dwError = ERROR_TDNF_CLI_SETOPT_NO_EQUALS;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(sizeof(TDNF_CMD_OPT), (void**)&pCmdOpt);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFAllocateString(pszOptArg, &pCmdOpt->pszOptName);
    BAIL_ON_CLI_ERROR(dwError);

    nEqualsPos = pszIndex - pszOptArg;
    pCmdOpt->pszOptName[nEqualsPos] = '\0';

    dwError = TDNFAllocateString(pszOptArg+nEqualsPos+1, &pCmdOpt->pszOptValue);
    BAIL_ON_CLI_ERROR(dwError);

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
