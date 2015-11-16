/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
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
    {"refresh",       no_argument, &_opt.nRefresh, 1},     //--refresh 
    {"rpmverbosity",  required_argument, 0, 0},            //--rpmverbosity
    {"setopt",        required_argument, 0, 0},            //--set or override options
    {"showduplicates",required_argument, 0, 0},            //--showduplicates
    {"version",       no_argument, &_opt.nShowVersion, 1}, //--version
    {"verbose",       no_argument, &_opt.nVerbose, 1},     //-v --verbose
    {"4",             no_argument, 0, '4'},                //-4 resolve to IPv4 addresses only
    {"6",             no_argument, 0, '6'},                //-4 resolve to IPv4 addresses only
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
                            sizeof(TDNF_CMD_ARGS),
                            (void**)&pCmdArgs);
    BAIL_ON_CLI_ERROR(dwError);

    opterr = 0;//tell getopt to not print errors
    while (1)
    {
                
            nOption = getopt_long (
                           argc,
                           argv,
                           "46bCc:d:e:hiqvxy",
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
                case 'e':
                break;
                case 'C':
                    _opt.nCacheOnly = 1;
                break;
                case 'h':
                    _opt.nShowHelp = 1;
                break;
                case 'i':
                    dwError = TDNFAllocateString(
                             optarg,
                             &pCmdArgs->pszInstallRoot);
                    BAIL_ON_CLI_ERROR(dwError);
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
                case '?':
                    dwError = HandleOptionsError(argv[optind-1], optarg, pstOptions);
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
        pCmdArgs->nCmdCount = argc-optind;
        dwError = TDNFAllocateMemory(
                                pCmdArgs->nCmdCount * sizeof(char*),
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
    if(!pOptionArgs || !pArgs)
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
    pArgs->nRefresh       = pOptionArgs->nRefresh;
    pArgs->nShowDuplicates= pOptionArgs->nShowDuplicates;
    pArgs->nShowHelp      = pOptionArgs->nShowHelp;
    pArgs->nShowVersion   = pOptionArgs->nShowVersion;
    pArgs->nVerbose       = pOptionArgs->nVerbose;
    pArgs->nIPv4          = pOptionArgs->nIPv4;
    pArgs->nIPv6          = pOptionArgs->nIPv6;

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliParseInfoArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_LIST_ARGS* ppListArgs
    )
{
    return TDNFCliParseListArgs(
        pCmdArgs,
        ppListArgs);
}

uint32_t
TDNFCliParsePackageArgs(
    PTDNF_CMD_ARGS pCmdArgs,
    char*** pppszPackageArgs,
    int* pnPackageCount
    )
{
    uint32_t dwError = 0;
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;
    int nIndex = 0;

    if(!pCmdArgs || !pppszPackageArgs || !pnPackageCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nPackageCount = pCmdArgs->nCmdCount - 1;
    if(nPackageCount < 0)
    {
        dwError = ERROR_TDNF_CLI_NOT_ENOUGH_ARGS;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(
                  sizeof(char*) * (nPackageCount + 1),
                  (void**)&ppszPackageArgs);
    BAIL_ON_CLI_ERROR(dwError);

    for(nIndex = 0; nIndex < nPackageCount; ++nIndex)
    {
        dwError = TDNFAllocateString(
                      pCmdArgs->ppszCmds[nIndex+1],
                      &ppszPackageArgs[nIndex]);
        BAIL_ON_CLI_ERROR(dwError);
    }

    *pppszPackageArgs = ppszPackageArgs;
cleanup:
    return dwError;

error:
    if(pppszPackageArgs)
    {
        *pppszPackageArgs = NULL;
    }
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    goto cleanup;
}

uint32_t
ParseOption(
    const char* pszName,
    const char* pszArg,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    if(!pszName || !pCmdArgs)
    {
        dwError = ERROR_TDNF_CLI_INVALID_ARGUMENT;
        BAIL_ON_CLI_ERROR(dwError);
    }
    dwError = TDNFCliValidateOptions(pszName, pszArg, pstOptions);
    BAIL_ON_CLI_ERROR(dwError);

    if(!strcasecmp(pszName, "rpmverbosity"))
    {
        if(!optarg)
        {
            dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
            BAIL_ON_CLI_ERROR(dwError);
        }
        dwError = ParseRpmVerbosity(
                      optarg,
                      &pCmdArgs->nRpmVerbosity);
        BAIL_ON_CLI_ERROR(dwError);
    }
    else if(!strcasecmp(pszName, "enablerepo"))
    {
        if(!optarg)
        {
            dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
            BAIL_ON_CLI_ERROR(dwError);
        }
        fprintf(stdout, "EnableRepo: %s\n", optarg);
    }
    else if(!strcasecmp(pszName, "disablerepo"))
    {
        if(!optarg)
        {
            dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
            BAIL_ON_CLI_ERROR(dwError);
        }
        fprintf(stdout, "DisableRepo: %s\n", optarg);
    }
    else if(!strcasecmp(pszName, "installroot"))
    {
        if(!optarg)
        {
            dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
            BAIL_ON_CLI_ERROR(dwError);
        }
        fprintf(stdout, "InstallRoot: %s\n", optarg);
    }
    else if(!strcasecmp(pszName, "setopt"))
    {
        if(!optarg)
        {
            dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
            BAIL_ON_CLI_ERROR(dwError);
        }
        fprintf(stdout, "setopt: %s\n", optarg);
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
ParseRpmVerbosity(
    const char* pszRpmVerbosity,
    int* pnRpmVerbosity
    )
{
    uint32_t dwError = 0;
    int nIndex = 0;
    TDNF_RPMLOG nRpmVerbosity = TDNF_RPMLOG_ERR;
    struct stTemp
    {
        char* pszTypeName;
        int nType;
    };
    struct stTemp  stTypes[] = 
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
    int nCount = sizeof(stTypes)/sizeof(stTypes[0]);
    for(nIndex = 0; nIndex < nCount; ++nIndex)
    {
        if(!strcasecmp(stTypes[nIndex].pszTypeName, pszRpmVerbosity))
        {
            nRpmVerbosity = stTypes[nIndex].nType;
            break;
        }
    }

    *pnRpmVerbosity = nRpmVerbosity;

    return dwError;
}

uint32_t
HandleOptionsError(
    const char* pszName,
    const char* pszArg,
    struct option* pstOptions
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliValidateOptions(
                  pszName,
                  pszArg,
                  pstOptions);
    if(dwError == ERROR_TDNF_CLI_OPTION_NAME_INVALID)
    {
       TDNFCliShowNoSuchOption(pszName);
    }
    else if(dwError == ERROR_TDNF_CLI_OPTION_ARG_REQUIRED)
    {
       fprintf(stderr, "Option %s requires an argument\n", pszName);
    }
    return dwError;
}
