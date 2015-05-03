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

    printf("List of Main Commands\n");
    printf("\n");

    printf("autoerase\n");
    printf("check-local               Checks local rpm folder for problems\n");
    printf("check-update              Check for available package upgrades\n");
    printf("clean                     Remove cached data\n");
    printf("distro-sync               Synchronize installed packages to the latest available versions\n");
    printf("downgrade                 downgrade a package\n");
    printf("erase                     Remove a package or packages from your system\n");
    printf("group                     Display, or use, the groups information\n");
    printf("help                      Display a helpful usage message\n");
    printf("history                   Display, or use, the transaction history\n");
    printf("info                      Display details about a package or group of packages\n");
    printf("install                   Install a package or packages on your system\n");
    printf("list                      List a package or groups of packages\n");
    printf("makecache                 Generate the metadata cache\n");
    printf("provides                  Find what package provides the given value\n");
    printf("reinstall                 reinstall a package\n");
    printf("remove                    Remove a package or packages from your system\n");
    printf("repolist                  Display the configured software repositories\n");
    printf("repository-packages       Run commands on top of all packages in given repository\n");
    printf("search                    Search package details for the given string\n");
    printf("updateinfo                Display advisories about packages\n");
    printf("upgrade                   Upgrade a package or packages on your system\n");
    printf("upgrade-to                Upgrade a package on your system to the specified version\n");
}

void
TDNFCliShowOptionsUsage(
    )
{
    printf("usage: dnf [--allowerasing] [-b] [-C] [-c [config file]] [-d [debug level]]\n");
    printf("           [--debugsolver] [--showduplicates] [-e ERRORLEVEL]\n");
    printf("           [--rpmverbosity [debug level name]] [-q] [-v] [-y] [--assumeno]\n");
    printf("           [--version] [--installroot [path]] [--enablerepo [repo]]\n");
    printf("           [--disablerepo [repo]] [-x [package]] [--disableexcludes [repo]]\n");
    printf("           [--obsoletes] [--noplugins] [--nogpgcheck]\n");
    printf("           [--disableplugin [plugin]] [--color COLOR]\n");
    printf("           [--releasever RELEASEVER] [--setopt SETOPTS] [--refresh] [-4] [-6]\n");
    printf("           [-h]\n");
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
    PTDNF pTdnf,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    TDNFCliShowHelp();
    return 0;
}
