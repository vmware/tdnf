/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : help.c
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

void
TDNFCliShowUsage(
    )
{
    printf("You need to give some command\n");
    TDNFCliShowHelp();
}

void
TDNFCliShowHelp(
    )
{
    printf("usage: tdnf [options] COMMAND\n");
    printf("\n");

    printf("options    [-c [config file]]\n");
    printf("           [--debugsolver]\n");
    printf("           [--disablerepo=<repoid>]\n");
    printf("           [--enablerepo=<repoid>]\n");
    printf("           [--rpmverbosity [debug level name]] [-v] [-y] [--assumeno]\n");
    printf("           [--version] [--installroot [path]]\n");
    printf("           [--nogpgcheck]\n");
    printf("           [-q, --quiet]\n");
    printf("           [--releasever RELEASEVER] [--setopt SETOPTS]\n");
    printf("           [--refresh]\n");
    printf("           [--exclude [file1,file2,...]]\n");
    printf("           [--security]\n");
    printf("           [--sec-severity CVSS_v3.0_Severity]\n");

    printf("List of Main Commands\n");
    printf("\n");

    printf("check-local               Checks local rpm folder for problems\n");
    printf("check-update              Check for available package upgrades\n");
    printf("clean                     Remove cached data\n");
    printf("distro-sync               Synchronize installed packages to the latest available versions\n");
    printf("downgrade                 downgrade a package\n");
    printf("erase                     Remove a package or packages from your system\n");
    printf("help                      Display a helpful usage message\n");
    printf("info                      Display details about a package or group of packages\n");
    printf("install                   Install a package or packages on your system\n");
    printf("list                      List a package or groups of packages\n");
    printf("makecache                 Generate the metadata cache\n");
    printf("provides                  Find what package provides the given value\n");
    printf("remove                    Remove a package or packages from your system\n");
    printf("reinstall                 reinstall a package\n");
    printf("repolist                  Display the configured software repositories\n");
    printf("search                    Search package details for the given string\n");
    printf("updateinfo                Display advisories about packages\n");
    printf("upgrade                   Upgrade a package or packages on your system\n");
    printf("upgrade-to                Upgrade a package on your system to the specified version\n");
}

void
TDNFCliShowNoSuchCommand(
    const char* pszCmd
    )
{
    printf("No such command: %s. Please use /usr/bin/tdnf --help\n", pszCmd);
}

void
TDNFCliShowNoSuchOption(
    const char* pszOption
    )
{
    printf("No such option: %s. Please use /usr/bin/tdnf --help\n", pszOption);
}

uint32_t
TDNFCliHelpCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    TDNFCliShowHelp();
    return 0;
}
