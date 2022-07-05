/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : parsehistoryargs.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tools
 */

#include "includes.h"

uint32_t
TDNFCliParseHistoryArgs(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_HISTORY_ARGS* ppHistoryArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_HISTORY_ARGS pHistoryArgs = NULL;
    PTDNF_CMD_OPT pSetOpt = NULL;
    char **ppszRange = NULL;

    if (!pArgs || !ppHistoryArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
        1,
        sizeof(TDNF_HISTORY_ARGS),
        (void**) &pHistoryArgs);
    BAIL_ON_CLI_ERROR(dwError);

    /* history subcommands */
    if (pArgs->nCmdCount > 1)
    {
        if (strcmp(pArgs->ppszCmds[1], "list") == 0)
        {
            pHistoryArgs->nCommand = HISTORY_CMD_LIST;
        }
        else if (strcmp(pArgs->ppszCmds[1], "init") == 0 ||
                 strcmp(pArgs->ppszCmds[1], "update") == 0)
        {
            pHistoryArgs->nCommand = HISTORY_CMD_INIT;
        }
        else if (strcmp(pArgs->ppszCmds[1], "rollback") == 0)
        {
            pHistoryArgs->nCommand = HISTORY_CMD_ROLLBACK;
        }
        else if (strcmp(pArgs->ppszCmds[1], "undo") == 0)
        {
            pHistoryArgs->nCommand = HISTORY_CMD_UNDO;
        }
        else if (strcmp(pArgs->ppszCmds[1], "redo") == 0)
        {
            pHistoryArgs->nCommand = HISTORY_CMD_REDO;
        }
    }

    if (pArgs->nCmdCount > 2 && isdigit(pArgs->ppszCmds[2][0]))
    {
        dwError = TDNFSplitStringToArray(pArgs->ppszCmds[2], "-", &ppszRange);
        BAIL_ON_CLI_ERROR(dwError);

        pHistoryArgs->nFrom = atoi(ppszRange[0]);
        if (ppszRange[1])
        {
            pHistoryArgs->nTo = atoi(ppszRange[1]);
        }
    }

    for (pSetOpt = pArgs->pSetOpt;
         pSetOpt;
         pSetOpt = pSetOpt->pNext)
    {
        if (strcasecmp(pSetOpt->pszOptName, "info") == 0)
        {
            pHistoryArgs->nInfo = 1;
        }
        else if (strcasecmp(pSetOpt->pszOptName, "reverse") == 0)
        {
            pHistoryArgs->nReverse = 1;
        }
        else if (strcasecmp(pSetOpt->pszOptName, "from") == 0)
        {
            pHistoryArgs->nFrom = atoi(pSetOpt->pszOptValue);
        }
        else if (strcasecmp(pSetOpt->pszOptName, "to") == 0)
        {
            pHistoryArgs->nTo = atoi(pSetOpt->pszOptValue);
        }
    }

    if (pHistoryArgs->nTo == 0)
    {
        pHistoryArgs->nTo = pHistoryArgs->nFrom;
    }

    *ppHistoryArgs = pHistoryArgs;
cleanup:
    return dwError;
error:
    if (pHistoryArgs)
    {
        TDNFCliFreeHistoryArgs(pHistoryArgs);
    }
    goto cleanup;
}

void
TDNFCliFreeHistoryArgs(
    PTDNF_HISTORY_ARGS pHistoryArgs
    )
{
    if(pHistoryArgs)
    {
        TDNFFreeMemory(pHistoryArgs);
    }
}

