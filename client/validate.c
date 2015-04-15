/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : validate.c
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
TDNFValidateCmdArgs(
    PTDNF pTdnf
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;

    if(!pTdnf || !pTdnf->pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    pCmdArgs = pTdnf->pArgs;

    //TODO: make a better mechanism to handle requirements per cmd
    //For now, check if at least one argument exists for install 
    if(!strcmp(pCmdArgs->ppszCmds[0], CMD_INSTALL))
    {
        if(pCmdArgs->nCmdCount <= 1)
        {
            dwError = ERROR_TDNF_PACKAGE_REQUIRED;
            BAIL_ON_TDNF_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
