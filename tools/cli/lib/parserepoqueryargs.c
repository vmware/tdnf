/*
 * Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
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
#include "../llconf/nodes.h"

const char *depKeys[] = {
    "provides",
    "obsoletes",
    "conflicts",
    "requires",
    "recommends",
    "suggests",
    "supplements",
    "enhances",
    "depends",
    "requires-pre"
};

const char *whatKeys[REPOQUERY_WHAT_KEY_COUNT] = {
    "whatprovides",
    "whatobsoletes",
    "whatconflicts",
    "whatrequires",
    "whatrecommends",
    "whatsuggests",
    "whatsupplements",
    "whatenhances",
    "whatdepends"
};

uint32_t
TDNFCliParseRepoQueryArgs(
    PTDNF_CMD_ARGS pArgs,
    PTDNF_REPOQUERY_ARGS* ppRepoqueryArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs = NULL;
    struct cnfnode *cn = NULL;

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
        REPOQUERY_WHAT_KEY_COUNT,
        sizeof(char **),
        (void **) &pRepoqueryArgs->pppszWhatKeys);

    for (cn = pArgs->cn_setopts->first_child; cn; cn = cn->next) {
        if (strcasecmp(cn->name, "arch") == 0)
        {
            int i;
            if (pRepoqueryArgs->ppszArchs == NULL)
            {
                dwError = TDNFAllocateMemory(TDNF_REPOQUERY_MAXARCHS+1, sizeof(char *),
                    (void **)&pRepoqueryArgs->ppszArchs);
                BAIL_ON_CLI_ERROR(dwError);
            }
            for (i = 0; i < TDNF_REPOQUERY_MAXARCHS && pRepoqueryArgs->ppszArchs[i]; i++);
            if (i < TDNF_REPOQUERY_MAXARCHS)
            {
                dwError = TDNFAllocateString(
                    cn->value,
                    &(pRepoqueryArgs->ppszArchs[i]));
                BAIL_ON_CLI_ERROR(dwError);
            }
        }
        else if (strcasecmp(cn->name, "file") == 0)
        {
            dwError = TDNFAllocateString(cn->value,
                                         &pRepoqueryArgs->pszFile);
            BAIL_ON_CLI_ERROR(dwError);
        }
        else if (strcasecmp(cn->name, "changelogs") == 0)
        {
            pRepoqueryArgs->nChangeLogs = 1;
        }
        else if (strcasecmp(cn->name, "available") == 0)
        {
            pRepoqueryArgs->nAvailable = 1;
        }
        else if (strcasecmp(cn->name, "installed") == 0)
        {
            pRepoqueryArgs->nInstalled = 1;
        }
        else if (strcasecmp(cn->name, "extras") == 0)
        {
            pRepoqueryArgs->nExtras = 1;
        }
        else if (strcasecmp(cn->name, "location") == 0)
        {
            pRepoqueryArgs->nLocation = 1;
        }
        else if (strcasecmp(cn->name, "duplicates") == 0)
        {
            pRepoqueryArgs->nDuplicates = 1;
        }
        else if (strcasecmp(cn->name, "list") == 0)
        {
            pRepoqueryArgs->nList = 1;
        }
        else if (strcasecmp(cn->name, "qf") == 0)
        {
            if (cn->next != NULL)
            {
                dwError = ERROR_TDNF_CLI_INVALID_MIXED_QUERY_QUERYFORMAT;
                BAIL_ON_CLI_ERROR(dwError);
            }

            dwError = TDNFAllocateString(cn->value,
                                         &pRepoqueryArgs->pszQueryFormat);
            BAIL_ON_CLI_ERROR(dwError);

            /* this triggers TDNFRepoQuery() to fill in full URL */
            if (strstr(pRepoqueryArgs->pszQueryFormat, "%{location}") != NULL)
                pRepoqueryArgs->nLocation = 1;
        }
        else if (strcasecmp(cn->name, "source") == 0)
        {
            pRepoqueryArgs->nSource = 1;
        }
        else if (strcasecmp(cn->name, "upgrades") == 0)
        {
            pRepoqueryArgs->nUpgrades = 1;
        }
        else if (strcasecmp(cn->name, "downgrades") == 0)
        {
            pRepoqueryArgs->nDowngrades = 1;
        }
        else if (strcasecmp(cn->name, "userinstalled") == 0)
        {
            pRepoqueryArgs->nUserInstalled = 1;
        }
        else
        {
            int depKey;

            for (depKey = 0; depKey < REPOQUERY_DEP_KEY_COUNT; depKey++)
            {
                if (strcasecmp(cn->name, depKeys[depKey]) == 0)
                {
                    if (!(pRepoqueryArgs->depKeySet & (1 << depKey)))
                    {
                        if (cn->next != NULL)
                        {
                            dwError = ERROR_TDNF_CLI_INVALID_MIXED_QUERY_QUERYFORMAT;
                            BAIL_ON_CLI_ERROR(dwError);
                        }

                        pRepoqueryArgs->depKeySet |= 1 << depKey;
                        break;
                    }
                    else
                    {
                        dwError = ERROR_TDNF_CLI_ONE_DEP_ONLY;
                        BAIL_ON_CLI_ERROR(dwError);
                    }
                }
            }
            if (depKey == REPOQUERY_DEP_KEY_COUNT) /* not found in loop above */
            {
                REPOQUERY_WHAT_KEY whatKey;

                for (whatKey = 0; whatKey < REPOQUERY_WHAT_KEY_COUNT; whatKey++)
                {
                    if (strcasecmp(cn->name, whatKeys[whatKey]) == 0)
                    {
                        dwError = TDNFSplitStringToArray(cn->value,
                            (char *)",",
                            &pRepoqueryArgs->pppszWhatKeys[whatKey]);
                        BAIL_ON_CLI_ERROR(dwError);
                        break;
                    }
                }
            } /* if (i == REPOQUERY_WHAT_KEY_COUNT) */
        } /* if (strcasecmp(cn->name, ... */
    } /* for (cn ... */

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
        BAIL_ON_CLI_ERROR(dwError);
    }

    *ppRepoqueryArgs = pRepoqueryArgs;
cleanup:
    return dwError;
error:
    if (ppRepoqueryArgs)
    {
        *ppRepoqueryArgs = NULL;
    }
    TDNFCliFreeRepoQueryArgs(pRepoqueryArgs);
    goto cleanup;
}

void
TDNFCliFreeRepoQueryArgs(
    PTDNF_REPOQUERY_ARGS pRepoqueryArgs
    )
{
    if(pRepoqueryArgs)
    {
        TDNF_CLI_SAFE_FREE_STRINGARRAY(pRepoqueryArgs->ppszArchs);
        if (pRepoqueryArgs->pppszWhatKeys)
        {
            int i;
            for (i = 0; i < REPOQUERY_WHAT_KEY_COUNT; i++)
            {
                TDNF_CLI_SAFE_FREE_STRINGARRAY(pRepoqueryArgs->pppszWhatKeys[i]);
                TDNFFreeMemory(pRepoqueryArgs->pppszWhatKeys[i]);
            }
            TDNFFreeMemory(pRepoqueryArgs->pppszWhatKeys);
        }
        TDNF_CLI_SAFE_FREE_MEMORY(pRepoqueryArgs->pszFile);
        TDNF_CLI_SAFE_FREE_MEMORY(pRepoqueryArgs->pszSpec);
        TDNF_CLI_SAFE_FREE_MEMORY(pRepoqueryArgs);
    }
}

