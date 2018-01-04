/*
 * Copyright (C) 2015-2018 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : client.c
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
TDNFApplyScopeFilter(
    PSolvQuery pQuery,
    TDNF_SCOPE nScope
    )
{
    uint32_t dwError = 0;

    if(!pQuery || nScope == SCOPE_NONE)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pQuery->nScope = nScope;

    switch(nScope)
    {
        case SCOPE_INSTALLED:
            dwError = SolvAddSystemRepoFilter(pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
            break;

        case SCOPE_AVAILABLE:
            dwError = SolvAddAvailableRepoFilter(pQuery);
            BAIL_ON_TDNF_ERROR(dwError);
            break;

        default:
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
