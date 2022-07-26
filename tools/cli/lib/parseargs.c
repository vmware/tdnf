/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
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

static struct option pstOptions[] =
{
    {"4",             no_argument, 0, '4'},                //-4 resolve to IPv4 addresses only
    {"6",             no_argument, 0, '6'},                //-4 resolve to IPv4 addresses only
    {"alldeps",       no_argument, &_opt.nAllDeps, 1},
    {"allowerasing",  no_argument, &_opt.nAllowErasing, 1},//--allowerasing
    {"assumeno",      no_argument, &_opt.nAssumeNo, 1},    //--assumeno
    {"assumeyes",     no_argument, 0, 'y'},                //--assumeyes
    {"best",          no_argument, &_opt.nBest, 1},        //--best
    {"cacheonly",     no_argument, &_opt.nCacheOnly, 1}, //-C, --cacheonly
    {"config",        required_argument, 0, 'c'},          //-c, --config
    {"debuglevel",    required_argument, 0, 'd'},          //-d, --debuglevel
    {"debugsolver",   no_argument, &_opt.nDebugSolver, 1}, //--debugsolver
    {"disableexcludes", no_argument, &_opt.nDisableExcludes, 1}, //--disableexcludes
    {"disableplugin", required_argument, 0, 0},            //--disableplugin
    {"disablerepo",   required_argument, 0, 0},            //--disablerepo
    {"downloaddir",   required_argument, 0, 0},            //--downloaddir
    {"downloadonly",  no_argument, &_opt.nDownloadOnly, 1}, //--downloadonly
    {"enableplugin",  required_argument, 0, 0},            //--enableplugin
    {"enablerepo",    required_argument, 0, 0},            //--enablerepo
    {"exclude",       required_argument, 0, 0},            //--exclude
    {"help",          no_argument, 0, 'h'},                //-h --help
    {"installroot",   required_argument, 0, 'i'},          //--installroot
    {"json",          no_argument, &_opt.nJsonOutput, 1},
    {"noautoremove",  no_argument, &_opt.nNoAutoRemove, 1},
    {"nogpgcheck",    no_argument, &_opt.nNoGPGCheck, 1},  //--nogpgcheck
    {"noplugins",     no_argument, 0, 0},                  //--noplugins
    {"quiet",         no_argument, &_opt.nQuiet, 1},       //--nogpgcheck
    {"refresh",       no_argument, &_opt.nRefresh, 1},     //--refresh
    {"releasever",    required_argument, 0, 0},            //--releasever
    {"reboot-required", no_argument, 0, 0},                //--reboot-required
    {"repo",          required_argument, 0, 0},            //--repo
    {"repofrompath",  required_argument, 0, 0},            //--repofrompath
    {"repoid",        required_argument, 0, 0},            //--repoid (same as --repo)
    {"rpmverbosity",  required_argument, 0, 0},            //--rpmverbosity
    {"sec-severity",  required_argument, 0, 0},            //--sec-severity
    {"security",      no_argument, 0, 0},                  //--security
    {"setopt",        required_argument, 0, 0},            //--set or override options
    {"skipconflicts", no_argument, 0, 0},                  //--skipconflicts to skip conflict problems
    {"skipdigest",    no_argument, 0, 0},                  //--skipdigest to skip verifying RPM digest
    {"skipobsoletes", no_argument, 0, 0},                  //--skipobsoletes to skip obsolete problems
    {"skipsignature", no_argument, 0, 0},                  //--skipsignature to skip verifying RPM signatures
    {"verbose",       no_argument, &_opt.nVerbose, 1},                //-v --verbose
    {"version",       no_argument, &_opt.nShowVersion, 1}, //--version
    // reposync options
    {"arch",          required_argument, 0, 0},
    {"delete",        no_argument, 0, 0},
    {"download-metadata", no_argument, 0, 0},
    {"download-path", required_argument, 0, 0},
    {"gpgcheck", no_argument, 0, 0},
    {"metadata-path", required_argument, 0, 0},
    {"newest-only",   no_argument, 0, 0},
    {"norepopath",    no_argument, 0, 0},
    {"source",        no_argument, 0, 0},
    {"urls",          no_argument, 0, 0},
    // repoquery option
    // repoquery select options
    {"file",          required_argument, 0, 0},
    {"whatdepends",   required_argument, 0, 0},
    {"whatprovides",  required_argument, 0, 0},
    {"whatobsoletes", required_argument, 0, 0},
    {"whatconflicts", required_argument, 0, 0},
    {"whatrequires",  required_argument, 0, 0},
    {"whatrecommends",required_argument, 0, 0},
    {"whatsuggests",  required_argument, 0, 0},
    {"whatsupplements", required_argument, 0, 0},
    {"whatenhances",  required_argument, 0, 0},
    // repoquery query options
    {"available",     no_argument, 0, 0},
    {"changelogs",    no_argument, 0, 0},
    {"conflicts",     no_argument, 0, 0},
    {"depends",       no_argument, 0, 0},
    {"duplicates",    no_argument, 0, 0},
    {"enhances",      no_argument, 0, 0},
    {"extras",        no_argument, 0, 0},
    {"installed",     no_argument, 0, 0},
    {"list",          no_argument, 0, 0},
    {"obsoletes",     no_argument, 0, 0},
    {"provides",      no_argument, 0, 0},
    {"recommends",    no_argument, 0, 0},
    {"requires",      no_argument, 0, 0},
    {"requires-pre",  no_argument, 0, 0},
    {"suggests",      no_argument, 0, 0},
    {"supplements",   no_argument, 0, 0},
    {"upgrades",      no_argument, 0, 0},
    // update-info mode options (also 'list' from above)
    {"all",           no_argument, 0, 0},
    {"info",          no_argument, 0, 0},
    {"summary",       no_argument, 0, 0},
    // scope options for list and update-info
    {"recent",       no_argument, 0, 0},
    {"updates",       no_argument, 0, 0},
    {"downgrades",    no_argument, 0, 0},
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

