/*
 * Copyright (C) 2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : parserepoqueryargs.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tools
 */

#include "includes.h"

char *whatKeys[REPOQUERY_KEY_COUNT] = {
    "whatprovides",
    "whatobsoletes",
    "whatconflicts",
    "whatrequires",
    "whatrecommends",
    "whatsuggests",
    "whatsupplements",
    "whatenhances"
};

uint32_t
TDNFCliParseRepoQueryArgs(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_REPOQUERY_ARGS* ppRepoqueryArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs = NULL;
    PTDNF_CMD_OPT pSetOpt = NULL;

    if (!pArgs || !ppRepoqueryArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
        1,
        sizeof(TDNF_REPOQUERY_ARGS),
        (void**) &pRepoqueryArgs);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFAllocateMemory(
        REPOQUERY_KEY_COUNT,
        sizeof(char **),
        (void **) &pRepoqueryArgs->pppszWhatKeys);

    for (pSetOpt = pArgs->pSetOpt;
         pSetOpt;
         pSetOpt = pSetOpt->pNext)
    {
        if(pSetOpt->nType == CMDOPT_KEYVALUE)
        {
            if (strcasecmp(pSetOpt->pszOptName, "whatdepends") == 0)
            {
                dwError = TDNFSplitStringToArray(pSetOpt->pszOptValue,
                    ",",
                    &pRepoqueryArgs->ppszWhatDepends);
                BAIL_ON_CLI_ERROR(dwError);
            }
            else if (strcasecmp(pSetOpt->pszOptName, "depends") == 0)
            {
                pRepoqueryArgs->nDepends = 1;
            }
            else
            {
                int i;
                for (i = 0; i < REPOQUERY_KEY_COUNT; i++)
                {
                    if (strcasecmp(pSetOpt->pszOptName, whatKeys[i]) == 0)
                    {
                        dwError = TDNFSplitStringToArray(pSetOpt->pszOptValue,
                            ",",
                            &pRepoqueryArgs->pppszWhatKeys[i]);
                        BAIL_ON_CLI_ERROR(dwError);
                    }
                }
            }
        }
    }

    if(pArgs->nCmdCount > 2)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pArgs->nCmdCount > 1)
    {
        dwError = TDNFAllocateString(pArgs->ppszCmds[1],
                                     &pRepoqueryArgs->pszSpec);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *ppRepoqueryArgs = pRepoqueryArgs;
cleanup:
    return dwError;
error:
    if (pRepoqueryArgs)
    {
        TDNFCliFreeRepoQueryArgs(pRepoqueryArgs);
    }
    goto cleanup;
}

void
TDNFCliFreeRepoQueryArgs(
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs
    )
{
    if(pRepoqueryArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pRepoqueryArgs->ppszWhatDepends);
        if (pRepoqueryArgs->pppszWhatKeys)
        {
            int i;
            for (i = 0; i < REPOQUERY_KEY_COUNT; i++)
            {
                TDNF_CLI_SAFE_FREE_STRINGARRAY(pRepoqueryArgs->pppszWhatKeys[i]);
            }
        }
        TDNFFreeMemory(pRepoqueryArgs->pppszWhatKeys);
        TDNFFreeMemory(pRepoqueryArgs);
    }
}

