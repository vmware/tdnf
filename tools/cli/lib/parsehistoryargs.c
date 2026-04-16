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
#include "../llconf/nodes.h"


uint32_t
TDNFCliParseHistoryArgs(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_HISTORY_ARGS* ppHistoryArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_HISTORY_ARGS pHistoryArgs = NULL;
    struct cnfnode *cn = NULL;
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
        else if (strcmp(pArgs->ppszCmds[1], "id") == 0)
        {
            pHistoryArgs->nCommand = HISTORY_CMD_ID;
        }
    }

    if (pArgs->nCmdCount > 2 && isdigit(pArgs->ppszCmds[2][0]))
    {
        dwError = TDNFSplitStringToArray(pArgs->ppszCmds[2], (char *)"-", &ppszRange);
        BAIL_ON_CLI_ERROR(dwError);

        pHistoryArgs->nFrom = strtoi(ppszRange[0]);
        if (ppszRange[1])
        {
            pHistoryArgs->nTo = strtoi(ppszRange[1]);
        }
    }

    for (cn = pArgs->cn_setopts->first_child; cn; cn = cn->next) {
        if (strcasecmp(cn->name, "info") == 0) {
            pHistoryArgs->nInfo = 1;
        }
        else if (strcasecmp(cn->name, "reverse") == 0) {
            pHistoryArgs->nReverse = 1;
        }
        else if (strcasecmp(cn->name, "from") == 0) {
            pHistoryArgs->nFrom = strtoi(cn->value);
        }
        else if (strcasecmp(cn->name, "to") == 0) {
            pHistoryArgs->nTo = strtoi(cn->value);
        }
    }

    if (pHistoryArgs->nTo == 0)
    {
        pHistoryArgs->nTo = pHistoryArgs->nFrom;
    }

    *ppHistoryArgs = pHistoryArgs;
cleanup:
    TDNF_SAFE_FREE_MEMORY(ppszRange);
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