    /*
     * when invoked as 'tdnfj', act as if invoked with with '-j' and '-y'
    * for json output and non-interactive
    */
    if (strlen(argv[0]) >= 5)
    {
        const char *arg0 = argv[0];
        if (strcmp(&arg0[strlen(arg0) - 5], "tdnfj") == 0)
        {
            _opt.nJsonOutput = 1;
            _opt.nAssumeYes = 1;
        }
    }

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

    if (pCmdArgs->pszDownloadDir && !pCmdArgs->nDownloadOnly) {
        dwError = ERROR_TDNF_CLI_DOWNLOADDIR_REQUIRES_DOWNLOADONLY;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if (pCmdArgs->nAllDeps && !pCmdArgs->nDownloadOnly) {
        dwError = ERROR_TDNF_CLI_ALLDEPS_REQUIRES_DOWNLOADONLY;
        BAIL_ON_CLI_ERROR(dwError);
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

    pArgs->nAllDeps       = pOptionArgs->nAllDeps;
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
    pArgs->nDownloadOnly  = pOptionArgs->nDownloadOnly;
    pArgs->nNoAutoRemove  = pOptionArgs->nNoAutoRemove;
    pArgs->nJsonOutput    = pOptionArgs->nJsonOutput;

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
    else if (!strcasecmp(pszName, "downloaddir"))
    {
        dwError = TDNFAllocateString(optarg, &pCmdArgs->pszDownloadDir);
    }
    else if (!strcasecmp(pszName, "releasever"))
    {
        dwError = TDNFAllocateString(optarg, &pCmdArgs->pszReleaseVer);
    }
    else if (!strcasecmp(pszName, "setopt"))
    {
        if (!optarg)
        {
            dwError = ERROR_TDNF_CLI_OPTION_ARG_REQUIRED;
            BAIL_ON_CLI_ERROR(dwError);
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
                                pszName,
                                pszToken);
            BAIL_ON_CLI_ERROR((dwError && (pszCopyArgs = ToFree)));
        }

        pszCopyArgs = ToFree;
    }
    else
    {
        for (int i = 0; i < (int)ARRAY_SIZE(pstOptions); i++)
        {
            if (strcasecmp(pstOptions[i].name, pszName) == 0)
            {
                dwError = AddSetOptWithValues(pCmdArgs,
                                    pszName,
                                    optarg ? optarg : "1");
                break;
            }
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
       pr_err("Option %s requires an argument\n", pszName);
    }

    return dwError;
}
