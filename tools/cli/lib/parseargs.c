/*
 * Copyright (C) 2015-2017 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : parseargs.c
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

typedef struct _SetOptArgs
{
    TDNF_CMDOPT_TYPE Type;
    const char *OptName;
    const char *OptVal;
} SetOptArgs;

static SetOptArgs OptValTable[] = {
    {CMDOPT_KEYVALUE, "sec-severity", NULL},
    {CMDOPT_ENABLEREPO, "enablerepo", NULL},
    {CMDOPT_DISABLEREPO, "disablerepo", NULL},
    {CMDOPT_ENABLEPLUGIN, "enableplugin", NULL},
    {CMDOPT_DISABLEPLUGIN, "disableplugin", NULL},
    {CMDOPT_KEYVALUE, "skipconflicts;skipobsoletes;skipsignature;skipdigest;noplugins;reboot-required;security", "1"}
};

static TDNF_CMD_ARGS _opt = {0};

//options - incomplete
static struct option pstOptions[] =
{
    {"allowerasing",  no_argument, &_opt.nAllowErasing, 1},//--allowerasing
    {"assumeno",      no_argument, &_opt.nAssumeNo, 1},    //--assumeno
    {"assumeyes",     no_argument, 0, 'y'},                //--assumeyes
    {"best",          no_argument, &_opt.nBest, 1},        //--best
    {"cacheonly",     no_argument, 0, 'C'},                //-C, --cacheonly
    {"config",        required_argument, 0, 'c'},          //-c, --config
    {"debuglevel",    required_argument, 0, 'd'},          //-d, --debuglevel
    {"debugsolver",   no_argument, &_opt.nDebugSolver, 1}, //--debugsolver
    {"disablerepo",   required_argument, 0, 0},            //--disablerepo
    {"enablerepo",    required_argument, 0, 0},            //--enablerepo
    {"errorlevel",    required_argument, 0, 'e'},          //-e --errorlevel
    {"help",          no_argument, 0, 'h'},                //-h --help
    {"installroot",   required_argument, 0, 'i'},          //--installroot
    {"nogpgcheck",    no_argument, &_opt.nNoGPGCheck, 1},  //--nogpgcheck
    {"quiet",         no_argument, &_opt.nQuiet, 1},       //--nogpgcheck
    {"refresh",       no_argument, &_opt.nRefresh, 1},     //--refresh
    {"releasever",    required_argument, 0, 0},            //--releasever
    {"rpmverbosity",  required_argument, 0, 0},            //--rpmverbosity
    {"setopt",        required_argument, 0, 0},            //--set or override options
    {"showduplicates",required_argument, 0, 0},            //--showduplicates
    {"version",       no_argument, &_opt.nShowVersion, 1}, //--version
    {"verbose",       no_argument, 0, 'v'},                //-v --verbose
    {"4",             no_argument, 0, '4'},                //-4 resolve to IPv4 addresses only
    {"6",             no_argument, 0, '6'},                //-4 resolve to IPv4 addresses only
    {"exclude",       required_argument, 0, 0},            //--exclude
    {"security",      no_argument, 0, 0},                  //--security
    {"sec-severity",  required_argument, 0, 0},            //--sec-severity
    {"reboot-required", no_argument, 0, 0},                //--reboot-required
    {"skipconflicts", no_argument, 0, 0},                  //--skipconflicts to skip conflict problems
    {"skipobsoletes", no_argument, 0, 0},                  //--skipobsoletes to skip obsolete problems
    {"skipsignature", no_argument, 0, 0},                  //--skipsignature to skip verifying RPM signatures
    {"skipdigest",    no_argument, 0, 0},                  //--skipdigest to skip verifying RPM digest
    {"noplugins",     no_argument, 0, 0},                  //--noplugins
    {"disableplugin", required_argument, 0, 0},            //--disableplugin
    {"enableplugin",  required_argument, 0, 0},            //--enableplugin
    {"disableexcludes", no_argument, &_opt.nDisableExcludes, 1}, //--disableexcludes
    {0, 0, 0, 0}
};

uint32_t
TDNFCliParseArgs(
    int argc,
    char* const* argv,
    PTDNF_CMD_ARGS* ppCmdArgs
    )
{
    uint32_t dwError = 0;
    PTDNF_CMD_ARGS pCmdArgs = NULL;
    int nOptionIndex = 0;
    int nOption = 0;
    int nIndex = 0;
    char* pszDefaultInstallRoot = "/";

    if(!ppCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  1,
                  sizeof(TDNF_CMD_ARGS),
                  (void**)&pCmdArgs);
    BAIL_ON_CLI_ERROR(dwError);

    opterr = 0;//tell getopt to not print errors
    while (1)
    {

            nOption = getopt_long_only (
                          argc,
                          argv,
                          "46bCc:d:e:hi:qvxy",
                          pstOptions,
                          &nOptionIndex);
            if (nOption == -1)
                break;

            switch (nOption)
            {
                case 0:
                    dwError = ParseOption(
                                  pstOptions[nOptionIndex].name,
                                  optarg,
                                  pCmdArgs);
                    BAIL_ON_CLI_ERROR(dwError);
                break;
                case 'b':
                    _opt.nBest = 1;
                break;
                case 'c':
                    dwError = ParseOption(
                                  "config",
                                  optarg,
                                  pCmdArgs);
                    BAIL_ON_CLI_ERROR(dwError);
                break;
                case 'e':
                break;
                case 'C':
                    _opt.nCacheOnly = 1;
                break;
                case 'h':
                    _opt.nShowHelp = 1;
                break;
                case 'i':
                    dwError = ParseOption(
                                  "installroot",
                                  optarg,
                                  pCmdArgs);
                    BAIL_ON_CLI_ERROR(dwError);
                break;
                case 'q':
                    _opt.nQuiet = 1;
                break;
                case 'r':
                break;
                case 'y':
                    _opt.nAssumeYes = 1;
                break;
                case '4':
                    _opt.nIPv4 = 1;
                break;
                case '6':
                    _opt.nIPv6 = 1;
                break;
                case 'v':
                    _opt.nVerbose = 1;
                break;
                case '?':
                    dwError = HandleOptionsError(
                                  argv[optind-1],
                                  optarg,
                                  pstOptions);
                    BAIL_ON_CLI_ERROR(dwError);
                //TODO: Handle unknown option, incomplete options
                break;
            }
    }

    if(pCmdArgs->pszInstallRoot == NULL)
    {
        dwError = TDNFAllocateString(
                      pszDefaultInstallRoot,
                      &pCmdArgs->pszInstallRoot);
                      BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCopyOptions(&_opt, pCmdArgs);
    BAIL_ON_CLI_ERROR(dwError);

    //Collect extra args
    if (optind < argc)
    {
        pCmdArgs->nCmdCount = argc - optind;
        dwError = TDNFAllocateMemory(
                      pCmdArgs->nCmdCount,
                      sizeof(char*),
                      (void**)&pCmdArgs->ppszCmds);
        BAIL_ON_CLI_ERROR(dwError);

        while (optind < argc)
        {
            dwError = TDNFAllocateString(
                          argv[optind++],
                          &pCmdArgs->ppszCmds[nIndex++]);
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

    *ppCmdArgs = pCmdArgs;

cleanup:
    return dwError;

error:
    if(ppCmdArgs)
    {
        *ppCmdArgs = NULL;
    }
    if(pCmdArgs)
    {
        TDNFFreeCmdArgs(pCmdArgs);
    }
    goto cleanup;
}

uint32_t
TDNFCopyOptions(
    PTDNF_CMD_ARGS pOptionArgs,
    PTDNF_CMD_ARGS pArgs
    )
{
    uint32_t dwError = 0;

    if (!pOptionArgs || !pArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pArgs->nAllowErasing  = pOptionArgs->nAllowErasing;
    pArgs->nAssumeNo      = pOptionArgs->nAssumeNo;
    pArgs->nAssumeYes     = pOptionArgs->nAssumeYes;
    pArgs->nBest          = pOptionArgs->nBest;
    pArgs->nCacheOnly     = pOptionArgs->nCacheOnly;
    pArgs->nDebugSolver   = pOptionArgs->nDebugSolver;
    pArgs->nNoGPGCheck    = pOptionArgs->nNoGPGCheck;
    pArgs->nNoOutput      = pOptionArgs->nQuiet && pOptionArgs->nAssumeYes;
    pArgs->nQuiet         = pOptionArgs->nQuiet;
    pArgs->nRefresh       = pOptionArgs->nRefresh;
    pArgs->nShowDuplicates= pOptionArgs->nShowDuplicates;
    pArgs->nShowHelp      = pOptionArgs->nShowHelp;
    pArgs->nShowVersion   = pOptionArgs->nShowVersion;
    pArgs->nVerbose       = pOptionArgs->nVerbose;
    pArgs->nIPv4          = pOptionArgs->nIPv4;
    pArgs->nIPv6          = pOptionArgs->nIPv6;
    pArgs->nDisableExcludes = pOptionArgs->nDisableExcludes;

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
ParseOption(
    const char *pszName,
    const char *pszArg,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    char *pszCopyArgs = NULL;

    if (!pszName || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliValidateOptions(pszName, pszArg, pstOptions);
    BAIL_ON_CLI_ERROR(dwError);

    if (!strcasecmp(pszName, "config"))
    {
        dwError = TDNFAllocateString(optarg, &pCmdArgs->pszConfFile);
    }
    else if (!strcasecmp(pszName, "rpmverbosity"))
    {
        dwError = ParseRpmVerbosity(pszArg, &pCmdArgs->nRpmVerbosity);
    }
    else if (!strcasecmp(pszName, "installroot"))
    {
        dwError = TDNFAllocateString(optarg, &pCmdArgs->pszInstallRoot);
    }
    else if (!strcasecmp(pszName, "releasever"))
    {
        dwError = TDNFAllocateString(optarg, &pCmdArgs->pszReleaseVer);
    }
    else if (!strcasecmp(pszName, "setopt"))
    {
        if (!optarg)
        {
            BAIL_ON_CLI_ERROR((dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED));
        }

        dwError = AddSetOpt(pCmdArgs, optarg);
        if (dwError == ERROR_TDNF_SETOPT_NO_EQUALS)
        {
            dwError = ERROR_TDNF_CLI_SETOPT_NO_EQUALS;
        }
    }
    else if (!strcasecmp(pszName, "exclude"))
    {
        char *ToFree = NULL;
        char *pszToken = NULL;

        dwError = TDNFAllocateString(pszArg, &pszCopyArgs);
        BAIL_ON_CLI_ERROR(dwError);

        ToFree = pszCopyArgs;

        while ((pszToken = strsep(&pszCopyArgs, ",:")))
        {
            dwError = AddSetOptWithValues(pCmdArgs,
                                CMDOPT_KEYVALUE,
                                pszName,
                                pszToken);
            BAIL_ON_CLI_ERROR((dwError && (pszCopyArgs = ToFree)));
        }

        pszCopyArgs = ToFree;
    }
    else
    {
        uint32_t i = 0;

        for (i = 0; i < ARRAY_SIZE(OptValTable); i++)
        {
            const char *OptVal = NULL;

            if (!strstr(OptValTable[i].OptName, pszName))
            {
                continue;
            }

            OptVal = !OptValTable[i].OptVal ? optarg : OptValTable[i].OptVal;

            dwError = AddSetOptWithValues(pCmdArgs,
                                OptValTable[i].Type,
                                pszName,
                                OptVal);
            break;
        }
    }

    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    TDNF_CLI_SAFE_FREE_MEMORY(pszCopyArgs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
ParseRpmVerbosity(
    const char *pszRpmVerbosity,
    int *pnRpmVerbosity
    )
{
    uint32_t dwError = 0;
    uint32_t nIndex = 0;
    typedef struct _stTemp
    {
        char *pszTypeName;
        int nType;
    } stTemp;

    if (!pszRpmVerbosity || !pnRpmVerbosity)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    stTemp stTypes[] =
    {
        {"emergency",  TDNF_RPMLOG_EMERG},
        {"alert",      TDNF_RPMLOG_ALERT},
        {"critical",   TDNF_RPMLOG_CRIT},
        {"error",      TDNF_RPMLOG_ERR},
        {"warning",    TDNF_RPMLOG_WARNING},
        {"notice",     TDNF_RPMLOG_NOTICE},
        {"info",       TDNF_RPMLOG_INFO},
        {"debug",      TDNF_RPMLOG_DEBUG},
    };

    for (nIndex = 0; nIndex < ARRAY_SIZE(stTypes); ++nIndex)
    {
        if (!strcasecmp(stTypes[nIndex].pszTypeName, pszRpmVerbosity))
        {
            *pnRpmVerbosity = stTypes[nIndex].nType;
            return dwError;
        }
    }

    *pnRpmVerbosity = TDNF_RPMLOG_ERR;

    return dwError;
}

uint32_t
HandleOptionsError(
    const char *pszName,
    const char *pszArg,
    struct option *pstOptions
    )
{
    uint32_t dwError = 0;

    if (IsNullOrEmptyString(pszName) || !pstOptions)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    dwError = TDNFCliValidateOptions(
                  pszName,
                  pszArg,
                  pstOptions);
    if (dwError == ERROR_TDNF_CLI_OPTION_NAME_INVALID)
    {
       TDNFCliShowNoSuchOption(pszName);
    }
    else if (dwError == ERROR_TDNF_CLI_OPTION_ARG_REQUIRED)
    {
       fprintf(stderr, "Option %s requires an argument\n", pszName);
    }

    return dwError;
}
