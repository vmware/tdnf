/*
 * Copyright (C) 2015-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

void
TDNFCliShowUsage(
    void
    )
{
    pr_crit("You need to give some command\n");
    TDNFCliShowHelp();
}

void
TDNFCliShowHelp(
    void
    )
{
    pr_crit("usage: tdnf [options] COMMAND\n");
    pr_crit("\n");

    pr_crit("options    [-c [config file]]\n");
    pr_crit("           [--debugsolver]\n");
    pr_crit("           [--disablerepo=<repoid>]\n");
    pr_crit("           [--enablerepo=<repoid>]\n");
    pr_crit("           [--noplugins]\n");
    pr_crit("           [--enableplugin=<plugin_name>]\n");
    pr_crit("           [--disableplugin=<plugin_name>]\n");
    pr_crit("           [--rpmverbosity [debug level name]] [-v] [-y] [--assumeno]\n");
    pr_crit("           [--version] [--installroot [path]]\n");
    pr_crit("           [--nogpgcheck]\n");
    pr_crit("           [-q, --quiet]\n");
    pr_crit("           [--releasever RELEASEVER] [--setopt SETOPTS]\n");
    pr_crit("           [--refresh]\n");
    pr_crit("           [--exclude [file1,file2,...]]\n");
    pr_crit("           [--security]\n");
    pr_crit("           [--sec-severity CVSS_v3.0_Severity]\n");
    pr_crit("           [--reboot-required]\n");
    pr_crit("           [--skipsignature]\n");
    pr_crit("           [--skipdigest]\n");
    pr_crit("           [--disableexcludes]\n");
    pr_crit("           [--downloadonly]\n");
    pr_crit("           [--downloaddir=<directory>]\n");
    pr_crit("\n");
    pr_crit("repoquery select options:\n");
    pr_crit("           [--available\n");
    pr_crit("           [--duplicates\n");
    pr_crit("           [--extras\n");
    pr_crit("           [--file <file>\n");
    pr_crit("           [--installed\n");
    pr_crit("           [--whatdepends <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatrequires <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatenhances <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatobsoletes <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatprovides <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatrecommends <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatrequires <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatsuggests <capability1>[,<capability2>[..]]]\n");
    pr_crit("           [--whatsupplements <capability1>[,<capability2>[..]]]\n");
    pr_crit("repoquery query options:\n");
    pr_crit("           [--depends\n");
    pr_crit("           [--enhances\n");
    pr_crit("           [--list\n");
    pr_crit("           [--obsoletes\n");
    pr_crit("           [--provides\n");
    pr_crit("           [--recommends\n");
    pr_crit("           [--requires\n");
    pr_crit("           [--requires-pre\n");
    pr_crit("           [--suggests\n");
    pr_crit("           [--supplements\n");
    pr_crit("\n");
    pr_crit("reposync options:\n");
    pr_crit("           [--arch=<arch> [--arch=<arch> [..]]\n");
    pr_crit("           [--delete]\n");
    pr_crit("           [--download-path=<directory>]\n");
    pr_crit("           [--download-metadata]\n");
    pr_crit("           [--gpgcheck]\n");
    pr_crit("           [--metadata-path=<directory>]\n");
    pr_crit("           [--newest-only]\n");
    pr_crit("           [--norepopath]\n");
    pr_crit("           [--source]\n");
    pr_crit("           [--urls]\n");
    pr_crit("\n");

    pr_crit("List of Main Commands\n");
    pr_crit("\n");

    pr_crit("check-local               Checks local rpm folder for problems\n");
    pr_crit("check-update              Check for available package upgrades\n");
    pr_crit("clean                     Remove cached data\n");
    pr_crit("distro-sync               Synchronize installed packages to the latest available versions\n");
    pr_crit("downgrade                 downgrade a package\n");
    pr_crit("erase                     Remove a package or packages from your system\n");
    pr_crit("help                      Display a helpful usage message\n");
    pr_crit("info                      Display details about a package or group of packages\n");
    pr_crit("install                   Install a package or packages on your system\n");
    pr_crit("list                      List a package or groups of packages\n");
    pr_crit("makecache                 Generate the metadata cache\n");
    pr_crit("provides                  Find what package provides the given value\n");
    pr_crit("remove                    Remove a package or packages from your system\n");
    pr_crit("reinstall                 reinstall a package\n");
    pr_crit("repolist                  Display the configured software repositories\n");
    pr_crit("repoquery                 Query repositories\n");
    pr_crit("reposync                  Download all packages from one or more repositories to a directory\n");
    pr_crit("search                    Search package details for the given string\n");
    pr_crit("update                    Upgrade a package or packages on your system (same as 'upgrade')\n");
    pr_crit("updateinfo                Display advisories about packages\n");
    pr_crit("upgrade                   Upgrade a package or packages on your system\n");
    pr_crit("upgrade-to                Upgrade a package on your system to the specified version\n");
}

void
TDNFCliShowNoSuchCommand(
    const char *pszCmd
    )
{
    pr_crit("No such command: %s. Please use /usr/bin/tdnf --help\n",
            pszCmd ? pszCmd : "");
}

void
TDNFCliShowNoSuchOption(
    const char *pszOption
    )
{
    pr_crit("No such option: %s. Please use /usr/bin/tdnf --help\n",
            pszOption ? pszOption : "");
}

uint32_t
TDNFCliHelpCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    if (!pCmdArgs || !pContext)
    {
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    TDNFCliShowHelp();

    return 0;
}
